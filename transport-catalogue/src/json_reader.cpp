#include "json_reader.h"

#include <algorithm>

#include "json.h"

namespace trc::io {

using namespace std;

JsonReader::JsonReader(istream& input) : document_(json::Load(input)) {}

vector<AddStopRequest> JsonReader::GetStops() const {
    vector<AddStopRequest> parsed_stops;

    for (const auto& base_request :
         document_.GetRoot().AsDict().at(BASE_REQUESTS_FIELD).AsArray()) {
        if (base_request.AsDict().at(TYPE_FIELD).AsString() ==
            STOP_TYPE_FIELD) {
            parsed_stops.push_back(ParseStop(base_request.AsDict()));
        }
    }

    return parsed_stops;
}

vector<AddBusRequest> JsonReader::GetBuses() const {
    vector<AddBusRequest> parsed_buses;

    for (const auto& base_request :
         document_.GetRoot().AsDict().at(BASE_REQUESTS_FIELD).AsArray()) {
        if (base_request.AsDict().at(TYPE_FIELD).AsString() == BUS_TYPE_FIELD) {
            parsed_buses.push_back(ParseBus(base_request.AsDict()));
        }
    }

    return parsed_buses;
}

vector<StatRequest> JsonReader::GetStatRequests() const {
    vector<StatRequest> stat_requests;

    for (const auto& stat_request :
         document_.GetRoot().AsDict().at(STAT_REQUESTS_FIELD).AsArray()) {
        stat_requests.push_back(ParseStatRequest(stat_request.AsDict()));
    }

    return stat_requests;
}

render::RenderSettings JsonReader::GetRenderSettings() const {
    json::Dict settings_json =
        document_.GetRoot().AsDict().at(RENDER_SETTINGS_FIELD).AsDict();

    return {settings_json.at(WIDTH_FIELD).AsDouble(),
            settings_json.at(HEIGHT_FIELD).AsDouble(),
            settings_json.at(PADDING_FIELD).AsDouble(),
            settings_json.at(LINE_WIDTH_FIELD).AsDouble(),
            settings_json.at(STOP_RADIUS_FIELD).AsDouble(),
            settings_json.at(BUS_LABEL_FONT_SIZE_FIELD).AsInt(),
            ParsePoint(settings_json.at(BUS_LABEL_OFFSET_FIELD).AsArray()),
            settings_json.at(STOP_LABEL_FONT_SIZE_FIELD).AsInt(),
            ParsePoint(settings_json.at(STOP_LABEL_OFFSET_FIELD).AsArray()),
            ParseColor(settings_json.at(UNDERLAYER_COLOR_FIELD)),
            settings_json.at(UNDERLAYER_WIDTH_FIELD).AsDouble(),
            ParseColorPalette(settings_json.at(COLOR_PALETTE_FIELD).AsArray())};
}

TransportRouter::Settings JsonReader::GetRoutingSettings() const {
    json::Dict settings_json =
        document_.GetRoot().AsDict().at(ROUTING_SETTINGS_FIELD).AsDict();

    return {settings_json.at(BUS_WAIT_TIME_FIELD).AsDouble(),
            settings_json.at(BUS_VELOCITY_FIELD).AsDouble()};
}

SerializationSettings JsonReader::GetSerializationSettings() const {
    json::Dict settings_json =
        document_.GetRoot().AsDict().at(SERIALIZATION_SETTINGS_FIELD).AsDict();

    return {settings_json.at(FILE_FIELD).AsString()};
}

AddStopRequest JsonReader::ParseStop(const json::Dict& stop_properties) const {
    return {
        stop_properties.at(NAME_FIELD).AsString(),
        stop_properties.at(LATITUDE_FIELD).AsDouble(),
        stop_properties.at(LONGITUDE_FIELD).AsDouble(),
        ParseRoadDistances(stop_properties.at(ROAD_DISTANCES_FIELD).AsDict())};
}

map<string, double> JsonReader::ParseRoadDistances(
    const json::Dict& distances) const {
    map<string, double> stop_to_distance;

    for (const auto& [stop_name, distance] : distances) {
        stop_to_distance[stop_name] = distance.AsDouble();
    }

    return stop_to_distance;
}

AddBusRequest JsonReader::ParseBus(const json::Dict& bus_properties) const {
    return {bus_properties.at(NAME_FIELD).AsString(),
            ParseRouteStops(bus_properties.at(STOPS_FIELD).AsArray()),
            bus_properties.at(IS_ROUNDTRIP__FIELD).AsBool()};
}

vector<string> JsonReader::ParseRouteStops(
    const json::Array& route_stops) const {
    vector<string> result;

    for (const auto& stop : route_stops) {
        result.push_back(stop.AsString());
    }

    return result;
}

StatRequest JsonReader::ParseStatRequest(const json::Dict& stat_request) const {
    const std::string& request_type = stat_request.at(TYPE_FIELD).AsString();

    if (request_type == "Stop") {
        return GetStopRequest{stat_request.at(ID_FIELD).AsInt(),
                              stat_request.at(NAME_FIELD).AsString()};
    } else if (request_type == "Bus") {
        return GetBusRequest{stat_request.at(ID_FIELD).AsInt(),
                             stat_request.at(NAME_FIELD).AsString()};
    } else if (request_type == "Map") {
        return GetMapRequest{stat_request.at(ID_FIELD).AsInt()};
    } else if (request_type == "Route") {
        return GetRouteRequest{stat_request.at(ID_FIELD).AsInt(),
                               stat_request.at(FROM_FIELD).AsString(),
                               stat_request.at(TO_FIELD).AsString()};
    } else {
        return UnknownRequest{};
    }
}

svg::Color JsonReader::ParseColor(const json::Node& color) const {
    if (color.IsString()) {
        return color.AsString();
    }

    if (color.IsArray()) {
        if (color.AsArray().size() == 3) {
            return svg::Rgb(static_cast<uint8_t>(color.AsArray()[0].AsInt()),
                            static_cast<uint8_t>(color.AsArray()[1].AsInt()),
                            static_cast<uint8_t>(color.AsArray()[2].AsInt()));
        }
        if (color.AsArray().size() == 4) {
            return svg::Rgba(static_cast<uint8_t>(color.AsArray()[0].AsInt()),
                             static_cast<uint8_t>(color.AsArray()[1].AsInt()),
                             static_cast<uint8_t>(color.AsArray()[2].AsInt()),
                             color.AsArray()[3].AsDouble());
        }
    }

    return {};
}

std::vector<svg::Color> JsonReader::ParseColorPalette(
    const json::Array& colors) const {
    std::vector<svg::Color> color_palette(colors.size());
    std::transform(
        colors.begin(), colors.end(), color_palette.begin(),
        [this](const json::Node& color) { return ParseColor(color); });

    return color_palette;
}

svg::Point JsonReader::ParsePoint(const json::Array& point) const {
    return {point[0].AsDouble(), point[1].AsDouble()};
}

}  // namespace trc::io
