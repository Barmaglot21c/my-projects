#include "map_renderer.h"

#include <array>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

void MapRenderer::SetConfig(const RenderSettings& config) {
  config_ = config;
}

void MapRenderer::Draw(const BusDB& buses, std::ostream& os) const {
  using namespace svg;

  StopDB stops;
  std::vector<geo::Coordinates> stop_coordinates;

  for (auto [bus, is_round] : buses) {
    for (const auto* stop : bus->path) {
      stops[stop->name] = stop->coordinates.value();
      stop_coordinates.push_back(stop->coordinates.value());
    }
  }

  SphereProjector flat(stop_coordinates.begin(), stop_coordinates.end(), config_.width, config_.height,
                       config_.padding);

  Document picture;

  DrawBuses(picture, buses, flat);
  DrawBusesIncription(picture, buses, flat);

  DrawStop(picture, stops, flat);
  DrawStopName(picture, stops, flat);

  picture.Render(os);
}

void MapRenderer::DrawBuses(svg::Document& picture, const BusDB& buses,
                            const SphereProjector& flat) const {
  using namespace svg;

  for (size_t color_counter = 0; auto [bus, is_round] : buses) {
    Polyline route_line;
    Color color = config_.color_palette[color_counter];
    for (const Stop* stop : bus->path) {
      route_line.AddPoint(flat(stop->coordinates.value()));
    }

    picture.Add(route_line.SetStrokeColor(color)
                    .SetStrokeWidth(config_.line_width)
                    .SetFillColor(Color())
                    .SetStrokeLineCap(StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(StrokeLineJoin::ROUND));

    if (color_counter == config_.color_palette.size() - 1) {
      color_counter = 0;
    } else {
      ++color_counter;
    }
  }
}

void MapRenderer::DrawBusesIncription(svg::Document& picture, const BusDB& buses,
                                      const SphereProjector& flat) const {
  using namespace svg;

  for (size_t color_counter = 0; auto [bus, is_round] : buses) {
    const auto& path = bus->path;
    Text inscription;
    Text substrate;
    Color color = config_.color_palette[color_counter];

    inscription.SetData(std::string(bus->name))
        .SetOffset(config_.bus_label_offset)
        .SetFontSize(config_.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold");

    substrate = inscription;

    substrate.SetFillColor(config_.underlayer_color)
        .SetStrokeColor(config_.underlayer_color)
        .SetStrokeWidth(config_.underlayer_width)
        .SetStrokeLineCap(StrokeLineCap::ROUND)
        .SetStrokeLineJoin(StrokeLineJoin::ROUND);
    inscription.SetFillColor(color);

    Point pos_of_path_begin = flat(path.at(0)->coordinates.value());
    inscription.SetPosition(pos_of_path_begin);
    substrate.SetPosition(pos_of_path_begin);
    picture.Add(Text(substrate));
    picture.Add(Text(inscription));

    Stop* end_stop = path.at(path.size() / 2);
    if (!is_round && path.at(0) != end_stop) {
      Point pos_of_path_end = flat(end_stop->coordinates.value());
      inscription.SetPosition(pos_of_path_end);
      substrate.SetPosition(pos_of_path_end);
      picture.Add(substrate);
      picture.Add(inscription);
    }

    if (color_counter == config_.color_palette.size() - 1) {
      color_counter = 0;
    } else {
      ++color_counter;
    }
  }
}

void MapRenderer::DrawStop(svg::Document& picture, const StopDB& stops,
                           const SphereProjector& flat) const {
  using namespace svg;

  for (const auto& [stop_name, coordinates] : stops) {
    Circle stop_marker;
    picture.Add(
        stop_marker.SetCenter(flat(coordinates)).SetRadius(config_.stop_radius).SetFillColor("white"));
  }
}

void MapRenderer::DrawStopName(svg::Document& picture, const StopDB& stops,
                               const SphereProjector& flat) const {
  using namespace svg;

  Text inscription;
  Text substrate;

  inscription.SetOffset(config_.stop_label_offset)
      .SetFontSize(config_.stop_label_font_size)
      .SetFontFamily("Verdana");

  substrate = inscription;

  substrate.SetFillColor(config_.underlayer_color)
      .SetStrokeColor(config_.underlayer_color)
      .SetStrokeWidth(config_.underlayer_width)
      .SetStrokeLineCap(StrokeLineCap::ROUND)
      .SetStrokeLineJoin(StrokeLineJoin::ROUND);
  inscription.SetFillColor("black");

  for (const auto& [stop_name, coordinates] : stops) {
    picture.Add(Text(substrate).SetData(std::string(stop_name)).SetPosition(flat(coordinates)));
    picture.Add(Text(inscription).SetData(std::string(stop_name)).SetPosition(flat(coordinates)));
  }
}
