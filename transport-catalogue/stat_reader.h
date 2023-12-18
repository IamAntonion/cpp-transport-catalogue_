#pragma once
#include "transport_catalogue.h"
#include <algorithm>
#include <vector>

namespace transport_catalogue {
namespace bus {
    
void ShowTerminalBus(std::ostream& output, const transport_catalogue::BusResult& bus_info);
transport_catalogue::BusResult ExecuteQueryInfoBus (TransportCatalogue& catalogue, std::string_view str);
    
} // end namespace bus
    
namespace stop {
    
void ShowTerminalStop(std::ostream& output, const transport_catalogue::StopResult& stop_info);
transport_catalogue::StopResult ExecuteQueryInfoStop(TransportCatalogue& catalogue, std::string_view stop_name);
    
} // end namespace stop
    
void ExecuteQuery(TransportCatalogue& catalogue, std::string_view str);
void ParseAndExecuteQueries(std::istream& input, std::ostream& output, TransportCatalogue& catalogue);
    
} // end namespace branch