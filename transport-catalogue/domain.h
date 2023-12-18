#pragma once
 
#include "geo.h"
#include "graph.h"

#include <algorithm>
#include <vector>
#include <string>
#include <variant>
 
namespace domain {

struct Bus;
    
struct Stop {    
    std::string name;
    std::pair<double, double> coordinate; // latitude longitude
};
 
struct Bus { 
    std::string name;
    std::vector<Stop*> stops;
    bool is_roundtrip;
};
    
struct BusResult{
    std::string_view name;
    bool not_found;
    int stops_on_route;
    int unique_stops;
    int route_length;
    double curvature;
};
    
struct StopResult{
    std::string_view name;
    bool not_found;
    std::vector <std::string> buses_name;
};

struct StatRequest { 
    int id;
    std::string type;
    std::string name;    
    std::string from;
    std::string to;
};
    
struct StopEdge {
    std::string_view name;
    double time = 0;
};

struct BusEdge {
    std::string_view bus_name;
    size_t span_count = 0;
    double time = 0;
};

struct RoutingSettings {
    double bus_wait_time = 0;
    double bus_velocity = 0;
};

struct RouterByStop {
    graph::VertexId bus_wait_start;
    graph::VertexId bus_wait_end;
};

struct RouteInfo {
    double total_time = 0.;
    std::vector<std::variant<StopEdge, BusEdge>> edges;
};

}//end namespace domian