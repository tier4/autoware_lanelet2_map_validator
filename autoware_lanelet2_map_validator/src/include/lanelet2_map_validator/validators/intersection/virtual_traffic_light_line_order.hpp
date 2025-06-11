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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__INTERSECTION__VIRTUAL_TRAFFIC_LIGHT_LINE_ORDER_HPP_
#define LANELET2_MAP_VALIDATOR__VALIDATORS__INTERSECTION__VIRTUAL_TRAFFIC_LIGHT_LINE_ORDER_HPP_  // NOLINT

#include "lanelet2_map_validator/config_store.hpp"

#include "lanelet2_map_validator/config_store.hpp"

#include <lanelet2_routing/LaneletPath.h>
#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
class VirtualTrafficLightLineOrderValidator : public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name()
  {
    return "mapping.intersection.virtual_traffic_light_line_order";
  }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

  VirtualTrafficLightLineOrderValidator()
  {
    const auto parameters = ValidatorConfigStore::parameters()[name()];
    target_refers_ = get_parameter_or<std::vector<std::string>>(
      parameters, "validation_target_refers", {"intersection_coordination"});
  }

protected:
  bool is_target_virtual_traffic_light(const lanelet::RegulatoryElementConstPtr & reg_elem);
  lanelet::Optional<lanelet::ConstLanelet> belonging_lanelet(
    const lanelet::ConstLineString3d & linestring, const lanelet::LaneletMap & map);
  lanelet::ConstLineString3d get_aligned_linestring(
    const lanelet::ConstLineString3d & linestring, const lanelet::CompoundLineString3d & base_arc);
  lanelet::Optional<lanelet::ConstLineString3d> select_end_line(
    const lanelet::ConstLineStrings3d & candidate_lines,
    const lanelet::ConstLanelet & base_lanelet);
  double intersection_ratio(
    const lanelet::ConstLineString3d & linestring, const lanelet::CompoundPolygon3d & polygon);
  double intersection_ratio(
    const lanelet::ConstLineString3d & linestring, const lanelet::ConstLanelet & lane);

private:
  lanelet::validation::Issues check_virtual_traffic_light_line_order(
    const lanelet::LaneletMap & map);
  lanelet::CompoundLineString3d get_concatenated_bound(
    const lanelet::routing::LaneletPath & path, const bool get_left = true);
  bool is_ordered_in_length_manner(
    const lanelet::ConstPoint3d & p1, const lanelet::ConstPoint3d & p2,
    const lanelet::ConstPoint3d & p3, const lanelet::CompoundLineString3d & base_arc);

  std::vector<std::string> target_refers_;
};
}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__INTERSECTION__VIRTUAL_TRAFFIC_LIGHT_LINE_ORDER_HPP_
        // // NOLINT
