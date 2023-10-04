#include "request_handler.h"

#include <iterator>
#include <map>
#include <string>
#include <variant>

#include "json_builder.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace trc::rh {

using namespace std;

// BaseRequestHandler
BaseRequestHandler::BaseRequestHandler(const io::JsonReader& json_reader)
    : json_reader_(json_reader) {}

TransportCatalogue BaseRequestHandler::BuildTransportCatalogue() {
    TransportCatalogue transport_catalogue;

    AddStops(transport_catalogue);
    AddBuses(transport_catalogue);

    return transport_catalogue;
}

void BaseRequestHandler::AddStops(TransportCatalogue& transport_catalogue) {
    vector<io::AddStopRequest> stop_requests = json_reader_.GetStops();

    for (const auto& stop_request : stop_requests) {
        Stop stop = {stop_request.name,
                     {stop_request.latitude, stop_request.longitude}};

        transport_catalogue.AddStop(std::move(stop));
    }

    for (const auto& stop_request : stop_requests) {
        const string& stop_from = stop_request.name;

        for (const auto& [stop_to, distance] : stop_request.road_distances) {
            transport_catalogue.AddDistance(stop_from, stop_to, distance);
        }
    }
}

void BaseRequestHandler::AddBuses(TransportCatalogue& transport_catalogue) {
    for (const auto& bus_request : json_reader_.GetBuses()) {
        transport_catalogue.AddBus(bus_request.name, bus_request.stops,
                                   bus_request.is_roundtrip);
    }
}

// StatRequestHandler
StatRequestHandler::StatRequestHandler(
    const TransportCatalogue& transport_catalogue,
    render::MapRenderer& map_renderer, const TransportRouter& transport_router,
    const io::JsonReader& json_reader, ostream& output)
    : transport_catalogue_(transport_catalogue),
      json_reader_(json_reader),
      map_renderer_(map_renderer),
      router_(transport_router),
      output_(output) {}

void StatRequestHandler::HandleStatRequests() {
    vector<io::StatRequest> stat_requests = json_reader_.GetStatRequests();

    StatHandler stat_handler(transport_catalogue_, map_renderer_, router_,
                             output_);

    for (const auto& stat_request : stat_requests) {
        std::visit(stat_handler, stat_request);
    }

    stat_handler.Print();
}

StatRequestHandler::StatHandler::StatHandler(
    const TransportCatalogue& transport_catalogue,
    render::MapRenderer& map_renderer, const TransportRouter& router,
    std::ostream& output)
    : transport_catalogue_(transport_catalogue),
      map_renderer_(map_renderer),
      router_(router),
      output_(output) {}

void StatRequestHandler::StatHandler::operator()(
    const io::GetStopRequest& get_stop_request) {
    optional<set<string>> stop_info =
        transport_catalogue_.GetStopInfo(get_stop_request.name);

    if (stop_info.has_value()) {
        json::Array buses(std::make_move_iterator(stop_info->begin()),
                          std::make_move_iterator(stop_info->end()));
        // clang-format off
        responses_.push_back(
            json::Builder{}
                .StartDict()
                    .Key(io::BUSES_FIELD).Value(std::move(buses))
                    .Key(io::REQUEST_ID_FIELD).Value(get_stop_request.id)
                .EndDict().Build());
        // clang-format on
    } else {
        HandleNotFound(get_stop_request.id);
    }
}

void StatRequestHandler::StatHandler::operator()(
    const io::GetBusRequest& get_bus_request) {
    std::optional<TransportCatalogue::BusInfo> bus_info =
        transport_catalogue_.GetBusInfo(get_bus_request.name);

    if (bus_info.has_value()) {
        // clang-format off
        responses_.push_back(
            json::Builder{}
                .StartDict()
                    .Key(io::CURVATURE_FIELD)
                        .Value(bus_info->curvature)
                    .Key(io::REQUEST_ID_FIELD)
                        .Value(get_bus_request.id)
                    .Key(io::ROUTE_LENGTH_FIELD)
                        .Value(bus_info->route_length)
                    .Key(io::STOP_COUNT_FIELD)
                        .Value(static_cast<int>(bus_info->stop_count))
                    .Key(io::UNIQUE_STOP_COUNT_FIELD)
                        .Value(static_cast<int>(bus_info->unique_stop_count))
                .EndDict().Build());
        // clang-format on
    } else {
        HandleNotFound(get_bus_request.id);
    }
}

