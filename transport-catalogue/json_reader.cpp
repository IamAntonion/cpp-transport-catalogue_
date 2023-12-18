#include "json_reader.h"
 
namespace transport_catalogue {
namespace detail {
namespace json {

JSONReader::JSONReader(Document doc) : document_(std::move(doc)) {}
JSONReader::JSONReader(std::istream& input) : document_(json::Load(input)) {}  

void JSONReader::ParseNodeSerialization(const Node& node, serialization::SerializationSettings& serialization_set) {

    Dict serialization;

    if (node.IsDict()) {
        serialization = node.AsDict();

        try {
            serialization_set.file_name = serialization.at("file").AsString();

        } catch(...) {
            std::cout << "unable to parse serialization settings";
        }

    } else {
        std::cout << "serialization settings is not map";
    }
}

void JSONReader::ParseNodeProcessRequests(std::vector<StatRequest>& stat_request,
                                          serialization::SerializationSettings& serialization_settings) { 
    Dict root_dictionary;

    if (document_.GetRoot().IsDict()) {
        root_dictionary = document_.GetRoot().AsDict();

        try {
            ParseNodeStat(root_dictionary.at("stat_requests"), stat_request);

        } catch(...) {}

        try {
            ParseNodeSerialization(root_dictionary.at("serialization_settings"), serialization_settings);

        } catch(...) {}

    } else {
        std::cout << "root is not map";
    }
}
    
void JSONReader::ParseNodeMakeBase(TransportCatalogue& catalogue, 
                                   map_renderer::RenderSettings& render_settings, 
                                   router::RoutingSettings& routing_settings,
                                   serialization::SerializationSettings& serialization_settings) { 
    Dict root_dict;
    
    if (document_.GetRoot().IsDict()) {
        root_dict = document_.GetRoot().AsDict();
 
        try {          
            ParseNodeBase(root_dict.at("base_requests"), catalogue);
        } catch(...) {}
 
        // try {
        //     ParseNodeStat(root_dict.at("stat_requests"), stat_request);
        // } catch(...) {}

        try {
            ParseNodeRender(root_dict.at("render_settings"), render_settings);
        }
        catch(...){}
        
        try {
            ParseNodeRouting(root_dict.at("routing_settings"), routing_settings);
        } catch(...) {}

        try {
            ParseNodeSerialization(root_dict.at("serialization_settings"), serialization_settings);

        } catch(...) {}
        
    } else {
        std::cout << "root is not map";
    }
}

void JSONReader::ParseNodeRouting(const Node& node, 
                                  router::RoutingSettings& route_set) {
    Dict route;

    if (node.IsDict()) {
        route = node.AsDict();

        try {
            route_set.bus_velocity = route.at("bus_velocity").AsDouble();
            route_set.bus_wait_time = route.at("bus_wait_time").AsDouble();
        } catch(...) {
            std::cout << "unable to parse routing settings";
        }
    } else {
        std::cout << "routing settings is not map";
    }
}
 
void JSONReader::ParseNodeBase(const Node& root, 
                               TransportCatalogue& catalogue) {
    Array base_requests;
    Dict req_map;
    Node req_node;
    
    std::vector<Node> buses;
    std::vector<Node> stops;
    
    if (root.IsArray()) {
        base_requests = root.AsArray();
        
        for (Node& node : base_requests) {            
            if (node.IsDict()) {
                req_map = node.AsDict();
                
                try {
                    req_node = req_map.at("type");
                    if (req_node.IsString()) {
                        
                        if (req_node.AsString() == "Bus") {
                            buses.push_back(req_map);
                            
                        } else if (req_node.AsString() == "Stop") {
                            stops.push_back(req_map);
                            
                        } else {
                            std::cout << "Error: base_requests have bad type";
                        }
                    }
                    
                } catch(...) {
                    std::cout << "Error: base_requests not have type value";
                }                
            }
        }   
        
        for (auto stop : stops) {
            catalogue.AddStop(ParseNodeStop(stop));
        }
        
        for (auto stop : stops) {
            ParseNodeDistances(stop, catalogue);
        }
        
        for (auto bus : buses) {
            catalogue.AddBus(ParseNodeBus(bus, catalogue));
        }
        
    } else {
        std::cout << "Error: base_requests is not array";
    }
}
 
Stop JSONReader::ParseNodeStop(Node& node) {
    Stop stop;
    Dict stop_node;
    
    if (node.IsDict()) {
        
        stop_node = node.AsDict();
        stop.name = stop_node.at("name").AsString();
        stop.coordinate.first = stop_node.at("latitude").AsDouble();
        stop.coordinate.second = stop_node.at("longitude").AsDouble();
    }
    
    return stop;
}
    
void JSONReader::ParseNodeDistances(Node& node, 
                                    TransportCatalogue& catalogue) {
    Dict stop_node;
    Dict stop_road_map;
    std::string begin_name;
    std::string last_name;
    int distance;
    
    if (node.IsDict()) {
        stop_node = node.AsDict();
        begin_name = stop_node.at("name").AsString();
        
        try {
            stop_road_map = stop_node.at("road_distances").AsDict();
            
            for (auto [key, value] : stop_road_map) {
                last_name = key;
                distance = value.AsInt();
                catalogue.AddDistance(catalogue.GetStop(begin_name), 
                                      catalogue.GetStop(last_name), 
                                      distance);
            }            
        }
        
        catch(...) {
            std::cout << "Error: Road invalide" << std::endl;
        }        
    }
}
 
Bus JSONReader::ParseNodeBus(Node& node, 
                             TransportCatalogue& catalogue) {
    Bus bus;
    Dict bus_node;
    Array bus_stops;
    
    if (node.IsDict()) {
        bus_node = node.AsDict();
        bus.name = bus_node.at("name").AsString();
        bus.is_roundtrip = bus_node.at("is_roundtrip").AsBool();
        
        try {
            bus_stops = bus_node.at("stops").AsArray();
            
            for (Node stop : bus_stops) {
                bus.stops.push_back(catalogue.GetStop(stop.AsString()));
            }
 
            if (!bus.is_roundtrip) {
                size_t size = bus.stops.size() - 1;
                
                for (size_t i = size; i > 0; i--) {
                    bus.stops.push_back(bus.stops[i-1]);
                }
            }   
            
        } catch(...) {
            std::cout << "Error: base_requests: bus: stops is empty" << std::endl;
        }        
    }
    
    return bus;
}
 
void JSONReader::ParseNodeStat(const Node& node, 
                               std::vector<StatRequest>& stat_request) {
    Array stat_requests;
    Dict req_map;
    StatRequest req;
    
    if (node.IsArray()) {
        stat_requests = node.AsArray();
        
        for (Node req_node : stat_requests) {
            
            if (req_node.IsDict()) {
                
                req_map = req_node.AsDict();
                req.id = req_map.at("id").AsInt();
                req.type = req_map.at("type").AsString();

                if ((req.type == "Bus") || (req.type == "Stop")) {
                    req.name = req_map.at("name").AsString();
                    req.from ="";
                    req.to = "";
                } else {
                    req.name = "";
                    
                    if(req.type == "Route"){
                        req.from = req_map.at("from").AsString();
                        req.to = req_map.at("to").AsString();
                    }
                    else{
                        req.from ="";
                        req.to = "";
                    }
                }

                stat_request.push_back(req);
            }
        }
        
    } else {
        std::cout << "Error: base_requests is not array";
    }
}
    
void JSONReader::ParseNodeRender(const Node& node, 
                                 map_renderer::RenderSettings& rend_set){
    Dict rend_map;
    Array bus_lab_offset;
    Array stop_lab_offset;
    Array arr_color;
    Array arr_palette;
    uint8_t red_;
    uint8_t green_;
    uint8_t blue_;
    double opacity_;
 
    if (node.IsDict()) {
        rend_map = node.AsDict();
        
        try {
            rend_set.width_ = rend_map.at("width").AsDouble();
            rend_set.height_ = rend_map.at("height").AsDouble();
            rend_set.padding_ = rend_map.at("padding").AsDouble();
            rend_set.line_width_ = rend_map.at("line_width").AsDouble();
            rend_set.stop_radius_ = rend_map.at("stop_radius").AsDouble();
            rend_set.bus_label_font_size_ = rend_map.at("bus_label_font_size").AsInt();
            
            if (rend_map.at("bus_label_offset").IsArray()) {
                bus_lab_offset = rend_map.at("bus_label_offset").AsArray();
                rend_set.bus_label_offset_ = std::make_pair(bus_lab_offset[0].AsDouble(),
                                                            bus_lab_offset[1].AsDouble());
            }
            
            rend_set.stop_label_font_size_ = rend_map.at("stop_label_font_size").AsInt();
 
            if (rend_map.at("stop_label_offset").IsArray()) {
                stop_lab_offset = rend_map.at("stop_label_offset").AsArray();
                rend_set.stop_label_offset_ = std::make_pair(stop_lab_offset[0].AsDouble(),
                                                             stop_lab_offset[1].AsDouble());
            }
            
            if (rend_map.at("underlayer_color").IsString()) {
                rend_set.underlayer_color_ = svg::Color(rend_map.at("underlayer_color").AsString());
            } else if (rend_map.at("underlayer_color").IsArray()) {
                arr_color = rend_map.at("underlayer_color").AsArray();
                red_ = arr_color[0].AsInt();
                green_ = arr_color[1].AsInt();
                blue_ = arr_color[2].AsInt();
 
                if(arr_color.size() == 4){
                    opacity_ = arr_color[3].AsDouble();
                    rend_set.underlayer_color_ = svg::Color(svg::Rgba(red_, 
                                                                      green_, 
                                                                      blue_, 
                                                                      opacity_));
                } else if (arr_color.size() == 3) {
                    rend_set.underlayer_color_ = svg::Color(svg::Rgb(red_, 
                                                                     green_, 
                                                                     blue_));
                }
                
            }
 
            rend_set.underlayer_width_ = rend_map.at("underlayer_width").AsDouble();
 
            if (rend_map.at("color_palette").IsArray()) {
                arr_palette = rend_map.at("color_palette").AsArray();
                
                for (Node color_palette : arr_palette) {
                    
                    if (color_palette.IsString()) {
                        rend_set.color_palette_.push_back(svg::Color(color_palette.AsString()));
                    } else if (color_palette.IsArray()) {
                        arr_color = color_palette.AsArray();
                        red_ = arr_color[0].AsInt();
                        green_ = arr_color[1].AsInt();
                        blue_ = arr_color[2].AsInt();
 
                        if (arr_color.size() == 4) {
                            opacity_ = arr_color[3].AsDouble();
                            rend_set.color_palette_.push_back(svg::Color(svg::Rgba(red_, 
                                                                                   green_, 
                                                                                   blue_, 
                                                                                   opacity_)));
                        } else if (arr_color.size() == 3) {
                            rend_set.color_palette_.push_back(svg::Color(svg::Rgb(red_, 
                                                                                  green_, 
                                                                                  blue_)));
                        }
                    }
                }
            }            
        } catch(...) {
            std::cout << "unable to parsse init settings";
        }
        
    } else {
        std::cout << "render_settings is not map";
    }
}
    
}//end namespace json

namespace request_handler {
 
std::vector<geo::Coordinates> RequestHandler::GetStopsCoordinates(TransportCatalogue& catalogue_) const {
    
    std::vector<geo::Coordinates> stops_coordinates; 
    auto buses = catalogue_.GetBusNameToBus();
    
    for (auto& [busname, bus] : buses) {
        for (auto& stop : bus->stops) {
            geo::Coordinates coordinates;
            coordinates.latitude = stop->coordinate.first;   // latitude
            coordinates.longitude = stop->coordinate.second; // longitude
            
            stops_coordinates.push_back(coordinates);
        }
    }
    return stops_coordinates;
}

std::vector<std::string_view> RequestHandler::GetSortBusesNames(TransportCatalogue& catalogue_) const {
    std::vector<std::string_view> buses_names;
    
    auto buses = catalogue_.GetBusNameToBus();
    if (buses.size() > 0) {
        
        for (auto& [busname, bus] : buses) {
            buses_names.push_back(busname);
        }   
 
        std::sort(buses_names.begin(), buses_names.end());
        
        return buses_names;
    } else {
        return {};
    }
}

Node RequestHandler::ExecuteMakeNodeStop(int id_request, 
                                         const StopResult& stop_info){
    Node result;
    Array buses;
    Builder builder;

    std::string str_not_found = "not found";
    
    if (stop_info.not_found) {
        builder.StartDict()
               .Key("request_id").Value(id_request)
               .Key("error_message").Value(str_not_found)
               .EndDict();
        result = builder.Build();
    } else {
        builder.StartDict()
               .Key("request_id").Value(id_request)
               .Key("buses")
               .StartArray();

        for (std::string bus_name : stop_info.buses_name) {
            builder.Value(bus_name);
        }

        builder.EndArray().EndDict();
        result = builder.Build();
    }
    
    return result;
}
 
Node RequestHandler::ExecuteMakeNodeBus(int id_request, 
                                        const BusResult& bus_info){
    Node result;
    std::string str_not_found = "not found";
    
    if (bus_info.not_found) {
        result = Builder{}.StartDict()
                          .Key("request_id").Value(id_request)
                          .Key("error_message").Value(str_not_found)
                          .EndDict()
                          .Build();
    } else {
        result = Builder{}.StartDict()
                          .Key("request_id").Value(id_request)
                          .Key("curvature").Value(bus_info.curvature)
                          .Key("route_length").Value(bus_info.route_length)
                          .Key("stop_count").Value(bus_info.stops_on_route)
                          .Key("unique_stop_count").Value(bus_info.unique_stops)
                          .EndDict()
                          .Build();
    }
    
    return result;
}    
      
Node RequestHandler::ExecuteMakeNodeMap(int id_request, 
                                        TransportCatalogue& catalogue_, 
                                        RenderSettings render_settings){
    Node result;
    std::ostringstream map_stream;
    std::string map_str;
 
    MapRenderer map_catalogue(render_settings);      
    map_catalogue.InitSphereProjector(GetStopsCoordinates(catalogue_));
    ExecuteRenderMap(map_catalogue, catalogue_);
    map_catalogue.GetStreamMap(map_stream);
    map_str = map_stream.str();

    result = Builder{}.StartDict()
                      .Key("request_id").Value(id_request)
                      .Key("map").Value(map_str)
                      .EndDict()
                      .Build();
    
    return result;
}

Node RequestHandler::ExecuteMakeNodeRoute(StatRequest& request, 
                                          TransportCatalogue& catalogue, 
                                          TransportRouter& routing) {
    const auto& route_info = GetRouteInfo(request.from, request.to, catalogue, routing);

    if (!route_info) {
        return Builder{}.StartDict()
                        .Key("request_id").Value(request.id)
                        .Key("error_message").Value("not found")
                        .EndDict()
                        .Build();
    }

    Array items;
    for (const auto& item : route_info->edges) {
        items.emplace_back(std::visit(EdgeInfoGetter{}, item));
    }

    return Builder{}.StartDict()
                    .Key("request_id").Value(request.id)
                    .Key("total_time").Value(route_info->total_time)
                    .Key("items").Value(items)
                    .EndDict()
                    .Build();
}
    
void RequestHandler::ExecuteQueries(TransportCatalogue& catalogue, 
                                    std::vector<StatRequest>& stat_requests, 
                                    RenderSettings& render_settings,
                                    RoutingSettings& routing_settings){
    std::vector<Node> result_request;
    TransportRouter routing(routing_settings, catalogue);

    routing.SetRoutingSettings(routing_settings);
    routing.BuildRouter(catalogue);
    
    for (StatRequest req : stat_requests) {
        
        if (req.type == "Stop") {
            result_request.push_back(ExecuteMakeNodeStop(req.id, ExecuteQueryInfoStop(catalogue, req.name))); 
        } else if (req.type == "Bus") {
            result_request.push_back(ExecuteMakeNodeBus(req.id, ExecuteQueryInfoBus(catalogue, req.name)));
        } else if(req.type == "Map") {            
            result_request.push_back(ExecuteMakeNodeMap(req.id, catalogue, render_settings));
        } else if (req.type == "Route") {
            result_request.push_back(ExecuteMakeNodeRoute(req, catalogue, routing));
        }
        
    }
    
    doc_out = Document{Node(result_request)};
}
 
void RequestHandler::ExecuteRenderMap(MapRenderer& map_catalogue, 
                                      TransportCatalogue& catalogue) const {   
    std::vector<std::pair<Bus*, int>> buses_palette;  
    std::vector<Stop*> stops_sort;
    int palette_size = 0;
    int palette_index = 0;
    
    palette_size = map_catalogue.GetPaletteSize();
    if (palette_size == 0) {
        std::cout << "color palette is empty";
        return;
    }
 
    auto buses = catalogue.GetBusNameToBus();   
    if (buses.size() > 0) {
        
        for (std::string_view bus_name : GetSortBusesNames(catalogue)) {
            Bus* bus_info = catalogue.GetBus(bus_name);
 
            if (bus_info) {  
                if (bus_info->stops.size() > 0) {
                    buses_palette.push_back(std::make_pair(bus_info, palette_index));
                    palette_index++;
                    
                    if (palette_index == palette_size) {
                        palette_index = 0;
                    }
                }
            }
        }
 
        if (buses_palette.size() > 0) {
            map_catalogue.AddLine(buses_palette);
            map_catalogue.AddBusesName(buses_palette);            
        }          
    }
  
    auto stat = catalogue.GetStopStatistics();
    if (stat.size() > 0) {
        std::vector<std::string_view> stops_name;
 
        for (auto& [stop, buses] : stat) {
            
            if (buses.size() > 0) {
                stops_name.push_back(stop->name);
            }
        }
        
        std::sort(stops_name.begin(), stops_name.end());
        
        for(std::string_view stop_name : stops_name){
            Stop* stop = catalogue.GetStop(stop_name);
            if (stop) {
                stops_sort.push_back(stop);
            }
        }
        
        if (stops_sort.size() > 0) { 
            map_catalogue.AddStopsCircle(stops_sort);
            map_catalogue.AddStopsName(stops_sort);
        }
    }
}

std::optional<RouteInfo> RequestHandler::GetRouteInfo(std::string_view start, 
                                                      std::string_view end, 
                                                      TransportCatalogue& catalogue, 
                                                      TransportRouter& routing) const {

    
    return routing.GetRouteInfo(catalogue.GetStop(start), 
                                catalogue.GetStop(end));
}

BusResult RequestHandler::ExecuteQueryInfoBus(TransportCatalogue& catalogue, 
                                              std::string_view bus_name) {
    BusResult bus_info;
    
    bus_info = catalogue.GetBusInfo(bus_name);
    return bus_info;
}   
    
StopResult RequestHandler::ExecuteQueryInfoStop(TransportCatalogue& catalogue, 
                                                std::string_view stop_name) {
    StopResult stop_info;
 
    stop_info = catalogue.GetStopInfo(stop_name);
    return stop_info;
}

const Document& RequestHandler::GetDocument(){
    return doc_out;
}
    
} // end namespace request_handler
} // end namespace detail
} // end namespace transport_catalogue