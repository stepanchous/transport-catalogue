#pragma once

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "domain.h"

namespace trc {

class TransportCatalogue {
   public:
    struct BusInfo {
        size_t stop_count;
        size_t unique_stop_count;
        double route_length;
        double curvature;
    };

    TransportCatalogue() = default;

    void AddStop(Stop&& stop);

    void AddBus(const std::string& bus_name,
                const std::vector<std::string>& route, bool is_roundtrip);

    void AddBus(const Bus& bus);

    void AddDistance(const std::string& stop_from, const std::string& stop_to,
                     double distance);

    std::optional<std::set<std::string>> GetStopInfo(
        const std::string& stop_name) const;

    std::optional<BusInfo> GetBusInfo(const std::string& bus_name) const;

    const Stop* GetStopByName(const std::string& stop_name) const;

    const std::deque<Bus>& GetBuses() const;

    const std::deque<Stop>& GetStops() const;

    size_t GetStopCount() const;

    const std::unordered_map<const Stop*,
                             std::unordered_map<const Stop*, double>>&
    GetDistances() const;

    double GetDistance(const Stop* from, const Stop* to) const;

    const std::unordered_map<uint32_t, const Stop*>& GetStopIdToStop() const;

   private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;

    std::unordered_map<uint32_t, const Stop*> stop_id_to_stop_;
    std::unordered_map<std::string_view, const Stop*> stop_name_to_stop_;
    std::unordered_map<std::string_view, const Bus*> bus_name_to_bus_;
    std::unordered_map<const Stop*, std::unordered_map<const Stop*, double>>
        stop_to_stop_to_distance_;
    std::unordered_map<std::string_view, std::vector<const Bus*>>
        stop_name_to_buses_;

    double ComputeRouteLength(const Bus& bus) const;

    double ComputeRouteGeographicLength(const Bus& bus) const;
};
}  // namespace trc
