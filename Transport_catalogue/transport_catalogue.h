#pragma once

#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "domain.h"

class TransportCatalogue {
 public:
  void AddStop(const std::string& name, const geo::Coordinates& location);
  void AddBus(const std::string& name, const std::vector<std::string_view>& track, bool is_round);
  void SetDistance(std::string_view stopname_from, std::string_view stopname_to, int distance);
  double GetDistance(std::string_view stopname_from, std::string_view stopname_to) const;
  std::optional<BusInfo> GetBusInfo(const std::string_view bus) const;
  const std::set<std::string_view>* GetStopInfo(std::string_view stop) const;
  const std::deque<Bus>& GetBuses() const;
  bool IsRoundBus(std::string_view bus) const;
  const std::deque<Stop>& GetStops() const;
  // void IntegrityCheck() const;

 private:
  struct Hasher {
    std::size_t operator()(const std::pair<Stop*, Stop*>& key) const {
      uintptr_t v1 = reinterpret_cast<uintptr_t>(key.first);
      uintptr_t v2 = reinterpret_cast<uintptr_t>(key.second);
      return (v1 + 37) ^ (v2 << 1);
    }
  };

  std::deque<Stop> stops_;
  std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
  std::deque<Bus> buses_;
  std::unordered_map<std::string_view, Bus*> busname_to_bus_;
  std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_bus_;
  std::unordered_map<std::pair<Stop*, Stop*>, int, Hasher> distances_;
  std::unordered_map<Bus*, bool> is_round_bus_;

  Stop* FindStop(std::string_view name) const;
  Bus* FindBus(std::string_view name) const;
  double GetDistance(Stop* stop_from, Stop* stop_to) const;
};