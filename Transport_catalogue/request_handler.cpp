#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

RequestHandler::RequestHandler(const TransportCatalogue& db) : db_(db) {}

void RequestHandler::RenderMap(std::ostream& os) const {
  const std::deque<Bus>& raw_buses = db_.GetBuses();

  MapRenderer::BusDB sorted_buses;
  for (const auto& bus : raw_buses) {
    if (!bus.path.empty())
      sorted_buses.insert({&bus, db_.IsRoundBus(bus.name)});
  }

  renderer_.Draw(sorted_buses, os);
}

void RequestHandler::SetRenderer(const MapRenderer& renderer) {
  renderer_ = renderer;
}