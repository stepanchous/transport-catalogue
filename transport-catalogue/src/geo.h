#pragma once

#include <cmath>
#include <iostream>

namespace trc {

namespace geo {

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates other) const;
    bool operator!=(const Coordinates other) const;
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = 3.1415926535 / 180.;
    static const double earth_r = 6371'000.0;
    return acos(sin(from.lat * dr) * sin(to.lat * dr) +
                cos(from.lat * dr) * cos(to.lat * dr) *
                    cos(abs(from.lng - to.lng) * dr)) *
           earth_r;
}

}  // namespace geo
}  // namespace trc
