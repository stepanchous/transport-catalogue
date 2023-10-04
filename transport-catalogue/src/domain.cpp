#include "domain.h"

namespace trc {

uint32_t Stop::ID = 0;

Stop::Stop(const std::string& name, geo::Coordinates coordinates)
    : id(ID++), name(name), coordinates(coordinates) {}

size_t StopPairHasher::operator()(
    const std::pair<const Stop*, const Stop*>& stop_pair) const {
    const int prime_number = 47;
    return reinterpret_cast<size_t>(stop_pair.first) +
           prime_number * reinterpret_cast<size_t>(stop_pair.second);
}

bool operator<(const Bus& lhs, const Bus& rhs) { return lhs.name < rhs.name; }

bool StopPtrCompare::operator()(const Stop* lhs, const Stop* rhs) const {
    return lhs->name < rhs->name;
}

}  // namespace trc
