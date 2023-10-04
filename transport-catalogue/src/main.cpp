#include "map_renderer.h"
#include "request_handler.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

using namespace trc;

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        io::JsonReader json_reader(std::cin);

        render::MapRenderer map_renderer(json_reader.GetRenderSettings());

        Serializer serializer(json_reader.GetSerializationSettings());

        rh::BaseRequestHandler base_request_handler(json_reader);

        TransportCatalogue transport_catalogue =
            base_request_handler.BuildTransportCatalogue();

        TransportRouter::Settings router_settings =
            json_reader.GetRoutingSettings();

        serializer.Save(transport_catalogue, map_renderer.GetRenderSettings(),
                        router_settings);
    } else if (mode == "process_requests"sv) {
        io::JsonReader json_reader(std::cin);

        Serializer serializer(json_reader.GetSerializationSettings());

        auto [transport_catalogue, render_settings, router_settings] =
            serializer.Load();

        render::MapRenderer map_renderer(std::move(render_settings));

        TransportRouter transport_router(router_settings, transport_catalogue);

        rh::StatRequestHandler stat_request_handler(
            transport_catalogue, map_renderer, transport_router, json_reader,
            std::cout);

        stat_request_handler.HandleStatRequests();

    } else {
        PrintUsage();
        return 1;
    }
}
