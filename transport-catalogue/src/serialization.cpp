#include "serialization.h"

#include <fstream>
#include <unordered_map>

namespace trc {

Serializer::Serializer(const SerializationSettings& settings)
    : settings_(settings) {}

void Serializer::Save(const TransportCatalogue& transport_catalogue,
                      const render::RenderSettings& render_settings,
                      const TransportRouter::Settings& router_settings) const {
    std::ofstream output(settings_.file, std::ios::binary);
    Convert(transport_catalogue, render_settings, router_settings)
        .SerializeToOstream(&output);
}

Serializer::Data Serializer::Load() const {
    trc_serialization::SerializationData ser_data;

    std::ifstream input(settings_.file, std::ios::binary);

    ser_data.ParseFromIstream(&input);

    return Convert(ser_data);
}

trc_serialization::SerializationData Serializer::Convert(
    const TransportCatalogue& trc, const render::RenderSettings& rs,
    const TransportRouter::Settings& router_settings) {
    trc_serialization::SerializationData ser_data;

    *ser_data.mutable_transport_catalogue() = Convert(trc);
    *ser_data.mutable_render_settings() = Convert(rs);
    *ser_data.mutable_router_settings() = Convert(router_settings);

    return ser_data;
}

Serializer::Data Serializer::Convert(
    const trc_serialization::SerializationData& ser_data) {
    Data data;

    data.trc = Convert(ser_data.transport_catalogue());
    data.render_settings = Convert(ser_data.render_settings());
    data.router_settings = Convert(ser_data.router_settings());

    return data;
}

trc_serialization::TransportCatalogue Serializer::Convert(
    const TransportCatalogue& trc) {
    trc_serialization::TransportCatalogue ser_trc;

    const auto& stops = trc.GetStops();
    for_each(stops.begin(), stops.end(), [&ser_trc](const Stop& stop) {
        *ser_trc.add_stop() = Convert(stop);
    });

    const auto& stop_to_stop_to_distance = trc.GetDistances();
    std::for_each(
        stop_to_stop_to_distance.begin(), stop_to_stop_to_distance.end(),
        [&ser_trc](const std::pair<const Stop*,
                                   std::unordered_map<const Stop*, double>>&
                       stop_distances) {
            *ser_trc.add_distance() = Convert(stop_distances);
        });

    const auto& buses = trc.GetBuses();
    std::for_each(buses.begin(), buses.end(), [&ser_trc](const Bus& bus) {
        *ser_trc.add_bus() = Convert(bus);
    });

    return ser_trc;
}

TransportCatalogue Serializer::Convert(
    const trc_serialization::TransportCatalogue& ser_trc) {
    TransportCatalogue trc;

    for (size_t i = 0; i < ser_trc.stop_size(); ++i) {
        trc.AddStop(Convert(ser_trc.stop(i)));
    }

    for (size_t i = 0; i < ser_trc.distance_size(); ++i) {
        auto [from_stop, to_stop_to_distance] =
            Convert(ser_trc.distance(i), trc.GetStopIdToStop());
        for (const auto [to_stop, distance] : to_stop_to_distance) {
            trc.AddDistance(from_stop->name, to_stop->name, distance);
        }
    }

    for (size_t i = 0; i < ser_trc.bus_size(); ++i) {
        trc.AddBus(Convert(ser_trc.bus(i), trc.GetStopIdToStop()));
    }

    return trc;
}

trc_serialization::RenderSettings Serializer::Convert(
    const render::RenderSettings& rs) {
    trc_serialization::RenderSettings ser_rs;

    ser_rs.set_width(rs.width);
    ser_rs.set_height(rs.height);
    ser_rs.set_padding(rs.padding);
    ser_rs.set_line_width(rs.line_width);
    ser_rs.set_stop_radius(rs.stop_radius);
    ser_rs.set_bus_label_font_size(rs.bus_label_font_size);
    *ser_rs.mutable_bus_label_offset() = Convert(rs.bus_label_offset);
    ser_rs.set_stop_label_font_size(rs.stop_label_font_size);
    *ser_rs.mutable_stop_label_offset() = Convert(rs.stop_label_offset);
    *ser_rs.mutable_underlayer_color() = Convert(rs.underlayer_color);
    ser_rs.set_underlayer_width(rs.underlayer_width);

    std::for_each(rs.color_palette.begin(), rs.color_palette.end(),
                  [&ser_rs](const svg::Color& c) {
                      *ser_rs.add_color_palette() = Convert(c);
                  });

    return ser_rs;
}

render::RenderSettings Serializer::Convert(
    const trc_serialization::RenderSettings& ser_rs) {
    render::RenderSettings rs;

    rs.width = ser_rs.width();
    rs.height = ser_rs.height();
    rs.padding = ser_rs.padding();
    rs.line_width = ser_rs.line_width();
    rs.stop_radius = ser_rs.stop_radius();
    rs.bus_label_font_size = ser_rs.bus_label_font_size();
    rs.bus_label_offset = Convert(ser_rs.bus_label_offset());
    rs.stop_label_font_size = ser_rs.stop_label_font_size();
    rs.stop_label_offset = Convert(ser_rs.stop_label_offset());
    rs.underlayer_color = Convert(ser_rs.underlayer_color());
    rs.underlayer_width = ser_rs.underlayer_width();

    rs.color_palette.reserve(ser_rs.color_palette_size());

    for (size_t i = 0; i < ser_rs.color_palette_size(); ++i) {
        rs.color_palette.push_back(Convert(ser_rs.color_palette(i)));
    }

    return rs;
}

trc_serialization::RouterSettings Serializer::Convert(
    TransportRouter::Settings rs) {
    trc_serialization::RouterSettings ser_rs;

    ser_rs.set_bus_wait_time(rs.bus_wait_time);
    ser_rs.set_bus_velocity(rs.bus_velocity);

    return ser_rs;
}

TransportRouter::Settings Serializer::Convert(
    const trc_serialization::RouterSettings& ser_rs) {
    TransportRouter::Settings rs;

    rs.bus_wait_time = ser_rs.bus_wait_time();
    rs.bus_velocity = ser_rs.bus_velocity();

    return rs;
}

trc_serialization::Stop Serializer::Convert(const Stop& s) {
    trc_serialization::Stop ser_s;

    ser_s.set_id(s.id);
    ser_s.set_name(s.name);
    *ser_s.mutable_coordinates() = Convert(s.coordinates);

    return ser_s;
}

Stop Serializer::Convert(const trc_serialization::Stop& ser_s) {
    Stop s;

    s.id = ser_s.id();
    s.name = ser_s.name();
    s.coordinates = Convert(ser_s.coordinates());

    return s;
}

trc_serialization::Coordinates Serializer::Convert(geo::Coordinates c) {
    trc_serialization::Coordinates ser_c;

    ser_c.set_lat(c.lat);
    ser_c.set_lng(c.lng);

    return ser_c;
}

geo::Coordinates Serializer::Convert(
    const trc_serialization::Coordinates& ser_c) {
    geo::Coordinates c;

    c.lat = ser_c.lat();
    c.lng = ser_c.lng();

    return c;
}

trc_serialization::StopDistances Serializer::Convert(
    const std::pair<const Stop*, std::unordered_map<const Stop*, double>>& sd) {
    trc_serialization::StopDistances ser_sd;

    const auto& [from_stop, to_stop_to_distance] = sd;

    ser_sd.set_from_stop_id(from_stop->id);

    std::for_each(to_stop_to_distance.begin(), to_stop_to_distance.end(),
                  [&ser_sd](std::pair<const Stop*, double> stop_to_distance) {
                      *ser_sd.add_distance_info() = Convert(stop_to_distance);
                  });

    return ser_sd;
}

std::pair<const Stop*, std::unordered_map<const Stop*, double>>
Serializer::Convert(
    const trc_serialization::StopDistances& ser_sd,
    const std::unordered_map<uint32_t, const Stop*>& stop_id_to_stop) {
    std::pair<const Stop*, std::unordered_map<const Stop*, double>> sd;
    auto& [from_stop, to_stop_to_distance] = sd;

    from_stop = stop_id_to_stop.at(ser_sd.from_stop_id());

    for (size_t i = 0; i < ser_sd.distance_info_size(); ++i) {
        to_stop_to_distance.emplace(
            Convert(ser_sd.distance_info(i), stop_id_to_stop));
    }

    return sd;
}

trc_serialization::DistanceInfo Serializer::Convert(
    std::pair<const Stop*, double> di) {
    trc_serialization::DistanceInfo ser_di;

    ser_di.set_to_stop_id(di.first->id);
    ser_di.set_distance(di.second);

    return ser_di;
}

std::pair<const Stop*, double> Serializer::Convert(
    const trc_serialization::DistanceInfo& ser_di,
    const std::unordered_map<uint32_t, const Stop*>& stop_id_to_stop) {
    std::pair<const Stop*, double> di;

    di.first = stop_id_to_stop.at(ser_di.to_stop_id());
    di.second = ser_di.distance();

    return di;
}

trc_serialization::Bus Serializer::Convert(const Bus& b) {
    trc_serialization::Bus ser_b;

    ser_b.set_name(b.name);

    std::for_each(b.route.begin(), b.route.end(),
                  [&ser_b](const Stop* stop) { ser_b.add_stop(stop->id); });

    ser_b.set_route_length(b.route_length);
    ser_b.set_curvature(b.curvature);
    ser_b.set_is_roundtrip(b.is_roundtrip);

    return ser_b;
}

Bus Serializer::Convert(
    const trc_serialization::Bus& ser_b,
    const std::unordered_map<uint32_t, const Stop*>& stop_id_to_stop) {
    Bus b;

    b.name = ser_b.name();

    b.route.reserve(ser_b.stop_size());

    for (size_t i = 0; i < ser_b.stop_size(); ++i) {
        b.route.push_back(stop_id_to_stop.at(ser_b.stop(i)));
    }

    b.route_length = ser_b.route_length();
    b.curvature = ser_b.curvature();
    b.is_roundtrip = ser_b.is_roundtrip();

    return b;
}

trc_serialization::Point Serializer::Convert(svg::Point p) {
    trc_serialization::Point ser_p;

    ser_p.set_x(p.x);
    ser_p.set_y(p.y);

    return ser_p;
}

svg::Point Serializer::Convert(const trc_serialization::Point& ser_p) {
    svg::Point p;

    p.x = ser_p.x();
    p.y = ser_p.y();

    return p;
}

trc_serialization::Color Serializer::Convert(const svg::Color& c) {
    trc_serialization::Color ser_c;

    struct ColorSerializer {
        ColorSerializer(trc_serialization::Color& ser_c) : ser_c_(ser_c) {}

        void operator()(const std::string& str) {
            *ser_c_.mutable_str_color() = str;
        }
        void operator()(std::monostate) {
            *ser_c_.mutable_str_color() = "none";
        }
        void operator()(svg::Rgb rgb) {
            *ser_c_.mutable_rgb_color() = Convert(rgb);
        }
        void operator()(svg::Rgba rgba) {
            *ser_c_.mutable_rgba_color() = Convert(rgba);
        }

       private:
        trc_serialization::Color& ser_c_;
    };

    ColorSerializer color_serializer(ser_c);

    std::visit(color_serializer, c);

    return ser_c;
}

svg::Color Serializer::Convert(const trc_serialization::Color& ser_c) {
    svg::Color c;

    switch (ser_c.color_case()) {
        case trc_serialization::Color::kRgbColor: {
            c = Convert(ser_c.rgb_color());
            break;
        }
        case trc_serialization::Color::kRgbaColor: {
            c = Convert(ser_c.rgba_color());
            break;
        }
        case trc_serialization::Color::kStrColor: {
            c = ser_c.str_color();
            break;
        }
        default: {
            c = "none";
            break;
        }
    }

    return c;
}

trc_serialization::Rgb Serializer::Convert(svg::Rgb rgb) {
    trc_serialization::Rgb ser_rgb;

    ser_rgb.set_r(rgb.red);
    ser_rgb.set_g(rgb.green);
    ser_rgb.set_b(rgb.blue);

    return ser_rgb;
}

svg::Rgb Serializer::Convert(const trc_serialization::Rgb& ser_rgb) {
    svg::Rgb rgb;

    rgb.red = ser_rgb.r();
    rgb.green = ser_rgb.g();
    rgb.blue = ser_rgb.b();

    return rgb;
}

trc_serialization::Rgba Serializer::Convert(svg::Rgba rgba) {
    trc_serialization::Rgba ser_rgba;

    ser_rgba.set_r(rgba.red);
    ser_rgba.set_g(rgba.green);
    ser_rgba.set_b(rgba.blue);
    ser_rgba.set_a(rgba.opacity);

    return ser_rgba;
}

svg::Rgba Serializer::Convert(const trc_serialization::Rgba& ser_rgba) {
    svg::Rgba rgba;

    rgba.red = ser_rgba.r();
    rgba.green = ser_rgba.g();
    rgba.blue = ser_rgba.b();
    rgba.opacity = ser_rgba.a();

    return rgba;
}

}  // namespace trc
