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
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestRegulatoryElementsDetailsForCrosswalks : public MapValidationTester
{
private:
};

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, MissingRefers)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_regulatory_element_without_refers.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  EXPECT_EQ(issues[0].id, 31);
  EXPECT_EQ(issues[0].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[0].primitive, lanelet::validation::Primitive::RegulatoryElement);
  EXPECT_EQ(
    issues[0].message,
    "[Crosswalk.RegulatoryElementDetails-001] Regulatory element of crosswalk must have "
    "lanelet of crosswalk(refers).");
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, MultipleRefers)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_regulatory_element_with_multiple_refers.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  EXPECT_EQ(issues[0].id, 31);
  EXPECT_EQ(issues[0].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[0].primitive, lanelet::validation::Primitive::RegulatoryElement);
  EXPECT_EQ(
    issues[0].message,
    "[Crosswalk.RegulatoryElementDetails-002] Regulatory element of crosswalk must have "
    "only one lanelet of crosswalk(refers).");
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, MissingRefLine)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_without_stop_line.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  EXPECT_EQ(issues[0].id, 31);
  EXPECT_EQ(issues[0].severity, lanelet::validation::Severity::Info);
  EXPECT_EQ(issues[0].primitive, lanelet::validation::Primitive::RegulatoryElement);
  EXPECT_EQ(
    issues[0].message,
    "[Crosswalk.RegulatoryElementDetails-003] Regulatory element of crosswalk does not "
    "have stop line(ref_line).");
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, MultipleRefLines)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_regulatory_element_with_multiple_ref_lines.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  EXPECT_EQ(issues[0].id, 31);
  EXPECT_EQ(issues[0].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[0].primitive, lanelet::validation::Primitive::RegulatoryElement);
  EXPECT_EQ(
    issues[0].message,
    "[Crosswalk.RegulatoryElementDetails-009] Regulatory element of crosswalk should have only one "
    "stop line(ref_line).");
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, WrongRefersType)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_with_wrong_refers_type.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 2);
  EXPECT_EQ(issues[0].id, 18);
  EXPECT_EQ(issues[1].id, 18);
  EXPECT_EQ(issues[0].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[1].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[0].primitive, lanelet::validation::Primitive::Lanelet);
  EXPECT_EQ(issues[1].primitive, lanelet::validation::Primitive::Lanelet);
  EXPECT_EQ(
    issues[0].message,
    "[Crosswalk.RegulatoryElementDetails-006] Refers of crosswalk regulatory element must "
    "have type of crosswalk.");
  EXPECT_EQ(
    issues[1].message,
    "[Crosswalk.RegulatoryElementDetails-006] Refers of crosswalk regulatory element must "
    "have type of crosswalk.");
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, WrongRefLineType)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_with_wrong_ref_line_type.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  EXPECT_EQ(issues[0].id, 27);
  EXPECT_EQ(issues[0].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[0].primitive, lanelet::validation::Primitive::LineString);
  EXPECT_EQ(
    issues[0].message,
    "[Crosswalk.RegulatoryElementDetails-007] ref_line of crosswalk regulatory element "
    "must have type of stopline.");
}

TEST_F(
  TestRegulatoryElementsDetailsForCrosswalks, MissingParticipantPedestrian)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_without_participant_pedestrian.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 2);
  EXPECT_EQ(issues[0].id, 18);
  EXPECT_EQ(issues[1].id, 18);
  EXPECT_EQ(issues[0].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[1].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[0].primitive, lanelet::validation::Primitive::Lanelet);
  EXPECT_EQ(issues[1].primitive, lanelet::validation::Primitive::Lanelet);
  EXPECT_EQ(
    issues[0].message,
    "[Crosswalk.RegulatoryElementDetails-010] Attribute participant:pedestrian not found from "
    "refers.");
  EXPECT_EQ(
    issues[1].message,
    "[Crosswalk.RegulatoryElementDetails-010] Attribute participant:pedestrian not found from "
    "refers.");
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, WrongParticipantPedestrian)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_with_wrong_participant_pedestrian.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 2);
  EXPECT_EQ(issues[0].id, 18);
  EXPECT_EQ(issues[1].id, 18);
  EXPECT_EQ(issues[0].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[1].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[0].primitive, lanelet::validation::Primitive::Lanelet);
  EXPECT_EQ(issues[1].primitive, lanelet::validation::Primitive::Lanelet);
  EXPECT_EQ(
    issues[0].message,
    "[Crosswalk.RegulatoryElementDetails-011] Attribute participant:pedestrian of refers is not "
    "set to \"yes\" or \"true\".");
  EXPECT_EQ(
    issues[1].message,
    "[Crosswalk.RegulatoryElementDetails-011] Attribute participant:pedestrian of refers is not "
    "set to \"yes\" or \"true\".");
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, CorrectDetails)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_with_regulatory_element.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  uint64_t errors_and_warnings_count = 0;

  for (const auto & issue : issues) {
    if (
      issue.severity == lanelet::validation::Severity::Error ||
      issue.severity == lanelet::validation::Severity::Warning) {
      errors_and_warnings_count++;
    }
  }

  EXPECT_EQ(errors_and_warnings_count, 0);
  EXPECT_EQ(issues.size(), 4);  // Four INFOs should appear
}
