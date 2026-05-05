
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

// #define DEBUG

#ifdef DEBUG
#include <chrono>
#include <iostream>
#endif

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <unordered_map>

#include "json_reader.h"
#include "request_handler.h"

using namespace std;

constexpr double SECONDS_IN_MINUTE = 60;
constexpr double KILOMETER_PER_HOUR_IN_METER_PER_SECOND = 3.6;

JsonReader::JsonReader(TransportCatalogue& catalogue, std::istream& is)
    : db_(catalogue), request_(catalogue) {
  json::Document input_data = json::Load(is);
  input_.base_requests = move(input_data.GetRoot().AsDict().at("base_requests").AsArray());
  input_.stat_requests = move(input_data.GetRoot().AsDict().at("stat_requests").AsArray());
  if (json::Dict root = input_data.GetRoot().AsDict(); root.count("render_settings"))
    input_.render_settings = move(root.at("render_settings").AsDict());
  if (json::Dict root = input_data.GetRoot().AsDict(); root.count("routing_settings")) {
    input_.routing_settings = move(root.at("routing_settings").AsDict());
  }
}

//---------------Fill-Base-------------------------//

void JsonReader::FillBase() {
  using namespace json;

  for (const Node& command_node : input_.base_requests) {
    const Dict& command = command_node.AsDict();

    if (command.at("type").AsString() == "Stop"s) {
      AddStop(command);
    } else if (command.at("type").AsString() == "Bus"s) {
      AddBus(command);
    }
  }
  // catalogue.IntegrityCheck();
  input_.base_requests.clear();
  if (input_.render_settings) {
    SetRenderSettings();
    input_.render_settings->clear();
  }
  if (input_.routing_settings) {
#ifdef DEBUG
    auto start = std::chrono::high_resolution_clock::now();
#endif
    SetRouteSettings();
#ifdef DEBUG
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    cerr << "Graph prepare time: " << duration << endl;
#endif
    input_.routing_settings->clear();
  }
}

void JsonReader::AddStop(const json::Dict& command) const {
  geo::Coordinates coordinates;
  if (command.count("latitude") || command.count("longitude")) {
    coordinates = {command.at("latitude").AsDouble(), command.at("longitude").AsDouble()};
  }
  const string& stop = command.at("name").AsString();

  db_.AddStop(stop, coordinates);

  if (command.count("road_distances")) {
    for (const auto& [stop_to, value] : command.at("road_distances").AsDict()) {
      db_.SetDistance(stop, stop_to, value.AsInt());
    }
  }
}

void JsonReader::AddBus(const json::Dict& command) const {
  vector<string_view> route;
  for (const json::Node& stop : command.at("stops").AsArray()) {
    route.push_back(stop.AsString());
  }
  if (route.empty()) {
    db_.AddBus(command.at("name").AsString(), route, command.at("is_roundtrip").AsBool());
    return;
  }
  if (command.at("is_roundtrip").AsBool()) {
    if (route.at(0) != route.back()) {
      throw logic_error("Last stop round route must be the same as first");
    }
  } else {
    size_t last = route.size() - 1;
    for (size_t i = last; i > 0; --i) {
      route.push_back(route.at(i - 1));
    }
  }

  db_.AddBus(command.at("name").AsString(), route, command.at("is_roundtrip").AsBool());
}

//---------------Set-Svg-config------------------//

void JsonReader::SetRenderSettings() {
  using namespace json;

  RenderSettings settings;

  settings.width = input_.render_settings->at("width").AsDouble();
  settings.height = input_.render_settings->at("height").AsDouble();

  settings.padding = input_.render_settings->at("padding").AsDouble();

  settings.line_width = input_.render_settings->at("line_width").AsDouble();
  settings.stop_radius = input_.render_settings->at("stop_radius").AsDouble();

  settings.bus_label_font_size = input_.render_settings->at("bus_label_font_size").AsInt();
  Array point = input_.render_settings->at("bus_label_offset").AsArray();
  settings.bus_label_offset = {point[0].AsDouble(), point[1].AsDouble()};

  settings.stop_label_font_size = input_.render_settings->at("stop_label_font_size").AsInt();
  point = input_.render_settings->at("stop_label_offset").AsArray();
  settings.stop_label_offset = {point[0].AsDouble(), point[1].AsDouble()};

  settings.underlayer_color = GetColor(input_.render_settings->at("underlayer_color"));
  settings.underlayer_width = input_.render_settings->at("underlayer_width").AsDouble();

  vector<svg::Color> palette;
  for (auto color : input_.render_settings->at("color_palette").AsArray()) {
    palette.push_back(GetColor(color));
  }
  settings.color_palette = palette;

  MapRenderer renderer;
  renderer.SetConfig(settings);
  request_.SetRenderer(renderer);
}

svg::Color JsonReader::GetColor(json::Node js_color) const {
  if (js_color.IsString()) {
    return js_color.AsString();
  } else if (js_color.IsArray()) {
    auto channel = js_color.AsArray();
    if (channel.size() == 3) {
      return svg::Rgb(channel[0].AsInt(), channel[1].AsInt(), channel[2].AsInt());
    } else if (channel.size() == 4) {
      return svg::Rgba(channel[0].AsInt(), channel[1].AsInt(), channel[2].AsInt(),
                       channel[3].AsDouble());
    }
  }
  throw invalid_argument("Invalid color description");
}

