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

#include "lanelet2_map_validator/validators/lane/lanelet_geometry.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<LaneletGeometryValidator> reg;
}

lanelet::validation::Issues LaneletGeometryValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_lanelet_geometry(map));

  return issues;
}

lanelet::validation::Issues LaneletGeometryValidator::check_lanelet_geometry(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & lanelet : map.laneletLayer) {
    std::set<lanelet::Id> left_point_ids;
    std::set<lanelet::Id> right_point_ids;

    for (const auto & point : lanelet.leftBound()) {
      left_point_ids.insert(point.id());
    }

    for (const auto & point : lanelet.rightBound()) {
      right_point_ids.insert(point.id());
    }

    std::vector<lanelet::Id> shared_point_ids;
    std::set_intersection(
      left_point_ids.begin(), left_point_ids.end(), right_point_ids.begin(), right_point_ids.end(),
      std::back_inserter(shared_point_ids));

    // Issue-001: lanelet has shared points between left and right bounds
    if (!shared_point_ids.empty()) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), lanelet.id()));
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
