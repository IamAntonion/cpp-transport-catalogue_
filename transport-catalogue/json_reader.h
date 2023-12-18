#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"
#include "serialization.h"

using namespace transport_catalogue::detail::json::builder;
using namespace transport_catalogue::detail::router;
using namespace transport_catalogue::detail::json;
using namespace transport_catalogue;
using namespace map_renderer;
using namespace domain;
 
namespace transport_catalogue {
namespace detail {
namespace json {

class JSONReader{
public:
    JSONReader() = default;    
    JSONReader(Document doc);
    JSONReader(std::istream& input);
    
    void ParseNodeProcessRequests(std::vector<StatRequest>& stat_request,
                                  serialization::SerializationSettings& serialization_settings);
    void ParseNodeMakeBase(TransportCatalogue& catalogue, 
                           map_renderer::RenderSettings& render_settings, 
                           router::RoutingSettings& routing_settings,
                           serialization::SerializationSettings& serialization_settings);
    void ParseNodeSerialization(const Node& node,
                                serialization::SerializationSettings& serialization_set);
    void ParseNodeBase(const Node& root, 
                       TransportCatalogue& catalogue);
    void ParseNodeStat(const Node& root, 
                       std::vector<StatRequest>& stat_request);
    void ParseNodeRender(const Node& node, 
                         map_renderer::RenderSettings& render_settings);
    void ParseNodeRouting(const Node& node, 
                          router::RoutingSettings& route_set);
 
    Stop ParseNodeStop(Node& node);
    void ParseNodeDistances(Node& node, 
                            TransportCatalogue& catalogue);
    Bus ParseNodeBus(Node& node, 
                     TransportCatalogue& catalogue);

    const Document& GetDocument() const;
private:
    Document document_;
};
    
}//end namespace json

namespace request_handler {

struct EdgeInfoGetter {

    Node operator()(const StopEdge& edge_info) {
        using namespace std::literals;

        return Builder{}.StartDict()
                        .Key("type").Value("Wait")
                        .Key("stop_name").Value(std::string(edge_info.name))
                        .Key("time").Value(edge_info.time)
                        .EndDict()
                        .Build();
    }

    Node operator()(const BusEdge& edge_info) {
        using namespace std::literals;

        return Builder{}.StartDict()
                        .Key("type").Value("Bus")
                        .Key("bus").Value(std::string(edge_info.bus_name))
                        .Key("span_count").Value(static_cast<int>(edge_info.span_count))
                        .Key("time").Value(edge_info.time)
                        .EndDict()
                        .Build();
    }
};

class RequestHandler {
public:

    RequestHandler() = default;
    
    std::optional<RouteInfo> GetRouteInfo(std::string_view start, 
                                          std::string_view end, 
                                          TransportCatalogue& catalogue, 
                                          TransportRouter& routing) const;
    std::vector<geo::Coordinates> GetStopsCoordinates(TransportCatalogue& catalogue_) const;
    std::vector<std::string_view> GetSortBusesNames(TransportCatalogue& catalogue_) const;

    BusResult ExecuteQueryInfoBus(TransportCatalogue& catalogue, 
                                  std::string_view str);
    StopResult ExecuteQueryInfoStop(TransportCatalogue& catalogue, 
                                    std::string_view stop_name);

    Node ExecuteMakeNodeStop(int id_request, 
                             const StopResult& query_result);
    Node ExecuteMakeNodeBus(int id_request, 
                            const BusResult& query_result);
    Node ExecuteMakeNodeMap(int id_request, 
                            TransportCatalogue& catalogue, 
                            RenderSettings render_settings);
    Node ExecuteMakeNodeRoute(StatRequest& request, 
                              TransportCatalogue& catalogue, 
                              TransportRouter& routing);

    void ExecuteQueries(TransportCatalogue& catalogue, 
                        std::vector<StatRequest>& stat_requests, 
                        RenderSettings& render_settings, 
                        RoutingSettings& routing_settings);
    void ExecuteRenderMap(MapRenderer& map_catalogue, 
                          TransportCatalogue& catalogue_) const;
    

    const Document& GetDocument();
private:
    Document doc_out;
};
    
}//end namespace request_handler
}//end namespace detail
}//end namespace transport_catalogue