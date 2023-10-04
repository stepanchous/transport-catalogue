#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "geo.h"

namespace trc {

struct Stop {
    Stop() = default;

    Stop(const std::string& name, geo::Coordinates coordinates);

    uint32_t id;
    std::string name;
    geo::Coordinates coordinates;

   private:
    static uint32_t ID;
};

struct Bus {
    std::string name = {};
    std::vector<const Stop*> route = {};
    double route_length = 0.0;
    double curvature = 0.0;
    bool is_roundtrip = false;
};

bool operator<(const Bus& lhs, const Bus& rhs);

struct StopPtrCompare {
    bool operator()(const Stop* lhs, const Stop* rhs) const;
};

struct StopPairHasher {
    size_t operator()(
        const std::pair<const Stop*, const Stop*>& stop_pair) const;
};

}  // namespace trc
