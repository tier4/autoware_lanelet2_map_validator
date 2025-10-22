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

#include "lanelet2_map_validator/validators/traffic_light/body_height.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestBodyHeightValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::BodyHeightValidator::name());
};

TEST_F(TestBodyHeightValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name = lanelet::autoware::validation::BodyHeightValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestBodyHeightValidator, MissingHeightAttribute)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_without_height_tag.osm");

  lanelet::autoware::validation::BodyHeightValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 1), 48);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestBodyHeightValidator, InvalidHeightValues)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_with_invalid_height.osm");

  lanelet::autoware::validation::BodyHeightValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 2);

  std::map<std::string, std::string> reason_map1;
  reason_map1["actual_height"] = "0.150000";
  reason_map1["min_height"] = "0.200000";
  reason_map1["max_height"] = "2.000000";
  const auto expected_issue1 =
    construct_issue_from_code(issue_code(test_target_, 2), 48, reason_map1);

  std::map<std::string, std::string> reason_map2;
  reason_map2["actual_height"] = "-1.000000";
  reason_map2["min_height"] = "0.200000";
  reason_map2["max_height"] = "2.000000";
  const auto expected_issue2 =
    construct_issue_from_code(issue_code(test_target_, 2), 62, reason_map2);

  lanelet::validation::Issues expected_issues;
  expected_issues.push_back(expected_issue1);
  expected_issues.push_back(expected_issue2);

  const auto differences = compare_issues(expected_issues, issues);
  EXPECT_TRUE(differences.empty()) << differences;
}

TEST_F(TestBodyHeightValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::BodyHeightValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
