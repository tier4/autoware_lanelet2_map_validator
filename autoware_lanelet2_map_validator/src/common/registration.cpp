#include <lanelet2_io/io_handlers/Factory.h>
#include <lanelet2_io/io_handlers/OsmHandler.h>

#include <lanelet2_traffic_rules/TrafficRulesFactory.h>
#include <lanelet2_traffic_rules/GermanTrafficRules.h>

#include <lanelet2_core/primitives/RegulatoryElement.h>
#include <autoware_lanelet2_extension/regulatory_elements/autoware_traffic_light.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/bus_stop_area.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/crosswalk.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/detection_area.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/no_parking_area.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/no_stopping_area.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/road_marking.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/virtual_traffic_light.hpp>

namespace lanelet {
namespace io_handlers {
  RegisterParser<OsmParser> regParser;
}

namespace traffic_rules {
  RegisterTrafficRules<GermanVehicle> gvRules(Locations::Germany, Participants::Vehicle);
  RegisterTrafficRules<GermanPedestrian> gpRules(Locations::Germany, Participants::Pedestrian);
  RegisterTrafficRules<GermanBicycle> gbRules(Locations::Germany, Participants::Bicycle);
}

namespace autoware {
  RegisterRegulatoryElement<AutowareTrafficLight> regAutowareTrafficLight2;
  RegisterRegulatoryElement<BusStopArea> regBusStopArea2;
  RegisterRegulatoryElement<Crosswalk> regCrosswalk2;
  RegisterRegulatoryElement<DetectionArea> regDetectionArea2;
  RegisterRegulatoryElement<NoParkingArea> regNoParkingArea2;
  RegisterRegulatoryElement<NoStoppingArea> regNoStoppingArea2;
  RegisterRegulatoryElement<RoadMarking> regRoadMarking2;
  RegisterRegulatoryElement<VirtualTrafficLight> regVirtualTrafficLight2;
}
}
