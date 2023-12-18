#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "domain.h"

#include <deque>
#include <unordered_map>
#include <iostream>

namespace transport_catalogue {
namespace detail {
namespace router {

using namespace domain;
using namespace graph;

static const int KILOMETER = 1000;
static const int HOUR = 60;    

class TransportRouter {
public:
    TransportRouter(RoutingSettings routing_settings, TransportCatalogue& transport_catalogue): routing_settings_(std::move(routing_settings)) {
        GraphConstruction(transport_catalogue);
        router_ = std::make_unique<Router<double>>(*graph_);
        router_->Build();
    }
    void BuildRouter(TransportCatalogue& transport_catalogue);
    std::optional<RouteInfo> GetRouteInfo(Stop* const start, Stop* const end) const;
    void SetRoutingSettings(RoutingSettings routing_settings);

private:    
    std::unordered_map<Stop*, RouterByStop> stop_to_router_;
    std::unordered_map<EdgeId, std::variant<StopEdge, BusEdge>> edge_id_to_edge_;

    std::unique_ptr<DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<Router<double>> router_;

    RoutingSettings routing_settings_;

    const std::variant<StopEdge, BusEdge>& GetEdge(EdgeId id) const;

    std::deque<Stop*> GetStopsPtr(TransportCatalogue& transport_catalogue);
    std::deque<Bus*> GetBusPtr(TransportCatalogue& transport_catalogue);

    void AddEdgeToStop();
    void AddEdgeToBus(TransportCatalogue& transport_catalogue);

    void SetStops(const std::deque<Stop*>& stops);
    void SetGraph(TransportCatalogue& transport_catalogue);
    void GraphConstruction(TransportCatalogue& transport_catalogue);

    Edge<double> MakeEdgeToBus(Stop* start, Stop* end, const double distance) const;

    std::pair<graph::VertexId, graph::VertexId> GetIndexByStop(Stop* const from, Stop* const to) const;

    template <typename Iterator>
    void ParseBusToEdges(Iterator first, 
                         Iterator last,
                         const TransportCatalogue& transport_catalogue, 
                         const Bus* bus);
};

template <typename Iterator>
void TransportRouter::ParseBusToEdges(Iterator first, 
                                      Iterator last,
                                      const TransportCatalogue& transport_catalogue, 
                                      const Bus* bus) {

    for (auto it = first; it != last; ++it) {
        size_t distance = 0;
        size_t span = 0;

        for (auto it2 = std::next(it); it2 != last; ++it2) {
            distance += transport_catalogue.GetDistanceStop(*prev(it2), *it2);
            ++span;

            EdgeId id = graph_->AddEdge(MakeEdgeToBus(*it, *it2, distance));

            edge_id_to_edge_[id] = BusEdge{bus->name, span, graph_->GetEdge(id).weight};
        }
    }
}

} // end namespace router
} // end namespace detail
} // end namespace transport_catalogue