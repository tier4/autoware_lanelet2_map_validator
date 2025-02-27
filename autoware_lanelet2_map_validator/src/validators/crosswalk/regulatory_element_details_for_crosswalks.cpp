// Copyright 2024 Autoware Foundation
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

#include "lanelet2_map_validator/validators/crosswalk/regulatory_element_details_for_crosswalks.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <autoware_lanelet2_extension/regulatory_elements/crosswalk.hpp>
#include <range/v3/view/filter.hpp>

#include <boost/optional/optional_io.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_validation/Validation.h>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RegulatoryElementsDetailsForCrosswalksValidator> reg;
}  // namespace

lanelet::validation::Issues RegulatoryElementsDetailsForCrosswalksValidator::operator()(
  const lanelet::LaneletMap & map)
{
  // All issues found by all validators
  lanelet::validation::Issues issues;

  // Append issues found by each validator
  lanelet::autoware::validation::appendIssues(issues, checkRegulatoryElementOfCrosswalks(map));
  return issues;
}

lanelet::validation::Issues
RegulatoryElementsDetailsForCrosswalksValidator::checkRegulatoryElementOfCrosswalks(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;
  // filter elem whose Subtype is crosswalk
  auto elems = map.regulatoryElementLayer | ranges::views::filter([](auto && elem) {
                 const auto & attrs = elem->attributes();
                 const auto & it = attrs.find(lanelet::AttributeName::Subtype);
                 return it != attrs.end() && it->second == lanelet::AttributeValueString::Crosswalk;
               });

  for (const auto & elem : elems) {
    // Get lanelet of crosswalk referred by regulatory element
    auto refers = elem->getParameters<lanelet::ConstLanelet>(lanelet::RoleName::Refers);
    // Get stop line referred by regulatory element
    auto ref_lines = elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::RefLine);

    // Report error if regulatory element does not have lanelet of crosswalk
    if (refers.empty()) {
      issues.emplace_back(
        lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement,
        elem->id(),
        append_issue_code_prefix(
          this->name(), 1,
          "Regulatory element of crosswalk must have lanelet of crosswalk(refers)."));
    } else if (refers.size() > 1) {  // Report error if regulatory element has two or more lanelet
                                     // of crosswalk
      issues.emplace_back(
        lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement,
        elem->id(),
        append_issue_code_prefix(
          this->name(), 2,
          "Regulatory element of crosswalk must have only one lanelet of crosswalk(refers)."));
    }

    // Report Info if regulatory element does not have stop line
    if (ref_lines.empty()) {
      issues.emplace_back(
        lanelet::validation::Severity::Info, lanelet::validation::Primitive::RegulatoryElement,
        elem->id(),
        append_issue_code_prefix(
          this->name(), 3, "Regulatory element of crosswalk does not have stop line(ref_line)."));
    } else if (ref_lines.size() > 1) {
      issues.emplace_back(
        lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement,
        elem->id(),
        append_issue_code_prefix(
          this->name(), 9,
          "Regulatory element of crosswalk should have only one stop line(ref_line)."));
    }

    // If this is a crosswalk type regulatory element, the "refers" has to be a "crosswalk" subtype
    // lanelet
    const auto & issue_cw = lanelet::validation::Issue(
      lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet,
      lanelet::utils::getId(),
      append_issue_code_prefix(
        this->name(), 6, "Refers of crosswalk regulatory element must have type of crosswalk."));
    lanelet::autoware::validation::checkPrimitivesType(
      refers, lanelet::AttributeValueString::Lanelet, lanelet::AttributeValueString::Crosswalk,
      issue_cw, issues);

    // The refers must have an attribute participant:pedestrian and set to "yes" or "true"
    for (const lanelet::ConstLanelet & lane : refers) {
      if (!lane.hasAttribute(lanelet::AttributeName::ParticipantPedestrian)) {
        issues.emplace_back(
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, lane.id(),
          append_issue_code_prefix(
            this->name(), 10, "Attribute participant:pedestrian not found from refers."));
      } else if (!lane.attribute(lanelet::AttributeName::ParticipantPedestrian)
                    .asBool()
                    .value_or(false)) {
        issues.emplace_back(
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, lane.id(),
          append_issue_code_prefix(
            this->name(), 11,
            "Attribute participant:pedestrian of refers is not set to \"yes\" or \"true\"."));
      }
    }

    // If this is a crosswalk type regulatory element, the "ref_line" has to be a "stop_line" type
    // linestring
    const auto & issue_sl = lanelet::validation::Issue(
      lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString,
      lanelet::utils::getId(),
      append_issue_code_prefix(
        this->name(), 7, "ref_line of crosswalk regulatory element must have type of stopline."));
    lanelet::autoware::validation::checkPrimitivesType(
      ref_lines, lanelet::AttributeValueString::StopLine, issue_sl, issues);
  }
  return issues;
}

}  // namespace lanelet::autoware::validation
