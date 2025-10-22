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

#include "lanelet2_map_validator/validators/crosswalk/regulatory_element_details_for_crosswalks.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestRegulatoryElementsDetailsForCrosswalks : public MapValidationTester
{
protected:
  const std::string test_target_ = std::string(
    lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator::name());
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

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 1), 31);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, MultipleRefers)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_regulatory_element_with_multiple_refers.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 2), 31);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, MissingRefLine)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_without_stop_line.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 3), 31);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, MultipleRefLines)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_regulatory_element_with_multiple_ref_lines.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 9), 31);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, WrongRefersType)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_with_wrong_refers_type.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 6), 18);
  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 6), 18);
  const auto expected_issues = {expected_issue1, expected_issue2};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, WrongRefLineType)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_with_wrong_ref_line_type.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 7), 27);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(
  TestRegulatoryElementsDetailsForCrosswalks, MissingParticipantPedestrian)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_without_participant_pedestrian.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 10), 18);
  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 10), 18);
  const auto expected_issues = {expected_issue1, expected_issue2};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, WrongParticipantPedestrian)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_with_wrong_participant_pedestrian.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 11), 18);
  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 11), 18);
  const auto expected_issues = {expected_issue1, expected_issue2};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementsDetailsForCrosswalks, NonIntersectingLanelet)  // NOLINT for gtest
{
  load_target_map("crosswalk/crosswalk_non_intersecting_lanelet.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForCrosswalksValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 12), 32);
  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 12), 31);
  const auto expected_issues = {expected_issue1, expected_issue2};

  EXPECT_EQ(issues.size(), 2);

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
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