void StatRequestHandler::StatHandler::operator()(
    const io::GetMapRequest& get_map_request) {
    ostringstream svg_document;

    map_renderer_.Render(transport_catalogue_.GetBuses(), svg_document);

    // clang-format off
    responses_.push_back(
        json::Builder{}
           .StartDict()
               .Key(io::MAP_FIELD)
                   .Value(svg_document.str())
               .Key(io::REQUEST_ID_FIELD)
                   .Value(get_map_request.id)
            .EndDict().Build());
    // clang-format on
}

void StatRequestHandler::StatHandler::operator()(
    const io::GetRouteRequest& get_route_request) {
    const auto route_info = router_.BuildRoute(get_route_request.from_stop,
                                               get_route_request.to_stop);

    if (route_info.has_value()) {
        json::Array items;
        ItemVisitor item_visitor(items);

        for (const auto& item : route_info->items) {
            std::visit(item_visitor, item);
        }

        // clang-format off
        responses_.push_back(
                json::Builder{} 
                    .StartDict()
                        .Key(io::ITEMS_FIELD)
                            .Value(items)
                        .Key(io::TOTAL_TIME_FIELD)
                            .Value(route_info->total_time)
                        .Key(io::REQUEST_ID_FIELD)
                            .Value(get_route_request.id)
                    .EndDict().Build());
        //clang-format on

    } else {
        HandleNotFound(get_route_request.id);
    }
}

void StatRequestHandler::StatHandler::operator()(const io::UnknownRequest&) {
    responses_.push_back("Unknown request");
}

void StatRequestHandler::StatHandler::Print() {
    if (!responses_.empty()) {
        json::Print(json::Document{responses_}, output_);
    }
}

void StatRequestHandler::StatHandler::HandleNotFound(int id) {
    // clang-format off
    responses_.push_back(
            json::Builder{}
               .StartDict()
                   .Key(io::REQUEST_ID_FIELD)
                       .Value(id)
                   .Key(io::ERROR_MESSAGE_FIELD)
                       .Value(io::NOT_FOUND)
                .EndDict().Build());
    // clang-format on
}

StatRequestHandler::StatHandler::ItemVisitor::ItemVisitor(json::Array& items)
    : items_(items) {}

void StatRequestHandler::StatHandler::ItemVisitor::operator()(
    const TransportRouter::WaitItem& wait_item) {
    // clang-format off
    items_.push_back(
            json::Builder{}
                .StartDict()
                    .Key(io::STOP_NAME_FIELD)
                        .Value(std::string(wait_item.stop_name))
                    .Key(io::TIME_FIELD)
                        .Value(wait_item.time)
                    .Key(io::TYPE_FIELD)
                        .Value(io::WAIT_FIELD)
                .EndDict().Build());
    // clang-format on
}

void StatRequestHandler::StatHandler::ItemVisitor::operator()(
    const TransportRouter::BusItem& bus_item) {
    // clang-format off
    items_.push_back(
            json::Builder{}
                .StartDict()
                    .Key(io::BUS_FIELD)
                        .Value(std::string(bus_item.bus_name))
                    .Key(io::SPAN_COUNT_FIELD)
                        .Value(static_cast<int>(bus_item.span_count))
                    .Key(io::TIME_FIELD)
                        .Value(bus_item.time)
                    .Key(io::TYPE_FIELD)
                        .Value(io::BUS_TYPE_FIELD)
                .EndDict().Build());
    // clang-format on
}

}  // namespace trc::rh
