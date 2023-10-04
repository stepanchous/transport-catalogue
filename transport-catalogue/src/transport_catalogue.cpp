#include "transport_catalogue.h"

#include <cmath>
#include <numeric>
#include <unordered_set>

#include "geo.h"

namespace trc {

using namespace std;

void TransportCatalogue::AddStop(Stop&& stop) {
    stops_.push_front(std::move(stop));
    stop_id_to_stop_[stop.id] = &stops_.front();
    stop_name_to_stop_[stops_.front().name] = &stops_.front();
    stop_name_to_buses_[stops_.front().name];
}

void TransportCatalogue::AddBus(const string& bus_name,
                                const vector<string>& route,
                                bool is_roundtrip = false) {
    Bus bus{bus_name};
    bus.is_roundtrip = is_roundtrip;
    bus.route.reserve(route.size());

    for (const auto& stop : route) {
        bus.route.push_back(stop_name_to_stop_.at(stop));
    }

    if (!bus.is_roundtrip) {
        for (int i = bus.route.size() - 2; i >= 0; i--) {
            bus.route.push_back(bus.route[i]);
        }
    }

    bus.route_length = ComputeRouteLength(bus);
    bus.curvature = bus.route_length / ComputeRouteGeographicLength(bus);
    buses_.push_front(std::move(bus));
    bus_name_to_bus_[buses_.front().name] = &buses_.front();

    for (const Stop* stop : buses_.front().route) {
        stop_name_to_buses_[stop->name].push_back(&buses_.front());
    }
}

void TransportCatalogue::AddBus(const Bus& bus) {
    buses_.push_front(bus);
    bus_name_to_bus_[buses_.front().name] = &buses_.front();

    for (const Stop* stop : buses_.front().route) {
        stop_name_to_buses_[stop->name].push_back(&buses_.front());
    }
}

void TransportCatalogue::AddDistance(const string& stop_from,
                                     const string& stop_to, double distance) {
    const Stop* from = stop_name_to_stop_.at(stop_from);
    const Stop* to = stop_name_to_stop_.at(stop_to);

    stop_to_stop_to_distance_[from][to] = distance;
}

optional<set<string>> TransportCatalogue::GetStopInfo(
    const string& stop_name) const {
    if (!stop_name_to_buses_.count(stop_name)) {
        return {};
    }

    set<string> buses;
    for (const Bus* bus : stop_name_to_buses_.at(stop_name)) {
        buses.insert(bus->name);
    }

    return {std::move(buses)};
}

optional<TransportCatalogue::BusInfo> TransportCatalogue::GetBusInfo(
    const string& bus_name) const {
    if (!bus_name_to_bus_.count(bus_name)) {
        return {};
    }

    const Bus& bus = *bus_name_to_bus_.at(bus_name);

    size_t unique_stops_count =
        unordered_set(bus.route.begin(), bus.route.end()).size();

    return {{bus.route.size(), unique_stops_count, bus.route_length,
             bus.curvature}};
}

const Stop* TransportCatalogue::GetStopByName(const string& stop_name) const {
    return stop_name_to_stop_.at(stop_name);
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const { return buses_; }

const std::deque<Stop>& TransportCatalogue::GetStops() const { return stops_; }

size_t TransportCatalogue::GetStopCount() const { return stops_.size(); }

double TransportCatalogue::ComputeRouteLength(const Bus& bus) const {
    if (bus.route.size() == 0) {
        return 0.0;
    }

    if (bus.route.size() == 1) {
        if (stop_to_stop_to_distance_.count(bus.route[0]) &&
            stop_to_stop_to_distance_.at(bus.route[0]).count(bus.route[0])) {
            return stop_to_stop_to_distance_.at(bus.route[0]).at(bus.route[0]);
        }

        return 0.0;
    }

    double route_length = 0.0;

    for (size_t i = 1; i < bus.route.size(); ++i) {
        route_length += GetDistance(bus.route[i - 1], bus.route[i]);
    }

    return route_length;
}

double TransportCatalogue::ComputeRouteGeographicLength(const Bus& bus) const {
    if (bus.route.size() < 2) {
        return 0.0;
    }

    double route_geo_length = 0.0;
    for (size_t i = 1; i < bus.route.size(); ++i) {
        route_geo_length += ComputeDistance(bus.route[i - 1]->coordinates,
                                            bus.route[i]->coordinates);
    }

    return route_geo_length;
}

const std::unordered_map<const Stop*, std::unordered_map<const Stop*, double>>&
TransportCatalogue::GetDistances() const {
    return stop_to_stop_to_distance_;
}

const std::unordered_map<uint32_t, const Stop*>&
TransportCatalogue::GetStopIdToStop() const {
    return stop_id_to_stop_;
}

double TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
    if (stop_to_stop_to_distance_.count(from) &&
        stop_to_stop_to_distance_.at(from).count(to)) {
        return stop_to_stop_to_distance_.at(from).at(to);
    }

    if (stop_to_stop_to_distance_.count(to) &&
        stop_to_stop_to_distance_.at(to).count(from)) {
        return stop_to_stop_to_distance_.at(to).at(from);
    }

    return ComputeDistance(from->coordinates, to->coordinates);
}

}  // namespace trc
