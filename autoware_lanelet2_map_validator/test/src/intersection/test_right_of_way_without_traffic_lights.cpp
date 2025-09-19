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

#include "lanelet2_map_validator/validators/intersection/right_of_way_without_traffic_lights.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestRightOfWayWithoutTrafficLightsValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::RightOfWayWithoutTrafficLightsValidator::name());
};

TEST_F(TestRightOfWayWithoutTrafficLightsValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::RightOfWayWithoutTrafficLightsValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestRightOfWayWithoutTrafficLightsValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::RightOfWayWithoutTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestRightOfWayWithoutTrafficLightsValidator, MultipleRightofWayRole)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_without_traffic_light_with_multiple_role.osm");

  lanelet::autoware::validation::RightOfWayWithoutTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_regulatory_element_id = 20000;

    const auto expected_issue =
      construct_issue_from_code(issue_code(test_target_, 1), expected_regulatory_element_id);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_regulatory_element_id)
      << "Issue should be for regulatory element ID 20000";
  }
}

TEST_F(TestRightOfWayWithoutTrafficLightsValidator, WrongRightOfWayLaneletRole)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_without_traffic_light_with_wrong_role.osm");

  lanelet::autoware::validation::RightOfWayWithoutTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_regulatory_element_id = 20000;

    const auto expected_issue =
      construct_issue_from_code(issue_code(test_target_, 2), expected_regulatory_element_id);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_regulatory_element_id)
      << "Issue should be for regulatory element ID 20000";
  }
}

TEST_F(TestRightOfWayWithoutTrafficLightsValidator, MissingYieldRole)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_without_traffic_light_missing_yield_role.osm");

  lanelet::autoware::validation::RightOfWayWithoutTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_regulatory_element_id = 20000;

    std::map<std::string, std::string> reason_map;
    reason_map["conflicting_lanelet_id"] = "60";
    reason_map["turn_direction"] = "straight";

    const auto expected_issue = construct_issue_from_code(
      issue_code(test_target_, 3), expected_regulatory_element_id, reason_map);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_regulatory_element_id)
      << "Issue should be for regulatory element ID 20000";
  }
}

TEST_F(TestRightOfWayWithoutTrafficLightsValidator, UnnecessaryYieldRole)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_without_traffic_light_with_unnecessary_yield.osm");

  lanelet::autoware::validation::RightOfWayWithoutTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_regulatory_element_id = 20000;

    std::map<std::string, std::string> reason_map;
    reason_map["unnecessary_yield_to"] = "52";
    reason_map["turn_direction"] = "straight";

    const auto expected_issue = construct_issue_from_code(
      issue_code(test_target_, 4), expected_regulatory_element_id, reason_map);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_regulatory_element_id)
      << "Issue should be for regulatory element ID 20000";
  }
}

TEST_F(
  TestRightOfWayWithoutTrafficLightsValidator, MissingRightOfWayIntersection)  // NOLINT for gtest
{
  load_target_map(
    "intersection/right_of_way_without_traffic_light_missing_right_of_way_intersection.osm");

  lanelet::autoware::validation::RightOfWayWithoutTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_intersection_id = 10803;

    std::map<std::string, std::string> reason_map;
    reason_map["intersection_id"] = "10803";

    const auto expected_issue =
      construct_issue_from_code(issue_code(test_target_, 5), expected_intersection_id, reason_map);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_intersection_id)
      << "Issue should be for intersection area ID 10803";
  }
}
