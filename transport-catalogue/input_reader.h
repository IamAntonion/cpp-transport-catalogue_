#pragma once
#include "transport_catalogue.h"
#include <algorithm>
 
namespace transport_catalogue {
namespace stop {
    Stop ParseStop(const std::string& str);
    
} // end namespace stop
    
namespace distance {
    void ParseDistances(const std::string& str, TransportCatalogue& catalogue);
    
} // end namespace distance
    
namespace bus {
    Bus ParseBus(std::string_view str, TransportCatalogue& catalogue);
    
} // end namespace bus
 
void FillCatalogue(std::istream& input, TransportCatalogue& catalogue);
 
} // end namespace branch