//---------------Set-Route-config------------------//

void JsonReader::SetRouteSettings() {
  double velocity = input_.routing_settings->at("bus_velocity").AsDouble() * 1000. / 60;
  int wait_time = input_.routing_settings->at("bus_wait_time").AsInt();

  // route_manager_.SetParameters(wait_time, velocity);
  // route_manager_.SetBuses();
  route_manager_ = new TransportRouter<Minutes>(db_, wait_time, velocity);
}

//---------------Get-Stat-------------------------//

json::Document JsonReader::GetStat() {
  using namespace json;

  Builder root;
  root.StartArray();

#ifdef DEBUG
  std::chrono::nanoseconds duration{};
#endif

  for (const Node& command_node : input_.stat_requests) {
    const Dict& command = command_node.AsDict();

    if (command.at("type").AsString() == "Stop"s) {
      GetStopInfo(root, command.at("name").AsString(), command.at("id").AsInt());
    } else if (command.at("type").AsString() == "Bus"s) {
      GetBusInfo(root, command.at("name").AsString(), command.at("id").AsInt());
    } else if (command.at("type").AsString() == "Map"s) {
      if (command.count("path"))
        GetMap(root, command.at("id").AsInt(), command.at("path").AsString());
      else
        GetMap(root, command.at("id").AsInt());
    } else if (command.at("type").AsString() == "Route") {
#ifdef DEBUG
      auto start = chrono::high_resolution_clock::now();
#endif

      GetRouteInfo(root, command.at("from").AsString(), command.at("to").AsString(),
                   command.at("id").AsInt());

#ifdef DEBUG
      auto end = chrono::high_resolution_clock::now();
      duration += chrono::duration_cast<chrono::nanoseconds>(end - start);
#endif

    } else
      throw invalid_argument("Unknown request");
  }

#ifdef DEBUG
  cerr << "Get route info time: " << chrono::duration_cast<std::chrono::milliseconds>(duration) << endl;
#endif

  root.EndArray();
  input_.stat_requests.clear();
  return Document(root.Build());
}

void JsonReader::PrintStat(std::ostream& os) {
  json::Print(GetStat(), os);
}

void JsonReader::GetStopInfo(json::Builder& builder, string_view name, int id) const {
  using namespace json;

  const set<string_view>* buses = db_.GetStopInfo(name);
  builder.StartDict().Key("request_id").Value(id);

  if (!buses) {
    builder.Key("error_message").Value("not found");
  } else {
    builder.Key("buses").StartArray();
    for (auto bus : *buses) {
      builder.Value(string(bus));
    }
    builder.EndArray();
  }

  builder.EndDict();
}

void JsonReader::GetBusInfo(json::Builder& builder, string_view name, int id) const {
  using namespace json;

  optional<BusInfo> info = db_.GetBusInfo(name);
  builder.StartDict().Key("request_id").Value(id);

  if (!info) {
    builder.Key("error_message").Value("not found");
  } else {
    auto [stops_amount, unique_stops, path_length, geo_lenth] = info.value();
    builder.Key("curvature")
        .Value(path_length / geo_lenth)
        .Key("route_length")
        .Value(path_length)
        .Key("stop_count")
        .Value(stops_amount)
        .Key("unique_stop_count")
        .Value(unique_stops);
  }
  builder.EndDict();
}

void JsonReader::GetMap(json::Builder& builder, int id, string img_path) const {
  std::stringstream image;
  request_.RenderMap(image);
  builder.StartDict().Key("request_id").Value(id);
  builder.Key("map").Value(image.str()).EndDict();
  if (!img_path.empty()) {
    if (!img_path.ends_with(".svg")) {
      img_path += ".svg";
    }
    fstream svg_file(img_path, ios::out | ios::trunc);
    if (!svg_file.is_open()) {
      cerr << "Ошибка записи в файл" << endl;
      return;
    }
    svg_file << image.str();
    svg_file.close();
  }
}

void JsonReader::GetRouteInfo(json::Builder& builder, std::string_view stop_from,
                              std::string_view stop_to, int id) const {
  struct TrackSection {
    void operator()(const WaitSection& request, json::Builder& builder) {
      builder.StartDict()
          .Key("type")
          .Value("Wait")
          .Key("stop_name")
          .Value(string(request.stop_name))
          .Key("time")
          .Value(request.time)
          .EndDict();
    }
    void operator()(const BusSection& request, json::Builder& builder) {
      builder.StartDict()
          .Key("type")
          .Value("Bus")
          .Key("bus")
          .Value(string(request.bus_name))
          .Key("span_count")
          .Value(request.span_count)
          .Key("time")
          .Value(request.time)
          .EndDict();
    }
  };

  optional<RouteInfo> info = route_manager_->ComputeRoute(stop_from, stop_to);
  builder.StartDict().Key("request_id").Value(id);

  if (!info) {
    builder.Key("error_message").Value("not found");
  } else {
    builder.Key("total_time").Value(info->total_time);
    builder.Key("items").StartArray();
    for (const auto& route_district : info->detail) {
      visit([&builder](const auto& value) { TrackSection{}(value, builder); }, route_district);
    }
    builder.EndArray();
  }
  builder.EndDict();
}