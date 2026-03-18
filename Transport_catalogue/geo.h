#pragma once

#include <cmath>

namespace geo {

struct Coordinates {
  double lat = std::nan("");
  double lng = std::nan("");
  bool operator==(const Coordinates& other) const {
    return lat == other.lat && lng == other.lng;
  }
  bool operator!=(const Coordinates& other) const {
    return !(*this == other);
  }
  bool Empty() const {
    return std::isnan(lat) || std::isnan(lng);
  }
};

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo