// Copyright 2025 TIER IV, Inc.
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

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_traffic_rules/GenericTrafficRules.h>
#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

#include <array>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace lanelet::autoware::validation
{
class BorderSharingValidator : public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name() { return "mapping.lane.border_sharing"; }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

private:
  using Point = std::array<double, 3>;
  using Segment = std::pair<Point, Point>;

  struct Bound
  {
    lanelet::ConstLanelet lanelet;
    lanelet::ConstLineString3d line;
    std::vector<Point> points;
    bool lanelet_is_on_left;  // true if the lanelet lies on the left side of the points' direction
  };

  lanelet::validation::Issues check_border_sharing(const lanelet::LaneletMap & map);

  /**
   * @brief return whether the two point sequences are at the same position from end to end.
   * The returned value tells whether the order is reversed. Returns nullopt if not the same.
   */
  static std::optional<bool> same_position(
    const std::vector<Point> & points1, const std::vector<Point> & points2);

  /**
   * @brief return true if a fence linestring is at the same position as a part of the bound
   */
  static bool has_fence_on(const Bound & bound, const std::set<Segment> & fence_segments);

  static Segment make_segment(const Point & a, const Point & b);
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
