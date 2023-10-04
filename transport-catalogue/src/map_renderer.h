#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "svg.h"

namespace trc::render {

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
   public:
    SphereProjector(double min_lat, double max_lat, double min_lng,
                    double max_lng, double max_width, double max_height,
                    double padding);

    SphereProjector() = default;

    svg::Point operator()(geo::Coordinates coords) const;

   private:
    double padding_ = 0;
    double min_lng_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct RenderSettings {
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    int bus_label_font_size;
    svg::Point bus_label_offset;
    int stop_label_font_size;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};

class MapRenderer {
   public:
    explicit MapRenderer(RenderSettings&& settings);

    void Render(const std::deque<Bus>& buses, std::ostream& out);

    const RenderSettings& GetRenderSettings() const;

   private:
    RenderSettings settings_;
    size_t current_color_;
    svg::Document document_;

    void RenderRoute(const std::vector<const Stop*>& route) const;
    std::tuple<double, double, double, double> MinMaxLatLng(
        const std::set<Bus>& buses) const;
};

struct RouteCore {
    RouteCore(const std::vector<const Stop*>& route,
              const SphereProjector& projector);
    const std::vector<const Stop*>& route;
    const SphereProjector& projector;
};

class RouteLine : public svg::Drawable {
   public:
    RouteLine(const std::vector<const Stop*>& route,
              const SphereProjector& projector, svg::Color stroke_color,
              double stroke_width, svg::StrokeLineCap stroke_line_cap,
              svg::StrokeLineJoin stroke_line_join, svg::Color fill);
    void Draw(svg::ObjectContainer& container) const override;

   private:
    RouteCore core_;
    svg::Color stroke_color_;
    double stroke_width_;
    svg::StrokeLineCap stroke_line_cap_;
    svg::StrokeLineJoin stroke_line_join_;
    svg::Color fill_color_;
};

struct RouteTextObjectProperties {
    RouteTextObjectProperties(svg::Point offset, int font_size,
                              std::string font_family, svg::Color font_color,
                              svg::Color underlayer_color,
                              double underlayer_width,
                              svg::StrokeLineCap stroke_line_cap,
                              svg::StrokeLineJoin stroke_line_join);
    svg::Point offset;
    int font_size;
    std::string font_family;
    svg::Color font_color;
    svg::Color underlayer_color;
    double underlayer_width;
    svg::StrokeLineCap stroke_line_cap;
    svg::StrokeLineJoin stroke_line_join;
};

inline const std::string DEFAULT_FONT = "Verdana";
inline const std::string DEFAULT_FONT_WEIGHT = "bold";

class RouteName : public svg::Drawable {
   public:
    RouteName(const std::vector<const Stop*>& route,
              const SphereProjector& projector, const std::string& name,
              bool is_roundtrip, svg::Point bus_label_offset,
              int bus_label_font_size, std::string font_family,
              std::string font_weight, svg::Color font_color_,
              svg::Color underlayer_color, double underlayer_width,
              svg::StrokeLineCap stroke_line_cap,
              svg::StrokeLineJoin stroke_line_join);

    void Draw(svg::ObjectContainer& container) const override;

   private:
    RouteCore core_;
    const std::string& name_;
    bool is_roundtrip_;
    RouteTextObjectProperties text_properties_;
    std::string font_weight_;
};

inline const svg::Color DEFAULT_FILL_COLOR_STOP = "white";

class RouteStops : public svg::Drawable {
   public:
    RouteStops(const std::set<const Stop*, StopPtrCompare>& stops,
               const SphereProjector& projector, double radius,
               svg::Color fill_color);

    void Draw(svg::ObjectContainer& container) const override;

   private:
    const std::set<const Stop*, StopPtrCompare>& stops_;
    const SphereProjector& projector_;
    double radius_;
    svg::Color fill_color_;
};

inline const svg::Color DEFAULT_FILL_COLOR_STOP_NAME = "black";

class RouteStopNames : public svg::Drawable {
   public:
    RouteStopNames(const std::set<const Stop*, StopPtrCompare>& stops,
                   const SphereProjector& projector,
                   svg::Point stop_label_offset, int stop_label_font_size,
                   std::string font_family, svg::Color font_color,
                   svg::Color underlayer_color, double underlayer_width,
                   svg::StrokeLineCap stroke_line_cap,
                   svg::StrokeLineJoin stroke_line_join);

    void Draw(svg::ObjectContainer& container) const override;

   private:
    const std::set<const Stop*, StopPtrCompare>& stops_;
    const SphereProjector& projector_;
    RouteTextObjectProperties text_properties_;
};

}  // namespace trc::render
