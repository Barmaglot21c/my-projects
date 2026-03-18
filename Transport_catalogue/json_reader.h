#pragma once
#include "graph.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

// #include <iosfwd>
#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"

class JsonReader {
 public:
  using Minutes = std::chrono::duration<double, std::chrono::minutes::period>;
  JsonReader(TransportCatalogue& catalogue, std::istream& is);
  ~JsonReader() {
    delete route_manager_;
  }

  /**
   * Наполняет данными транспортный справочник, используя команды из input_data_
   */
  struct JsonInput {
    json::Array base_requests;
    json::Array stat_requests;
    std::optional<json::Dict> render_settings;
    std::optional<json::Dict> routing_settings;
  };

  void FillBase();

  void SetRenderSettings();
  void SetRouteSettings();

  json::Document GetStat();
  void PrintStat(std::ostream& os);

 private:
  void AddStop(const json::Dict& command) const;
  void AddBus(const json::Dict& command) const;
  void GetStopInfo(json::Builder& builder, std::string_view name, int id) const;
  void GetBusInfo(json::Builder& builder, std::string_view name, int id) const;
  svg::Color GetColor(json::Node js_color) const;
  void GetMap(json::Builder& builder, int id) const;
  void GetRouteInfo(json::Builder& builder, std::string_view stop_from, std::string_view stop_to,
                    int id) const;

 private:
  TransportCatalogue& db_;
  JsonInput input_;
  RequestHandler request_;
  TransportRouter<Minutes>* route_manager_;
};