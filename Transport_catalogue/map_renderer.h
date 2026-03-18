#pragma once

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <map>
#include <optional>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "svg.h"

inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
  return std::abs(value) < EPSILON;
}

class SphereProjector {
 public:
  // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
  template <typename PointInputIt>
  SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
                  double max_height, double padding)
      : padding_(padding)  //
  {
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
      return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end, [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end, [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
      width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
      height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
      // Коэффициенты масштабирования по ширине и высоте ненулевые,
      // берём минимальный из них
      zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
      // Коэффициент масштабирования по ширине ненулевой, используем его
      zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
      // Коэффициент масштабирования по высоте ненулевой, используем его
      zoom_coeff_ = *height_zoom;
    }
  }

  // Проецирует широту и долготу в координаты внутри SVG-изображения
  svg::Point operator()(geo::Coordinates coords) const {
    return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
  }

 private:
  double padding_;
  double min_lon_ = 0;
  double max_lat_ = 0;
  double zoom_coeff_ = 0;
};

struct RenderSettings {
  double width;
  double height;

  double padding;

  double line_width;
  double stop_radius;

  int bus_label_font_size;
  svg::Point bus_label_offset;

  int stop_label_font_size;
  svg::Point stop_label_offset;

  svg::Color underlayer_color;
  double underlayer_width;

  std::vector<svg::Color> color_palette;
};

class MapRenderer {
  public:
  struct BusnameComparator {
    bool operator()(const Bus* lhs, const Bus* rhs) const {
      return lhs->name < rhs->name;
    }
  };

  using BusDB = std::map<const Bus*, bool, BusnameComparator>;
  using StopDB = std::map<std::string_view, geo::Coordinates>;

 public:
  void SetConfig(const RenderSettings& config);
  void Draw(const BusDB& buses, std::ostream& os) const;

 private:
  void DrawBuses(svg::Document& picture, const BusDB& buses, const SphereProjector& flat) const;
  void DrawBusesIncription(svg::Document& picture, const BusDB& buses, const SphereProjector& flat) const;
  void DrawStop(svg::Document& picture, const StopDB& stops, const SphereProjector& flat) const;
  void DrawStopName(svg::Document& picture, const StopDB& stops, const SphereProjector& flat) const;

 private:
  RenderSettings config_;
};