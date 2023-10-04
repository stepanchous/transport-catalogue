#pragma once

#include <string>
#include <unordered_map>
#include <variant>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace trc {

class TransportRouter {
   public:
    struct Settings {
        double bus_wait_time{0.0};
        double bus_velocity{0.0};
    };

    struct EdgeInfo {
        double time{0.0};
        size_t span_count{0};
        std::string_view bus_name;

        EdgeInfo() = default;

        EdgeInfo(double time, size_t span_count, std::string_view bus_name);

        EdgeInfo(double time);

        bool operator<(const EdgeInfo& other) const;

        bool operator>(const EdgeInfo& other) const;

        double operator+(const EdgeInfo& other) const;
    };

    using Weight = EdgeInfo;

    struct WaitItem {
        std::string_view stop_name;
        double time;
    };

    struct BusItem {
        std::string_view bus_name;
        size_t span_count;
        double time;
    };

    using Item = std::variant<WaitItem, BusItem>;

    struct RouteInfo {
        std::vector<Item> items;
        double total_time;
    };

    TransportRouter(const Settings& router_settings,
                    const TransportCatalogue& transport_catalogue);

    std::optional<RouteInfo> BuildRoute(const std::string& from_stop,
                                        const std::string& to_stop) const;

   private:
    const trc::TransportCatalogue& transport_catalogue_;
    Settings router_settings_;

    std::vector<const Stop*> stop_id_to_stop_;
    std::unordered_map<std::string_view, size_t> stop_name_to_index_;
    graph::DirectedWeightedGraph<Weight> transport_graph_;
    graph::Router<Weight> router_;

    graph::DirectedWeightedGraph<Weight> BuildGraph();

    void InitRouter();

    static std::vector<const Stop*> InitIdToStop(
        const TransportCatalogue& transport_catalogue);

    static std::unordered_map<std::string_view, size_t> InitStopNameToId(
        const std::vector<const Stop*>& stop_id_to_stop);

    void AddRouteToGraph(const Bus& bus,
                         graph::DirectedWeightedGraph<Weight>& graph);

    void AddRoundTrip(const Bus& bus,
                      graph::DirectedWeightedGraph<Weight>& graph);

    void AddLinearTrip(const Bus& bus,
                       graph::DirectedWeightedGraph<Weight>& graph);

    double CalculateDriveTimeMinutes(double distance);
};

}  // namespace trc
