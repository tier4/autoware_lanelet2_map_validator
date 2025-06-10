// Copyright 2025 Autoware Foundation
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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__INTERSECTION__VIRTUAL_TRAFFIC_LIGHT_SECTION_OVERLAP_HPP_  // NOLINT
#define LANELET2_MAP_VALIDATOR__VALIDATORS__INTERSECTION__VIRTUAL_TRAFFIC_LIGHT_SECTION_OVERLAP_HPP_  // NOLINT

#include "lanelet2_map_validator/config_store.hpp"
#include "lanelet2_map_validator/validators/intersection/virtual_traffic_light_line_order.hpp"

#include <autoware_lanelet2_extension/regulatory_elements/virtual_traffic_light.hpp>

#include <lanelet2_routing/Forward.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
class VirtualTrafficLightSectionOverlapValidator : public VirtualTrafficLightLineOrderValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name()
  {
    return "mapping.intersection.virtual_traffic_light_section_overlap";
  }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

  VirtualTrafficLightSectionOverlapValidator()
  {
    const auto parameters = ValidatorConfigStore::parameters()[name()];
    target_refers_ = get_parameter_or<std::vector<std::string>>(
      parameters, "validation_target_refers", {"intersection_coordination"});
  }

private:
  /**
   * @brief main validation code
   *
   * @param map
   * @return lanelet::validation::Issues
   */
  lanelet::validation::Issues check_virtual_traffic_light_section_overlap(
    const lanelet::LaneletMap & map);

  /**
   * @brief Get the all paths for each end_line of a virtual traffic light
   *
   * @param reg_elem
   * @param map
   * @return lanelet::routing::LaneletPaths
   */
  lanelet::routing::LaneletPaths get_all_possible_paths(
    const lanelet::RegulatoryElementConstPtr & reg_elem, const lanelet::LaneletMap & map);

  /**
   * @brief Get the bounding box of the LaneletPath object
   *
   * @param path
   * @return lanelet::BoundingBox2d
   */
  lanelet::BoundingBox2d get_lanelet_path_bbox(const lanelet::routing::LaneletPath & path);

  /**
   * @brief Get a lanelet sequence that both lanelet paths have in common
   *
   * @param path1
   * @param path2
   * @return lanelet::ConstLanelets (a.k.a vector<ConstLanelet>)
   */
  lanelet::ConstLanelets get_overlapped_lanelets(
    const lanelet::routing::LaneletPath & path1, const lanelet::routing::LaneletPath & path2);

  std::vector<std::string> target_refers_;
  lanelet::traffic_rules::TrafficRulesPtr traffic_rules_;
  lanelet::routing::RoutingGraphUPtr routing_graph_ptr_;
};
}  // namespace lanelet::autoware::validation

// clang-format off
#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__INTERSECTION__VIRTUAL_TRAFFIC_LIGHT_SECTION_OVERLAP_HPP_  // NOLINT
// clang-format on
