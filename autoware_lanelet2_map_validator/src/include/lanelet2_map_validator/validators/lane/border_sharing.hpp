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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__BORDER_SHARING_HPP_
#define LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__BORDER_SHARING_HPP_

#include "lanelet2_map_validator/config_store.hpp"

#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/GenericTrafficRules.h>
#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

#include <string>

namespace lanelet::autoware::validation
{
class BorderSharingValidator : public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name() { return "mapping.lane.border_sharing"; }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

  BorderSharingValidator()
  {
    const auto parameters = ValidatorConfigStore::parameters()[name()];
    iou_threshold_ = get_parameter_or<double>(parameters, "iou_threshold", 0.05);
  }

private:
  lanelet::validation::Issues check_border_sharing(const lanelet::LaneletMap & map);

  /**
   * @brief return a polygon which is an expanded shape of the lanelet. each point will be expanded
   * to the lateral direction, and its variation length is about (average length of both edges of
   * the lanelet) * (scale_factor-1)
   */
  lanelet::BasicPolygon2d expanded_lanelet_polygon(
    const lanelet::ConstLanelet & lane, const double & scale_factor);

  lanelet::routing::RelationType get_relation(
    const lanelet::routing::RoutingGraphUPtr & routing_graph_ptr, const lanelet::ConstLanelet from,
    const lanelet::ConstLanelet to);

  /**
   * @brief return IoU
   */
  double intersection_over_union(
    const lanelet::BasicPolygon2d & polygon1, const lanelet::BasicPolygon2d & polygon2);

  double iou_threshold_;
};

namespace traffic_rules
{
class BorderValidationRules : public lanelet::traffic_rules::GenericTrafficRules
{
public:
  using GenericTrafficRules::GenericTrafficRules;

protected:
  Optional<bool> canPass(const RegulatoryElementConstPtrs & /*regElems*/) const override
  {
    return {};
  }
  Optional<bool> canPass(const std::string & type, const std::string & /*location*/) const override;

  const lanelet::traffic_rules::CountrySpeedLimits & countrySpeedLimits() const override
  {
    return speedLimits_;
  }
  Optional<lanelet::traffic_rules::SpeedLimitInformation> speedLimit(
    const RegulatoryElementConstPtrs & /*regelems*/) const override
  {
    return {};
  };

private:
  lanelet::traffic_rules::CountrySpeedLimits speedLimits_;
};
}  // namespace traffic_rules
}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__BORDER_SHARING_HPP_
