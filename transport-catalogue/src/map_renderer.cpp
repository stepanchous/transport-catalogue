#include "map_renderer.h"

#include <ostream>

#include "svg.h"

namespace trc::render {

bool IsZero(double value) { return std::abs(value) < EPSILON; }

SphereProjector::SphereProjector(double min_lat, double max_lat, double min_lng,
                                 double max_lng, double max_width,
                                 double max_height, double padding)
    : padding_(padding), min_lng_(min_lng), max_lat_(max_lat) {
    std::optional<double> width_zoom;
    if (!IsZero(max_lng - min_lng_)) {
        width_zoom = (max_width - 2 * padding) / (max_lng - min_lng_);
    }

    std::optional<double> height_zoom;

    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
        zoom_coeff_ = *height_zoom;
    }
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {(coords.lng - min_lng_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

MapRenderer::MapRenderer(RenderSettings&& settings)
    : settings_(std::move(settings)), current_color_(0) {}

void MapRenderer::Render(const std::deque<Bus>& buses, std::ostream& out) {
    const std::set<Bus> sorted_buses(buses.begin(), buses.end());

    auto [min_lat, max_lat, min_lng, max_lng] = MinMaxLatLng(sorted_buses);

    SphereProjector projector_(min_lat, max_lat, min_lng, max_lng,
                               settings_.width, settings_.height,
                               settings_.padding);

    for (const auto& bus : sorted_buses) {
        if (bus.route.empty()) {
            continue;
        }

        RouteLine(bus.route, projector_,
                  settings_.color_palette[current_color_], settings_.line_width,
                  svg::StrokeLineCap::ROUND, svg::StrokeLineJoin::ROUND,
                  std::monostate{})
            .Draw(document_);

        current_color_ = (current_color_ == settings_.color_palette.size() - 1)
                             ? 0
                             : current_color_ + 1;
    }

    current_color_ = 0;

    for (const auto& bus : sorted_buses) {
        if (bus.route.empty()) {
            continue;
        }

        RouteName(bus.route, projector_, bus.name, bus.is_roundtrip,
                  settings_.bus_label_offset, settings_.bus_label_font_size,
                  DEFAULT_FONT, DEFAULT_FONT_WEIGHT,
                  settings_.color_palette[current_color_],
                  settings_.underlayer_color, settings_.underlayer_width,
                  svg::StrokeLineCap::ROUND, svg::StrokeLineJoin::ROUND)
            .Draw(document_);
        current_color_ = (current_color_ == settings_.color_palette.size() - 1)
                             ? 0
                             : current_color_ + 1;
    }

    current_color_ = 0;

    std::set<const Stop*, StopPtrCompare> stops;

    for (const auto& bus : sorted_buses) {
        for (const Stop* stop : bus.route) {
            stops.insert(stop);
        }
    }

    RouteStops(stops, projector_, settings_.stop_radius,
               DEFAULT_FILL_COLOR_STOP)
        .Draw(document_);

    RouteStopNames(stops, projector_, settings_.stop_label_offset,
                   settings_.stop_label_font_size, DEFAULT_FONT,
                   DEFAULT_FILL_COLOR_STOP_NAME, settings_.underlayer_color,
                   settings_.underlayer_width, svg::StrokeLineCap::ROUND,
                   svg::StrokeLineJoin::ROUND)
        .Draw(document_);

    document_.Render(out);
}

const RenderSettings& MapRenderer::GetRenderSettings() const {
    return settings_;
}

std::tuple<double, double, double, double> MapRenderer::MinMaxLatLng(
    const std::set<Bus>& buses) const {
    double min_lat{}, max_lat{}, min_lng{}, max_lng{};

    bool point_exist = false;
    for (const auto& bus : buses) {
        if (!bus.route.empty()) {
            min_lat = bus.route[0]->coordinates.lat;
            max_lat = bus.route[0]->coordinates.lat;
            min_lng = bus.route[0]->coordinates.lng;
            max_lng = bus.route[0]->coordinates.lng;
            point_exist = true;
            break;
        }
    }

    if (!point_exist) {
        return {min_lat, max_lat, min_lng, max_lng};
    }

    for (const auto& bus : buses) {
        for (const auto& stop : bus.route) {
            if (stop->coordinates.lat < min_lat) {
                min_lat = stop->coordinates.lat;
            }

            if (stop->coordinates.lat > max_lat) {
                max_lat = stop->coordinates.lat;
            }

            if (stop->coordinates.lng < min_lng) {
                min_lng = stop->coordinates.lng;
            }

            if (stop->coordinates.lng > max_lng) {
                max_lng = stop->coordinates.lng;
            }
        }
    }

    return {min_lat, max_lat, min_lng, max_lng};
}

RouteCore::RouteCore(const std::vector<const Stop*>& route,
                     const SphereProjector& projector)
    : route(route), projector(projector) {}

RouteLine::RouteLine(const std::vector<const Stop*>& route,
                     const SphereProjector& projector, svg::Color stroke_color,
                     double stroke_width, svg::StrokeLineCap stroke_line_cap,
                     svg::StrokeLineJoin stroke_line_join,
                     svg::Color fill_color)
    : core_(route, projector),
      stroke_color_(stroke_color),
      stroke_width_(stroke_width),
      stroke_line_cap_(stroke_line_cap),
      stroke_line_join_(stroke_line_join),
      fill_color_(fill_color) {}

void RouteLine::Draw(svg::ObjectContainer& container) const {
    svg::Polyline route_representation;
    route_representation.SetStrokeWidth(stroke_width_)
        .SetStrokeColor(stroke_color_)
        .SetFillColor(fill_color_)
        .SetStrokeLineCap(stroke_line_cap_)
        .SetStrokeLineJoin(stroke_line_join_);

    for (const Stop* stop : core_.route) {
        route_representation.AddPoint(core_.projector(stop->coordinates));
    }

    container.Add(std::move(route_representation));
}

RouteTextObjectProperties::RouteTextObjectProperties(
    svg::Point offset, int font_size, std::string font_family,
    svg::Color font_color, svg::Color underlayer_color, double underlayer_width,
    svg::StrokeLineCap stroke_line_cap, svg::StrokeLineJoin stroke_line_join)
    : offset(offset),
      font_size(font_size),
      font_family(std::move(font_family)),
      font_color(font_color),
      underlayer_color(underlayer_color),
      underlayer_width(underlayer_width),
      stroke_line_cap(stroke_line_cap),
      stroke_line_join(stroke_line_join) {}

std::tuple<svg::Text, svg::Text> PreBuildUnderLayerAndName(
    const std::string& name, const RouteTextObjectProperties& text_properties) {
    return {svg::Text()
                .SetData(name)
                .SetOffset(text_properties.offset)
                .SetFontSize(text_properties.font_size)
                .SetFontFamily(text_properties.font_family)
                .SetFillColor(text_properties.underlayer_color)
                .SetStrokeColor(text_properties.underlayer_color)
                .SetStrokeWidth(text_properties.underlayer_width)
                .SetStrokeLineCap(text_properties.stroke_line_cap)
                .SetStrokeLineJoin(text_properties.stroke_line_join),
            svg::Text()
                .SetData(name)
                .SetOffset(text_properties.offset)
                .SetFontSize(text_properties.font_size)
                .SetFontFamily(text_properties.font_family)
                .SetFillColor(text_properties.font_color)};
}

RouteName::RouteName(const std::vector<const Stop*>& route,
                     const SphereProjector& projector, const std::string& name,
                     bool is_roundtrip, svg::Point bus_label_offset,
                     int bus_label_font_size, std::string font_family,
                     std::string font_weight, svg::Color font_color,
                     svg::Color underlayer_color, double underlayer_width,
                     svg::StrokeLineCap stroke_line_cap,
                     svg::StrokeLineJoin stroke_line_join)
    : core_(route, projector),
      name_(name),
      is_roundtrip_(is_roundtrip),
      text_properties_(bus_label_offset, bus_label_font_size,
                       std::move(font_family), font_color, underlayer_color,
                       underlayer_width, stroke_line_cap, stroke_line_join),
      font_weight_(std::move(font_weight)) {}

void RouteName::Draw(svg::ObjectContainer& container) const {
    auto [underlayer, route_name] =
        PreBuildUnderLayerAndName(name_, text_properties_);
    underlayer.SetFontWeight(font_weight_);
    route_name.SetFontWeight(font_weight_);

    if (is_roundtrip_ ||
        IsZero((core_.route[0]->coordinates.lat -
                core_.route[core_.route.size() / 2]->coordinates.lat) +
               (core_.route[0]->coordinates.lng -
                core_.route[core_.route.size() / 2]->coordinates.lng))) {
        underlayer.SetPosition(core_.projector(core_.route[0]->coordinates));
        route_name.SetPosition(core_.projector(core_.route[0]->coordinates));
        container.Add(underlayer);
        container.Add(route_name);
    } else {
        svg::Text underlayer_end = underlayer;
        underlayer.SetPosition(core_.projector(core_.route[0]->coordinates));
        underlayer_end.SetPosition(
            core_.projector(core_.route[core_.route.size() / 2]->coordinates));

        svg::Text route_name_end = route_name;
        route_name.SetPosition(core_.projector(core_.route[0]->coordinates));
        route_name_end.SetPosition(
            core_.projector(core_.route[core_.route.size() / 2]->coordinates));

        container.Add(underlayer);
        container.Add(route_name);
        container.Add(underlayer_end);
        container.Add(route_name_end);
    }
}

RouteStops::RouteStops(const std::set<const Stop*, StopPtrCompare>& stops,
                       const SphereProjector& projector, double radius,
                       svg::Color fill_color)
    : stops_(stops),
      projector_(projector),
      radius_(radius),
      fill_color_(fill_color) {}

void RouteStops::Draw(svg::ObjectContainer& container) const {
    for (const Stop* stop : stops_) {
        container.Add(svg::Circle()
                          .SetCenter(projector_(stop->coordinates))
                          .SetRadius(radius_)
                          .SetFillColor(fill_color_));
    }
}

RouteStopNames::RouteStopNames(
    const std::set<const Stop*, StopPtrCompare>& stops,
    const SphereProjector& projector, svg::Point stop_label_offset,
    int stop_label_font_size, std::string font_family, svg::Color font_color,
    svg::Color underlayer_color, double underlayer_width,
    svg::StrokeLineCap stroke_line_cap, svg::StrokeLineJoin stroke_line_join)
    : stops_(stops),
      projector_(projector),
      text_properties_(stop_label_offset, stop_label_font_size,
                       std::move(font_family), font_color, underlayer_color,
                       underlayer_width, stroke_line_cap, stroke_line_join) {}

void RouteStopNames::Draw(svg::ObjectContainer& container) const {
    for (const Stop* stop : stops_) {
        auto [underlayer, stop_name] =
            PreBuildUnderLayerAndName(stop->name, text_properties_);
        underlayer.SetPosition(projector_(stop->coordinates));
        stop_name.SetPosition(projector_(stop->coordinates));
        container.Add(underlayer);
        container.Add(stop_name);
    }
}

}  // namespace trc::render
