#define _USE_MATH_DEFINES
#include "geo.h"
 
namespace geo {
    
double ComputeDistance(Coordinates start, 
                       Coordinates end) {
    using namespace std;
    
    if (!(start == end)) {
        const double dr = PI / 180.;
        return acos(sin(start.latitude * dr) * sin(end.latitude * dr)
                    + cos(start.latitude * dr) * cos(end.latitude * dr) 
                    * cos(abs(start.longitude - end.longitude) * dr)) * EARTH_RADIUS;
    } else {
        return 0.0;
    }
}
    
}//end namespace geo