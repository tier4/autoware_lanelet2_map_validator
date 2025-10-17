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

#include "lanelet2_map_validator/validators/intersection/intersection_area_validity.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <boost/geometry/algorithms/is_valid.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/Polygon.h>
#include <lanelet2_core/primitives/Polygon.h>

#include <limits>
#include <map>
#include <string>

namespace bg = boost::geometry;

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<IntersectionAreaValidityValidator> reg;
}

lanelet::validation::Issues IntersectionAreaValidityValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_intersection_area_validity(map));

  return issues;
}

lanelet::validation::Issues IntersectionAreaValidityValidator::check_intersection_area_validity(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const lanelet::ConstPolygon3d & polygon3d : map.polygonLayer) {
    if (
      !polygon3d.hasAttribute(lanelet::AttributeName::Type) ||
      polygon3d.attribute(lanelet::AttributeName::Type).value() != "intersection_area") {
      continue;
    }

    lanelet::BasicPolygon2d basic_polygon2d = lanelet::traits::to2D(polygon3d.basicPolygon());

    bg::validity_failure_type failure_type;
    bool polygon_is_valid = bg::is_valid(basic_polygon2d, failure_type);
    if (!polygon_is_valid && failure_type != bg::validity_failure_type::failure_wrong_orientation) {
      std::map<std::string, std::string> reason_map;
      reason_map["boost_geometry_message"] = bg::validity_failure_type_message(failure_type);
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 1), polygon3d.id(), reason_map));
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
