#pragma once

#include <chrono>
#include <deque>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "domain.h"
#include "router.h"
#include "transport_catalogue.h"

struct VertexIdPair {
  graph::VertexId in;
  graph::VertexId out;
};

struct WaitSection {
  double time;
  std::string_view stop_name;
};

struct BusSection {
  double time;
  std::string_view bus_name;
  int span_count;
};

struct RouteInfo {
  double total_time = 0;
  std::vector<std::variant<WaitSection, BusSection>> detail;
};

template <typename Weight>
class TransportRouter {
 public:
  TransportRouter(const TransportCatalogue& catalogue, int interval, double velocity)
      : db_(catalogue), bus_interval_(Weight(interval)), bus_velocity_(velocity) {
    SetStops();
    SetBuses();
    router_ = new graph::Router<Weight>(graph_);
  }

  ~TransportRouter() {
    delete router_;
  }

  std::optional<RouteInfo> ComputeRoute(std::string_view stopname_from,
                                        std::string_view stopname_to) const {
    auto route = router_->BuildRoute(vertex_id_to_name_.at(stopname_from).out,
                                     vertex_id_to_name_.at(stopname_to).out);
    RouteInfo result;
    if (!route) {
      return std::nullopt;
    }
    result.total_time = route->weight.count();
    for (graph::EdgeId edge_id : route->edges) {
      graph::Edge edge = graph_.GetEdge(edge_id);
      if (edge_id < graph_.GetVertexCount() / 2) {
        WaitSection track_section;
        track_section.time = edge.weight.count();
        track_section.stop_name = vertexes_.at(edge.from);

        result.detail.push_back(std::move(track_section));
      } else {
        BusSection track_section;
        track_section.time = edge.weight.count();
        track_section.bus_name = track_section_to_edge_id_.at(edge_id).first;
        track_section.span_count = track_section_to_edge_id_.at(edge_id).second;

        result.detail.push_back(std::move(track_section));
      }
    }
    return result;
  }

 private:
  void SetStops() {
    using namespace graph;

    const std::deque<Stop>& stops = db_.GetStops();
    int stops_count = stops.size();
    vertexes_.resize(stops_count * 2);
    graph_ = graph::DirectedWeightedGraph<Weight>(stops_count * 2);
    for (int id = 0; const Stop& stop : stops) {
      VertexId id_input = id++;
      VertexId id_output = id++;
      vertex_id_to_name_.emplace(stop.name, VertexIdPair{id_input, id_output});
      vertexes_[id_input] = stop.name;
      vertexes_[id_output] = stop.name;

      Edge edge{id_output, id_input, Weight(bus_interval_.count())};
      graph_.AddEdge(std::move(edge));
    }
    track_section_to_edge_id_.resize(stops_count);
  }

  void SetBuses() {
    using namespace graph;

    const std::deque<Bus>& buses = db_.GetBuses();
    for (const auto& [name, stops] : buses) {
      if (stops.size() <= 1) {
        continue;
      }
      for (size_t id_stop_from = 0; id_stop_from + 1 < stops.size(); ++id_stop_from) {
        const Stop* stop_from = stops.at(id_stop_from);
        VertexIdPair id_vertex_from = vertex_id_to_name_.at(stop_from->name);

        double distance = 0;
        const Stop* prev_stop = stop_from;
        for (size_t id_stop_to = id_stop_from + 1; id_stop_to < stops.size(); ++id_stop_to) {
          const Stop* stop_to = stops.at(id_stop_to);
          // считаем дистанцию, прибавляя новые участки
          distance += db_.GetDistance(prev_stop->name, stop_to->name);
          VertexIdPair id_vertex_to = vertex_id_to_name_.at(stop_to->name);
          // Добавляем грань
          Edge edge{id_vertex_from.in, id_vertex_to.out, Weight(distance * 1. / bus_velocity_)};
          EdgeId id = graph_.AddEdge(edge);
          int span_count = id_stop_to - id_stop_from;
          track_section_to_edge_id_.emplace_back(name, span_count);
          assert(track_section_to_edge_id_.size() - 1 == id);

          prev_stop = stop_to;
        }
      }
    }
  }

 private:
  const TransportCatalogue& db_;
  std::unordered_map<std::string_view, VertexIdPair> vertex_id_to_name_;
  std::vector<std::string_view> vertexes_;
  std::vector<std::pair<std::string_view, int>> track_section_to_edge_id_;
  graph::DirectedWeightedGraph<Weight> graph_;
  graph::Router<Weight>* router_;
  Weight bus_interval_;
  double bus_velocity_;
};