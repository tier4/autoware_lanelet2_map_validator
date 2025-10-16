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

#include "lanelet2_map_validator/validators/stop_line/regulatory_element_details_for_traffic_signs.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <range/v3/view/filter.hpp>

#include <lanelet2_core/LaneletMap.h>

#include <algorithm>
#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RegulatoryElementDetailsForTrafficSignsValidator> reg;
}

lanelet::validation::Issues RegulatoryElementDetailsForTrafficSignsValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(
    issues, check_regulatory_element_details_for_traffic_signs(map));

  return issues;
}

lanelet::validation::Issues RegulatoryElementDetailsForTrafficSignsValidator::
  check_regulatory_element_details_for_traffic_signs(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  auto traffic_sign_elements =
    map.regulatoryElementLayer | ranges::views::filter([](auto && elem) {
      return elem->hasAttribute(lanelet::AttributeName::Subtype) &&
             elem->attribute(lanelet::AttributeName::Subtype).value() == "traffic_sign";
    });

  for (const auto & regulatory_element : traffic_sign_elements) {
    auto refers =
      regulatory_element->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::Refers);
    if (refers.empty()) {
      // Issue-001: missing refers in traffic_sign regulatory element
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 1), regulatory_element->id()));
      continue;
    }

    for (const auto & refer : refers) {
      const auto & attrs = refer.attributes();
      const auto & type_it = attrs.find(lanelet::AttributeName::Type);
      const auto & subtype_it = attrs.find(lanelet::AttributeName::Subtype);
      if (
        (type_it == attrs.end() || type_it->second != "traffic_sign") ||
        (subtype_it == attrs.end() || subtype_it->second != "stop_sign")) {
        // Issue-002: Refers linestring either does not have traffic_sign type or stop_sign subtype
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 2), refer.id()));
        continue;
      }
    }

    auto ref_lines =
      regulatory_element->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::RefLine);
    if (ref_lines.empty()) {
      // Issue-003: Missing ref_line in traffic_sign regulatory element
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 3), regulatory_element->id()));
      continue;
    }

    for (const auto & ref_line : ref_lines) {
      const auto & attrs = ref_line.attributes();
      const auto & subtype_it = attrs.find(lanelet::AttributeName::Subtype);
      if (subtype_it == attrs.end() || subtype_it->second != "stop_line") {
        // Issue-004: RefLine linestring does not have stop_line subtype
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 4), ref_line.id()));
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
