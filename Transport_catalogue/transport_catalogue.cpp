#include "transport_catalogue.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <set>
#include <stdexcept>
#include <unordered_set>

using namespace std;

void TransportCatalogue::AddStop(const string& name, const geo::Coordinates& location) {
  Stop* stop = FindStop(name);
  if (!stop) {
    if (location.Empty()) {
      stop = &stops_.emplace_back(name, nullopt);
    } else {
      stop = &stops_.emplace_back(name, location);
    }
    stopname_to_stop_.emplace(stops_.back().name, &stops_.back());
    // Если остановка уже есть забиваем только ее координаты
  } else {
    stop->coordinates = location;
  }
}

void TransportCatalogue::AddBus(const string& name, const vector<string_view>& track, bool is_round) {
  vector<Stop*> path;
  for (string_view stop : track) {
    // Если остановки пока нет добавляем заглушку
    if (!stopname_to_stop_.count(stop)) {
      AddStop(string(stop), {});
    }
    path.push_back(stopname_to_stop_[stop]);
  }
  buses_.emplace_back(name, path);
  busname_to_bus_.emplace(buses_.back().name, &buses_.back());
  for (string_view stop : track) {
    stopname_to_bus_[FindStop(stop)->name].insert(buses_.back().name);
  }

  is_round_bus_.emplace(&buses_.back(), is_round);
}

optional<BusInfo> TransportCatalogue::GetBusInfo(const string_view bus_name) const {
  Bus* bus = FindBus(bus_name);
  if (!bus)
    return nullopt;
  const vector<Stop*>& route = bus->path;
  int stops_amount = route.size();

  unordered_set<string_view> unique_stops_set;
  for (auto* stop : route) {
    unique_stops_set.insert(stop->name);
  }
  int unique_stops = unique_stops_set.size();

  double route_length = 0;
  double geo_route_length = 0;
  for (size_t i = 1; i < route.size(); i++) {
    geo_route_length +=
        geo::ComputeDistance(route[i - 1]->coordinates.value(), route[i]->coordinates.value());

    route_length += GetDistance(route[i - 1], route[i]);
  }

  return make_optional(BusInfo{stops_amount, unique_stops, route_length, geo_route_length});
}

Stop* TransportCatalogue::FindStop(string_view name) const {
  auto it = stopname_to_stop_.find(name);
  if (it == stopname_to_stop_.end()) {
    return nullptr;
  }
  return it->second;
}
Bus* TransportCatalogue::FindBus(string_view name) const {
  auto it = busname_to_bus_.find(name);
  if (it == busname_to_bus_.end()) {
    return nullptr;
  }
  return it->second;
}

const set<string_view>* TransportCatalogue::GetStopInfo(string_view stop_name) const {
  if (!FindStop(stop_name))
    return nullptr;
  if (stopname_to_bus_.find(stop_name) == stopname_to_bus_.end()) {
    static set<string_view> out;
    return &out;
  }
  return &(stopname_to_bus_.find(stop_name)->second);
}

void TransportCatalogue::SetDistance(std::string_view stopname_from, std::string_view stopname_to,
                                     int distance) {
  Stop* stop_from = FindStop(stopname_from);
  Stop* stop_to = FindStop(stopname_to);
  if (!stop_from) {
    AddStop(string(stopname_from), {});
    stop_from = FindStop(stopname_from);
  }
  if (!stop_to) {
    AddStop(string(stopname_to), {});
    stop_to = FindStop(stopname_to);
  }
  distances_[{stop_from, stop_to}] = distance;
  if (!distances_.count({stop_to, stop_from})) {
    distances_[{stop_to, stop_from}] = distance;
  }
}

double TransportCatalogue::GetDistance(Stop* stop_from, Stop* stop_to) const {
  pair<Stop*, Stop*> key = {stop_from, stop_to};
  if (distances_.count(key)) {
    return distances_.at(key);
  } else
    return geo::ComputeDistance(stop_from->coordinates.value(),
                                stop_to->coordinates.value());  // как без geo:: пашет?
}

double TransportCatalogue::GetDistance(std::string_view stopname_from,
                                       std::string_view stopname_to) const {
  Stop* stop_from = FindStop(stopname_from);
  Stop* stop_to = FindStop(stopname_to);
  return GetDistance(stop_from, stop_to);
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const {
  return buses_;
}

bool TransportCatalogue::IsRoundBus(std::string_view bus) const {
  return is_round_bus_.at(FindBus(bus));
}

const std::deque<Stop>& TransportCatalogue::GetStops() const {
  return stops_;
}

// void TransportCatalogue::IntegrityCheck() const {
//   for (const auto& stop : stops_) {
//     if (!stop.coordinates) {
//       throw domain_error("Some coordinates are missing");
//     }
//   }
// }