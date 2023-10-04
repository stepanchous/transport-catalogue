#pragma once

#include <sstream>
#include <string>

#include "json_reader.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace trc::rh {

class BaseRequestHandler {
   public:
    BaseRequestHandler(const io::JsonReader& json_reader);

    TransportCatalogue BuildTransportCatalogue();

   private:
    const io::JsonReader& json_reader_;

    void AddStops(TransportCatalogue& transport_catalogue);

    void AddBuses(TransportCatalogue& transport_catalogue);
};

class StatRequestHandler {
   public:
    StatRequestHandler(const TransportCatalogue& transport_catalogue,
                       render::MapRenderer& map_renderer_,
                       const TransportRouter& transport_router,
                       const io::JsonReader& json_reader, std::ostream& output);

    void HandleStatRequests();

   private:
    const TransportCatalogue& transport_catalogue_;
    const io::JsonReader& json_reader_;
    render::MapRenderer& map_renderer_;
    const TransportRouter& router_;
    std::ostream& output_;

    class StatHandler {
       public:
        StatHandler(const TransportCatalogue& transport_catalogue,
                    render::MapRenderer& map_renderer,
                    const TransportRouter& router_, std::ostream& output);

        void operator()(const io::GetStopRequest&);

        void operator()(const io::GetBusRequest&);

        void operator()(const io::GetMapRequest&);

        void operator()(const io::GetRouteRequest&);

        void operator()(const io::UnknownRequest&);

        void Print();

       private:
        json::Array responses_;
        const TransportCatalogue& transport_catalogue_;
        render::MapRenderer& map_renderer_;
        const TransportRouter& router_;
        std::ostream& output_;

        void HandleNotFound(int id);

        struct ItemVisitor {
            ItemVisitor(json::Array& items);

            void operator()(const TransportRouter::WaitItem&);

            void operator()(const TransportRouter::BusItem&);

           private:
            json::Array& items_;
        };
    };
};

}  // namespace trc::rh
