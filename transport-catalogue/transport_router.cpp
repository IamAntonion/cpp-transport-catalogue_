#include "transport_router.h"

namespace transport_catalogue {
namespace detail {
namespace router {

void TransportRouter::BuildRouter(TransportCatalogue& transport_catalogue) {
    SetGraph(transport_catalogue);
    router_ = std::make_unique<Router<double>>(*graph_);
    router_->Build();
}

void TransportRouter::SetGraph(TransportCatalogue& transport_catalogue) {
    const auto stops_ptr_size = GetStopsPtr(transport_catalogue).size();  
    
    graph_ = std::make_unique<DirectedWeightedGraph<double>>(2 * stops_ptr_size);    
    
    SetStops(GetStopsPtr(transport_catalogue));    
    AddEdgeToStop();
    AddEdgeToBus(transport_catalogue);
}

void TransportRouter::SetRoutingSettings(RoutingSettings routing_settings) {
    routing_settings_ = std::move(routing_settings);
}

const std::variant<StopEdge, BusEdge>& TransportRouter::GetEdge(EdgeId id) const {
    return edge_id_to_edge_.at(id);
}

std::pair<graph::VertexId, graph::VertexId> TransportRouter::GetIndexByStop(Stop* const from, Stop* const to) const {
    graph::VertexId form_tmp, to_tmp;

    if (stop_to_router_.count(from)) {
        form_tmp = stop_to_router_.at(from).bus_wait_start;
    } else {
        form_tmp = {};
    }
    
    if (stop_to_router_.count(to)) {
        to_tmp = stop_to_router_.at(to).bus_wait_start;
    } else {
        to_tmp = {};
    }

    return {form_tmp, to_tmp};
}

std::optional<RouteInfo> TransportRouter::GetRouteInfo(Stop* const start, 
                                                       Stop* const end) const {
    const auto& route_info = router_->BuildRoute(GetIndexByStop(start, end));
    if (route_info) {
        RouteInfo result;
        result.total_time = route_info->weight;

        for (const auto edge : route_info->edges) {
            result.edges.emplace_back(GetEdge(edge));
        }

        return result;

    } else {
        return std::nullopt;
    }
}   

std::deque<Stop*> TransportRouter::GetStopsPtr(TransportCatalogue& transport_catalogue) {
    std::deque<Stop*> stops_ptr;  

    for (auto [_, stop_ptr] : transport_catalogue.GetStopNameToStop()) {
        stops_ptr.push_back(stop_ptr);
    }

    return stops_ptr;
}

std::deque<Bus*> TransportRouter::GetBusPtr(TransportCatalogue& transport_catalogue) {
    std::deque<Bus*> buses_ptr;  

    for (auto [_, bus_ptr] : transport_catalogue.GetBusNameToBus()) {
        buses_ptr.push_back(bus_ptr);
    }

    return buses_ptr;
}

void TransportRouter::SetStops(const std::deque<Stop*>& stops) {
    size_t i = 0;

    for (const auto stop : stops) {
        VertexId first = i++;
        VertexId second = i++;

        stop_to_router_[stop] = RouterByStop{first, second};
    }
}

void TransportRouter::AddEdgeToStop() {   

    for (const auto [stop, num] : stop_to_router_) {
        EdgeId id = graph_->AddEdge(Edge<double>{num.bus_wait_start, 
                                                 num.bus_wait_end, 
                                                 routing_settings_.bus_wait_time});

        edge_id_to_edge_[id] = StopEdge{stop->name, routing_settings_.bus_wait_time};
    }
}

void TransportRouter::AddEdgeToBus(TransportCatalogue& transport_catalogue) {

    for (auto bus : GetBusPtr(transport_catalogue)) {        
        ParseBusToEdges(bus->stops.begin(), 
                        bus->stops.end(), 
                        transport_catalogue,
                        bus);

        if (!bus->is_roundtrip) {
            ParseBusToEdges(bus->stops.rbegin(), 
                            bus->stops.rend(), 
                            transport_catalogue,
                            bus);
        }
    }
}

void TransportRouter::GraphConstruction(TransportCatalogue& transport_catalogue) {
    const auto stops_ptr_size = GetStopsPtr(transport_catalogue).size();  

    graph_ = std::make_unique<DirectedWeightedGraph<double>>(2 * stops_ptr_size);    

    SetStops(GetStopsPtr(transport_catalogue));    
    AddEdgeToStop();
    AddEdgeToBus(transport_catalogue);
}

Edge<double> TransportRouter::MakeEdgeToBus(Stop* start, 
                                            Stop* end, 
                                            const double distance) const {
    Edge<double> result;

    result.from = stop_to_router_.at(start).bus_wait_end;
    result.to = stop_to_router_.at(end).bus_wait_start;
    result.weight = distance * 1.0 / (routing_settings_.bus_velocity * KILOMETER / HOUR);

    return result;
}

}//end namespace router
}//end namespace detail
}//end namespace transport_catalogue