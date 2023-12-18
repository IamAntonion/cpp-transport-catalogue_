#pragma once

#include <deque>
#include <string>
#include <vector>
#include <iomanip>
#include <deque>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
//#include <execution>
#include <numeric>
 
#include "domain.h"

using namespace domain;

namespace transport_catalogue {   
 
struct DistanceHasher {
    std::hash<const void*> hasher;
    
    std::size_t operator()(const std::pair<const Stop*, const Stop*> pair_stops) const noexcept {
        auto hash_1 = static_cast<const void*>(pair_stops.first);
        auto hash_2 = static_cast<const void*>(pair_stops.second);
        return hasher(hash_1) * 17 + hasher(hash_2);
    }  
};
    
using StopMap = std::unordered_map<std::string_view, Stop*>;
using BusMap = std::unordered_map<std::string_view, Bus*>;
using DistanceMap = std::unordered_map<std::pair<const Stop*, const  Stop*>, int, DistanceHasher>;
 
class TransportCatalogue {
public:
    void AddBus(const Bus&& bus);
    void AddStop(const Stop&& stop);
    void AddDistance(const Stop* coord1, 
                     const Stop* coord2, 
                     const int distance);
    
    Bus* GetBus(std::string_view bus_name);
    Stop* GetStop(std::string_view stop_name);
    std::deque<Stop> GetStops() const;
    std::deque<Bus> GetBuses() const;
    BusMap GetBusNameToBus() const;
    StopMap GetStopNameToStop() const;
    std::unordered_map<Stop*, std::unordered_set<Bus*>> GetStopStatistics() const;
    
    BusResult GetBusInfo(std::string_view bus_name);
    StopResult GetStopInfo(std::string_view stop_name);
    
    size_t GetDistanceStop(const Stop* start, 
                           const Stop* finish) const;
    size_t GetDistanceToBus(Bus* bus);
    DistanceMap GetDistance() const;
    
private:
    
    std::deque<Stop> stops;
    StopMap stopname_to_stop;
    
    std::deque<Bus> buses;
    BusMap busname_to_bus;
    
    DistanceMap distance_to_stop;

    std::unordered_set<const Bus*> StopGetUniqBuses(Stop* stop);    
    std::unordered_set<const Stop*> GetUniqStops(Bus* bus);
    double GetLength(Bus* bus);
    
    std::unordered_map<Stop*, std::unordered_set<Bus*>> buses_;
};
    
} // end namespace transport_catalogue