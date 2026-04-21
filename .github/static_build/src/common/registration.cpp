// Copyright 2024-2025 TIER IV, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <autoware_lanelet2_extension/regulatory_elements/autoware_traffic_light.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/bus_stop_area.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/crosswalk.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/detection_area.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/no_parking_area.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/no_stopping_area.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/road_marking.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/virtual_traffic_light.hpp>

#include <lanelet2_core/primitives/RegulatoryElement.h>
#include <lanelet2_io/io_handlers/Factory.h>
#include <lanelet2_io/io_handlers/OsmHandler.h>
#include <lanelet2_traffic_rules/GermanTrafficRules.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

namespace lanelet
{
namespace io_handlers
{
RegisterParser<OsmParser> regParser;
}

namespace traffic_rules
{
RegisterTrafficRules<GermanVehicle> gvRules(Locations::Germany, Participants::Vehicle);
RegisterTrafficRules<GermanPedestrian> gpRules(Locations::Germany, Participants::Pedestrian);
RegisterTrafficRules<GermanBicycle> gbRules(Locations::Germany, Participants::Bicycle);
}  // namespace traffic_rules

namespace autoware
{
RegisterRegulatoryElement<AutowareTrafficLight> regAutowareTrafficLight2;
RegisterRegulatoryElement<BusStopArea> regBusStopArea2;
RegisterRegulatoryElement<Crosswalk> regCrosswalk2;
RegisterRegulatoryElement<DetectionArea> regDetectionArea2;
RegisterRegulatoryElement<NoParkingArea> regNoParkingArea2;
RegisterRegulatoryElement<NoStoppingArea> regNoStoppingArea2;
RegisterRegulatoryElement<RoadMarking> regRoadMarking2;
RegisterRegulatoryElement<VirtualTrafficLight> regVirtualTrafficLight2;
}  // namespace autoware
}  // namespace lanelet
