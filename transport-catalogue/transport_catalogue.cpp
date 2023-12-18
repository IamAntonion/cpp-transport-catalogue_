#include "transport_catalogue.h"

namespace transport_catalogue {
 
void TransportCatalogue::AddStop(const Stop&& stop) {
    stops.push_back(std::move(stop));
    Stop* stop_buf = &stops.back();
    stopname_to_stop.insert(transport_catalogue::StopMap::value_type(stop_buf->name, stop_buf));
}
 
void TransportCatalogue::AddBus(const Bus&& bus) {
    Bus* bus_buf;
    
    buses.push_back(std::move(bus)); 
    bus_buf = &buses.back();
    busname_to_bus.insert(BusMap::value_type(bus_buf->name, bus_buf));
 
    for (Stop* stop : bus_buf->stops) {
        buses_[stop].insert(bus_buf);
    }
}
    
void TransportCatalogue::AddDistance(const Stop* coord1, 
                                     const Stop* coord2, 
                                     const int distance) {
    auto d_pair = std::make_pair(coord1, coord2);
    distance_to_stop.insert(DistanceMap::value_type(d_pair, distance));
}
 
std::unordered_set<const Stop*> TransportCatalogue::GetUniqStops(Bus* bus) {
    std::unordered_set<const Stop*> unique_stops;
    
    unique_stops.insert(bus->stops.begin(), bus->stops.end());
    
    return unique_stops;
}
    
std::unordered_set<const Bus*> TransportCatalogue::StopGetUniqBuses(Stop* stop){    
    std::unordered_set<const Bus*> unique_stops;
    unique_stops.insert(buses_[stop].begin(), buses_[stop].end());
    
    return unique_stops;
}
 
double TransportCatalogue::GetLength(Bus* bus) {
    return transform_reduce(next(bus->stops.begin()), 
                            bus->stops.end(), 
                            bus->stops.begin(),
                            0.0,
                            std::plus<>{},
                            [](const Stop* lhs, const Stop* rhs) { 
                                return geo::ComputeDistance({(*lhs).coordinate.first,
                                                             (*lhs).coordinate.second}, 
                                                            {(*rhs).coordinate.first,
                                                             (*rhs).coordinate.second});
                            });
}
 
size_t TransportCatalogue::GetDistanceStop(const Stop* begin, 
                                           const Stop* finish) const {
    if (distance_to_stop.empty()) {
        return 0;
    } else {

        if (const auto& stop_ptr_pair = std::make_pair(begin, finish); distance_to_stop.count(stop_ptr_pair)) {
            return distance_to_stop.at(stop_ptr_pair);
        } else if (const auto& stop_ptr_pair = std::make_pair(finish, begin); distance_to_stop.count(stop_ptr_pair)) {
            return distance_to_stop.at(stop_ptr_pair);
        } else {
            return 0;
        } 
    }
}
 
size_t TransportCatalogue::GetDistanceToBus(Bus* bus) {
    size_t distance = 0;
    auto stops_size = bus->stops.size() - 1;
    for (int i = 0; i < static_cast<int>(stops_size); i++) {
        distance += GetDistanceStop(bus->stops[i], bus->stops[i+1]);
    }
    return distance;
}
    
Bus* TransportCatalogue::GetBus(std::string_view bus_name) {   
    if(busname_to_bus.empty()){
        return nullptr;
    }
    
    if (busname_to_bus.count(bus_name)) {
        return busname_to_bus.at(bus_name);
    } else {
        return nullptr;
    }
}
 
Stop* TransportCatalogue::GetStop(std::string_view stop_name) {
    if(stopname_to_stop.empty()){
        return nullptr;
    }
    
    if (stopname_to_stop.count(stop_name)) {
        return stopname_to_stop.at(stop_name);
    } else {
        return nullptr;
    }
}

BusMap TransportCatalogue::GetBusNameToBus() const {
    return busname_to_bus;
}

StopMap TransportCatalogue::GetStopNameToStop() const {
    return stopname_to_stop;
}

std::unordered_map<Stop*, std::unordered_set<Bus*>> TransportCatalogue::GetStopStatistics() const {
    return buses_;
}
    
StopResult TransportCatalogue::GetStopInfo(std::string_view stop_name) {
    StopResult result;
    std::unordered_set<const Bus*> unique_buses;
    Stop* stop = GetStop(stop_name);
 
    if (stop != NULL) {
        result.name = stop->name;
        result.not_found = false;
        unique_buses = StopGetUniqBuses(stop);
        
        if(unique_buses.size() > 0){
            for (const Bus* bus : unique_buses) {
                result.buses_name.push_back(bus->name);
            }
            std::sort(result.buses_name.begin(), result.buses_name.end());
        }        
    } 
    else {    
        result.name = stop_name;
        result.not_found = true;
    }
    
    return result;
}
    
BusResult TransportCatalogue::GetBusInfo(std::string_view bus_name) {
    BusResult result;
    Bus* bus = GetBus(bus_name);
    if (bus != nullptr) {                
        result.name = bus->name;
        result.not_found = false;
        result.stops_on_route = static_cast<int>(bus->stops.size());
        result.unique_stops = static_cast<int>(GetUniqStops(bus).size());
        result.route_length = static_cast<int>(GetDistanceToBus(bus));
        result.curvature = double(GetDistanceToBus(bus) / GetLength(bus));
    } else {  
        result.name = bus_name;
        result.not_found = true;
    }
    return result;
}


std::deque<Stop> TransportCatalogue::GetStops() const {
    return stops;
}

std::deque<Bus> TransportCatalogue::GetBuses() const {
    return buses;
}

DistanceMap TransportCatalogue::GetDistance() const {
    return distance_to_stop;
}
    
} // end transport_catalogue