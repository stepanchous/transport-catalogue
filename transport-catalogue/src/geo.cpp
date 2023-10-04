#include "geo.h"

namespace trc::geo {

bool Coordinates::operator==(const Coordinates other) const {
    return lat == other.lat && lng == other.lng;
}

bool Coordinates::operator!=(const Coordinates other) const {
    return !(*this == other);
}

}  // namespace trc::geo
