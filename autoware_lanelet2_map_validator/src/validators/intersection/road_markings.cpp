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

#include "lanelet2_map_validator/validators/intersection/road_markings.hpp"

#include "lanelet2_map_validator/utils.hpp"

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RoadMarkingsValidator> reg;
}

lanelet::validation::Issues RoadMarkingsValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_road_markings(map));

  return issues;
}

lanelet::validation::Issues RoadMarkingsValidator::check_road_markings(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & reg_elem : map.regulatoryElementLayer) {
    if (reg_elem->attributeOr(lanelet::AttributeName::Subtype, "") != std::string("road_marking")) {
      continue;
    }

    const lanelet::ConstLineStrings3d refers =
      reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::Refers);
    const lanelet::ConstLineStrings3d ref_lines =
      reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::RefLine);

    if (refers.size() != 1 || ref_lines.size() != 0) {
      // Issue-001: road_marking should have exactly one refers linestring or no ref_lines
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), reg_elem->id()));
      continue;
    }

    for (const lanelet::ConstLineString3d & refer : refers) {
      if (
        refer.attributeOr(lanelet::AttributeName::Type, "") !=
        std::string(lanelet::AttributeValueString::StopLine)) {
        // Issue-002: refers linestring must be of type stop_line
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 2), refer.id()));
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
