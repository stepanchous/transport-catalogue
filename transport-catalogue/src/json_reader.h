#pragma once

#include <istream>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "json.h"
#include "map_renderer.h"
#include "serialization.h"
#include "transport_router.h"

namespace trc::io {

inline const std::string BASE_REQUESTS_FIELD = "base_requests";
inline const std::string STAT_REQUESTS_FIELD = "stat_requests";
inline const std::string TYPE_FIELD = "type";
inline const std::string BUS_TYPE_FIELD = "Bus";
inline const std::string STOP_TYPE_FIELD = "Stop";
inline const std::string NAME_FIELD = "name";
inline const std::string LATITUDE_FIELD = "latitude";
inline const std::string LONGITUDE_FIELD = "longitude";
inline const std::string ROAD_DISTANCES_FIELD = "road_distances";
inline const std::string STOPS_FIELD = "stops";
inline const std::string IS_ROUNDTRIP__FIELD = "is_roundtrip";
inline const std::string ID_FIELD = "id";
inline const std::string REQUEST_ID_FIELD = "request_id";
inline const std::string BUSES_FIELD = "buses";
inline const std::string CURVATURE_FIELD = "curvature";
inline const std::string ROUTE_LENGTH_FIELD = "route_length";
inline const std::string STOP_COUNT_FIELD = "stop_count";
inline const std::string UNIQUE_STOP_COUNT_FIELD = "unique_stop_count";
inline const std::string ERROR_MESSAGE_FIELD = "error_message";
inline const std::string NOT_FOUND = "not found";
inline const std::string RENDER_SETTINGS_FIELD = "render_settings";
inline const std::string WIDTH_FIELD = "width";
inline const std::string HEIGHT_FIELD = "height";
inline const std::string PADDING_FIELD = "padding";
inline const std::string LINE_WIDTH_FIELD = "line_width";
inline const std::string STOP_RADIUS_FIELD = "stop_radius";
inline const std::string BUS_LABEL_FONT_SIZE_FIELD = "bus_label_font_size";
inline const std::string BUS_LABEL_OFFSET_FIELD = "bus_label_offset";
inline const std::string STOP_LABEL_FONT_SIZE_FIELD = "stop_label_font_size";
inline const std::string STOP_LABEL_OFFSET_FIELD = "stop_label_offset";
inline const std::string UNDERLAYER_COLOR_FIELD = "underlayer_color";
inline const std::string UNDERLAYER_WIDTH_FIELD = "underlayer_width";
inline const std::string COLOR_PALETTE_FIELD = "color_palette";
inline const std::string MAP_FIELD = "map";
inline const std::string ROUTING_SETTINGS_FIELD = "routing_settings";
inline const std::string BUS_VELOCITY_FIELD = "bus_velocity";
inline const std::string BUS_WAIT_TIME_FIELD = "bus_wait_time";
inline const std::string FROM_FIELD = "from";
inline const std::string TO_FIELD = "to";
inline const std::string STOP_NAME_FIELD = "stop_name";
inline const std::string BUS_FIELD = "bus";
inline const std::string TIME_FIELD = "time";
inline const std::string WAIT_FIELD = "Wait";
inline const std::string SPAN_COUNT_FIELD = "span_count";
inline const std::string ITEMS_FIELD = "items";
inline const std::string TOTAL_TIME_FIELD = "total_time";
inline const std::string SERIALIZATION_SETTINGS_FIELD =
    "serialization_settings";
inline const std::string FILE_FIELD = "file";

using StatRequstField = std::variant<std::string, int>;

using ColorField = std::variant<std::string, std::vector<double>>;

struct AddStopRequest {
    std::string name;
    double latitude;
    double longitude;
    std::map<std::string, double> road_distances;
};

struct AddBusRequest {
    std::string name;
    std::vector<std::string> stops;
    bool is_roundtrip;
};

struct GetStopRequest {
    int id;
    std::string name;
};

struct GetBusRequest {
    int id;
    std::string name;
};

struct GetMapRequest {
    int id;
};

struct GetRouteRequest {
    int id;
    std::string from_stop;
    std::string to_stop;
};

struct UnknownRequest {};

using StatRequest = std::variant<GetStopRequest, GetBusRequest, GetMapRequest,
                                 GetRouteRequest, UnknownRequest>;

class JsonReader {
   public:
    explicit JsonReader(std::istream& input);

    std::vector<AddStopRequest> GetStops() const;

    std::vector<AddBusRequest> GetBuses() const;

    std::vector<StatRequest> GetStatRequests() const;

    render::RenderSettings GetRenderSettings() const;

    TransportRouter::Settings GetRoutingSettings() const;

    SerializationSettings GetSerializationSettings() const;

   private:
    json::Document document_;

    AddStopRequest ParseStop(const json::Dict& stop_properties) const;

    AddBusRequest ParseBus(const json::Dict& bus_properties) const;

    std::map<std::string, double> ParseRoadDistances(
        const json::Dict& distances) const;

    std::vector<std::string> ParseRouteStops(
        const json::Array& route_stops) const;

    StatRequest ParseStatRequest(const json::Dict& stat_requests) const;

    svg::Color ParseColor(const json::Node& color) const;

    std::vector<svg::Color> ParseColorPalette(const json::Array& colors) const;

    svg::Point ParsePoint(const json::Array& point) const;
};

}  // namespace trc::io
