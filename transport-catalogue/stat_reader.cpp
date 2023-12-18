#include "stat_reader.h"

namespace transport_catalogue { 
namespace bus {   
    
void ShowTerminalBus(std::ostream& output, const transport_catalogue::BusResult& bus_info) {
    if(bus_info.not_found) {
        output << "Bus " << bus_info.name << ": not found" << std::endl;
    }
    else{
        output << "Bus " << bus_info.name << ": "
               << bus_info.stops_on_route << " stops on route, "
               << bus_info.unique_stops << " unique stops, "
               << bus_info.route_length << " route length, " 
               << std::setprecision(6) << bus_info.curvature
               << " curvature" << std::endl;
    }
}
    
transport_catalogue::BusResult ExecuteQueryInfoBus(TransportCatalogue& catalogue, std::string_view bus_name) {
    BusResult bus_info;
    auto entry = 4;
    bus_name = bus_name.substr(entry);
    bus_info = catalogue.GetBusInfo(bus_name);
    return bus_info;
}
    
}//end namespace bus
    
namespace stop {   
    
void ShowTerminalStop(std::ostream& output, const transport_catalogue::StopResult& stop_info) {
    if(stop_info.not_found) {
        output << "Stop " << stop_info.name << ": not found" << std::endl;
    } else {
        if(stop_info.buses_name.size() == 0){
            output << "Stop " << stop_info.name << ": no buses" << std::endl;
        } else {
            output << "Stop " << stop_info.name << ": buses ";
 
            for (std::string_view bus_name : stop_info.buses_name) {
                 output << bus_name;
                 output << " ";
            }
            output << std::endl;
        }
    }
}
    
transport_catalogue::StopResult ExecuteQueryInfoStop(TransportCatalogue& catalogue, std::string_view stop_name) {
    StopResult stop_info;
    auto entry = 5;
    stop_name = stop_name.substr(entry);
 
    stop_info = catalogue.GetStopInfo(stop_name);
    return stop_info;
}
    
}//end namespace stop
    
void ExecuteQuery(std::ostream& output, TransportCatalogue& catalogue, std::string_view str) {
    if (str.substr(0, 3) == "Bus") {
        bus::ShowTerminalBus(output, bus::ExecuteQueryInfoBus(catalogue, str));
    } else if (str.substr(0, 4) == "Stop") {
        stop::ShowTerminalStop(output, stop::ExecuteQueryInfoStop(catalogue, str));
    } else{
        std::cout << "Error query" << std::endl;
    }
}
 
void ParseAndExecuteQueries(std::istream& input, std::ostream& output, TransportCatalogue& catalogue) {
    std::string count;
    std::getline(input, count);
    
    std::string str;
    std::vector<std::string> query;
    auto amount = stoi(count);
    
    for (int i = 0; i < amount; ++i) {
        std::getline(input, str);
        ExecuteQuery(output, catalogue, str);
    }
}
    
}//end namespace transport_catalogue