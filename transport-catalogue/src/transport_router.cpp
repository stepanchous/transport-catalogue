#include "transport_router.h"

namespace trc {

TransportRouter::TransportRouter(
    const TransportRouter::Settings& router_settings,
    const TransportCatalogue& transport_catalogue)
    : transport_catalogue_(transport_catalogue),
      router_settings_(router_settings),
      stop_id_to_stop_(InitIdToStop(transport_catalogue)),
      stop_name_to_index_(InitStopNameToId(stop_id_to_stop_)),
      transport_graph_(BuildGraph()),
      router_(transport_graph_) {}

std::optional<TransportRouter::RouteInfo> TransportRouter::BuildRoute(
    const std::string& from_stop, const std::string& to_stop) const {
    graph::VertexId route_start =
        stop_name_to_index_.at(from_stop) + stop_id_to_stop_.size();

    graph::VertexId route_end =
        stop_name_to_index_.at(to_stop) + stop_id_to_stop_.size();

    auto route_info_raw = router_.BuildRoute(route_start, route_end);

    if (!route_info_raw.has_value()) {
        return std::nullopt;
    }

    RouteInfo route_info;

    route_info.items.reserve(route_info_raw->edges.size());
    route_info.total_time = route_info_raw->weight.time;

    size_t stop_count = transport_catalogue_.GetStopCount();

    for (graph::EdgeId edge_id : route_info_raw->edges) {
        const auto& edge = transport_graph_.GetEdge(edge_id);

        if (edge.weight.span_count == 0) {
            auto edge_id =
                edge.to >= stop_count ? edge.to - stop_count : edge.to;

            route_info.items.push_back(
                WaitItem{stop_id_to_stop_.at(edge_id)->name, edge.weight.time});
        } else {
            route_info.items.push_back(BusItem{edge.weight.bus_name,
                                               edge.weight.span_count,
                                               edge.weight.time});
        }
    }

    return {std::move(route_info)};
}

graph::DirectedWeightedGraph<TransportRouter::Weight>
TransportRouter::BuildGraph() {
    graph::DirectedWeightedGraph<Weight> graph(2 * stop_id_to_stop_.size());

    for (size_t stop_id = 0; stop_id < stop_id_to_stop_.size(); ++stop_id) {
        graph::Edge<Weight> start_wait_to_bus_enter{
            stop_id + stop_id_to_stop_.size(),  // from
            stop_id,                            // to
            {router_settings_.bus_wait_time, 0, ""}};

        graph.AddEdge(start_wait_to_bus_enter);
    }

    const auto& buses = transport_catalogue_.GetBuses();

    for (const Bus& bus : buses) {
        AddRouteToGraph(bus, graph);
    }

    return graph;
}

void TransportRouter::AddRouteToGraph(
    const Bus& bus, graph::DirectedWeightedGraph<Weight>& graph) {
    if (bus.is_roundtrip) {
        AddRoundTrip(bus, graph);
    } else {
        AddLinearTrip(bus, graph);
    }
}

void TransportRouter::AddRoundTrip(
    const Bus& bus, graph::DirectedWeightedGraph<Weight>& graph) {
    for (size_t i = 0; i < bus.route.size(); ++i) {
        double accumulated_distance = 0.0;

        for (size_t j = i + 1; j < bus.route.size(); ++j) {
            accumulated_distance += transport_catalogue_.GetDistance(
                bus.route.at(j - 1), bus.route.at(j));

            size_t span_count = j - i;

            graph::Edge<Weight> edge_to_stop_exit{
                stop_name_to_index_.at(bus.route[i]->name),
                stop_name_to_index_.at(bus.route[j]->name) +
                    stop_id_to_stop_.size(),
                {CalculateDriveTimeMinutes(accumulated_distance), span_count,
                 bus.name}};

            graph.AddEdge(edge_to_stop_exit);
        }
    }
}

void TransportRouter::AddLinearTrip(
    const Bus& bus, graph::DirectedWeightedGraph<Weight>& graph) {
    size_t mid_stop = bus.route.size() / 2 + 1;

    for (size_t i = 0; i < mid_stop; ++i) {
        double accumulated_distance = 0.0;
        double accumulated_distance_reverse = 0.0;

        for (size_t j = i + 1; j < mid_stop; ++j) {
            accumulated_distance += transport_catalogue_.GetDistance(
                bus.route.at(j - 1), bus.route.at(j));

            accumulated_distance_reverse += transport_catalogue_.GetDistance(
                bus.route.at(j), bus.route.at(j - 1));

            size_t span_count = j - i;

            graph::Edge<Weight> edge_to_stop_exit{
                stop_name_to_index_.at(bus.route[i]->name),
                stop_name_to_index_.at(bus.route[j]->name) +
                    stop_id_to_stop_.size(),
                {CalculateDriveTimeMinutes(accumulated_distance), span_count,
                 bus.name}};

            graph::Edge<Weight> edge_to_stop_exit_reverse{
                stop_name_to_index_.at(bus.route[j]->name),
                stop_name_to_index_.at(bus.route[i]->name) +
                    stop_id_to_stop_.size(),
                {CalculateDriveTimeMinutes(accumulated_distance_reverse),
                 span_count, bus.name}};

            graph.AddEdge(edge_to_stop_exit);
            graph.AddEdge(edge_to_stop_exit_reverse);
        }
    }
}

std::unordered_map<std::string_view, size_t> TransportRouter::InitStopNameToId(
    const std::vector<const Stop*>& stop_id_to_stop) {
    std::unordered_map<std::string_view, size_t> stop_name_to_index;

    for (size_t i = 0; i < stop_id_to_stop.size(); ++i) {
        stop_name_to_index[stop_id_to_stop[i]->name] = i;
    }

    return stop_name_to_index;
}

std::vector<const Stop*> TransportRouter::InitIdToStop(
    const TransportCatalogue& transport_catalogue) {
    const auto& stops = transport_catalogue.GetStops();

    std::vector<const Stop*> stop_id_to_stop(stops.size());

    int index = 0;

    for (const auto& stop : stops) {
        stop_id_to_stop[index] = &stop;
        ++index;
    }

    return stop_id_to_stop;
}

double TransportRouter::CalculateDriveTimeMinutes(double distance) {
    double distance_km = distance / 1000;

    double time_h = distance_km / router_settings_.bus_velocity;

    return time_h * 60;
}

TransportRouter::EdgeInfo::EdgeInfo(double time, size_t span_count,
                                    std::string_view bus_name)
    : time(time), span_count(span_count), bus_name(bus_name) {}

TransportRouter::EdgeInfo::EdgeInfo(double time) : time(time) {}

bool TransportRouter::EdgeInfo::operator<(const EdgeInfo& other) const {
    return this->time < other.time;
}

bool TransportRouter::EdgeInfo::operator>(const EdgeInfo& other) const {
    return this->time > other.time;
}

double TransportRouter::EdgeInfo::operator+(
    const TransportRouter::EdgeInfo& other) const {
    return this->time + other.time;
}

}  // namespace trc
