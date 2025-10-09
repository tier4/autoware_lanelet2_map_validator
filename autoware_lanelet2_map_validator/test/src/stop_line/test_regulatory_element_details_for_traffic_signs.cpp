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

#include "lanelet2_map_validator/validators/stop_line/regulatory_element_details_for_traffic_signs.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestRegulatoryElementDetailsForTrafficSignsValidator : public MapValidationTester
{
protected:
  const std::string test_target_ = std::string(
    lanelet::autoware::validation::RegulatoryElementDetailsForTrafficSignsValidator::name());
};

TEST_F(
  TestRegulatoryElementDetailsForTrafficSignsValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::RegulatoryElementDetailsForTrafficSignsValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

// TEST_F(TestRegulatoryElementDetailsForTrafficSignsValidator, MissingRefers)  // NOLINT for gtest
// {
//   load_target_map("stop_line/traffic_signs_without_refers.osm");

//   lanelet::autoware::validation::RegulatoryElementDetailsForTrafficSignsValidator checker;
//   const auto & issues = checker(*map_);

//   EXPECT_EQ(issues.size(), 1);
//   EXPECT_EQ(issues[0].id, 2166);
// }

TEST_F(TestRegulatoryElementDetailsForTrafficSignsValidator, InvalidRefersType)  // NOLINT for gtest
{
  load_target_map("stop_line/traffic_signs_with_invalid_refers_type.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForTrafficSignsValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 2), 2164);
  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 2), 2165);
  const auto expected_issue3 = construct_issue_from_code(issue_code(test_target_, 4), 2156);
  const auto expected_issues = {expected_issue1, expected_issue2, expected_issue3};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementDetailsForTrafficSignsValidator, MissingRefLine)  // NOLINT for gtest
{
  load_target_map("stop_line/traffic_signs_without_ref_line.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForTrafficSignsValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 3), 2166);
  EXPECT_EQ(issues.size(), 1);
  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(
  TestRegulatoryElementDetailsForTrafficSignsValidator, ExcessiveBoundingBox)  // NOLINT for gtest
{
  load_target_map("stop_line/traffic_sign_out_of_bound.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForTrafficSignsValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 5), 2166);
  EXPECT_EQ(issues.size(), 1);
  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(
  TestRegulatoryElementDetailsForTrafficSignsValidator,
  TrafficSignRegulatoryElement)  // NOLINT for gtest
{
  load_target_map("stop_line/stop_line_with_traffic_sign.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForTrafficSignsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestRegulatoryElementDetailsForTrafficSignsValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForTrafficSignsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
