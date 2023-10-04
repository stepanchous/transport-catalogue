#pragma once

#include <transport_catalogue.pb.h>

#include <filesystem>

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace trc {

struct SerializationSettings {
    using Path = std::filesystem::path;

    Path file;
};

class Serializer {
   public:
    struct Data {
        TransportCatalogue trc;
        render::RenderSettings render_settings;
        TransportRouter::Settings router_settings;
    };

    Serializer(const SerializationSettings& settings);

    void Save(const TransportCatalogue& transport_catalogue,
              const render::RenderSettings& render_settings,
              const TransportRouter::Settings& router_settings) const;

    Data Load() const;

   private:
    SerializationSettings settings_;

   private:
    static trc_serialization::SerializationData Convert(
        const TransportCatalogue& trc, const render::RenderSettings& rs,
        const TransportRouter::Settings& router_settings);
    static Data Convert(const trc_serialization::SerializationData& ser_data);

    static trc_serialization::TransportCatalogue Convert(
        const TransportCatalogue& trc);
    static TransportCatalogue Convert(
        const trc_serialization::TransportCatalogue& ser_trc);

    static trc_serialization::RenderSettings Convert(
        const render::RenderSettings& rs);
    static render::RenderSettings Convert(
        const trc_serialization::RenderSettings& ser_rs);

    static trc_serialization::RouterSettings Convert(
        TransportRouter::Settings rs);
    static TransportRouter::Settings Convert(
        const trc_serialization::RouterSettings& ser_rs);

    static trc_serialization::Stop Convert(const Stop& s);
    static Stop Convert(const trc_serialization::Stop& ser_s);

    static trc_serialization::Coordinates Convert(geo::Coordinates c);
    static geo::Coordinates Convert(
        const trc_serialization::Coordinates& ser_c);

    static trc_serialization::StopDistances Convert(
        const std::pair<const Stop*, std::unordered_map<const Stop*, double>>&
            stop_distances);
    static std::pair<const Stop*, std::unordered_map<const Stop*, double>>
    Convert(const trc_serialization::StopDistances& ser_sd,
            const std::unordered_map<uint32_t, const Stop*>& stop_id_to_stop);

    static trc_serialization::DistanceInfo Convert(
        std::pair<const Stop*, double> di);
    static std::pair<const Stop*, double> Convert(
        const trc_serialization::DistanceInfo& ser_di,
        const std::unordered_map<uint32_t, const Stop*>& stop_id_to_stop);

    static trc_serialization::Bus Convert(const Bus& b);
    static Bus Convert(
        const trc_serialization::Bus& ser_b,
        const std::unordered_map<uint32_t, const Stop*>& stop_id_to_stop);

    static trc_serialization::Point Convert(svg::Point p);
    static svg::Point Convert(const trc_serialization::Point& ser_p);

    static trc_serialization::Color Convert(const svg::Color& c);
    static svg::Color Convert(const trc_serialization::Color& ser_c);

    static trc_serialization::Rgb Convert(svg::Rgb rgb);
    static svg::Rgb Convert(const trc_serialization::Rgb& ser_rgb);

    static trc_serialization::Rgba Convert(svg::Rgba rgba);
    static svg::Rgba Convert(const trc_serialization::Rgba& ser_rgba);
};

}  // namespace trc
