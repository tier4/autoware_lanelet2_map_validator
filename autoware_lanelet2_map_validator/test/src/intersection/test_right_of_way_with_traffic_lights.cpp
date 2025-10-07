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

#include "lanelet2_map_validator/utils.hpp"
#include "lanelet2_map_validator/validators/intersection/right_of_way_with_traffic_lights.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestRightOfWayWithTrafficLights : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator::name());
};

TEST_F(TestRightOfWayWithTrafficLights, ValidConfiguration)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestRightOfWayWithTrafficLights, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestRightOfWayWithTrafficLights, MissingRightOfWayReference)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_traffic_light_missing_reference.osm");

  lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_lanelet_id = 53;

    std::map<std::string, std::string> reason_map;
    reason_map["turn_direction"] = "left";

    const auto expected_issue =
      construct_issue_from_code(issue_code(test_target_, 1), expected_lanelet_id, reason_map);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_lanelet_id);
  }
}

TEST_F(TestRightOfWayWithTrafficLights, WrongRightOfWay)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_traffic_light_with_wrong_role_lanelet_reference.osm");

  lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_regulatory_element_id = 11147;
    const auto expected_issue =
      construct_issue_from_code(issue_code(test_target_, 3), expected_regulatory_element_id);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_regulatory_element_id);
  }
}

TEST_F(TestRightOfWayWithTrafficLights, MultipleRightOfWayElements)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_traffic_light_multiple_elements.osm");

  lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_lanelet_id = 53;
    const auto expected_issue =
      construct_issue_from_code(issue_code(test_target_, 2), expected_lanelet_id);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_lanelet_id);
  }
}

TEST_F(TestRightOfWayWithTrafficLights, MissingRequiredYield)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_traffic_light_missing_yield.osm");

  lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_regulatory_element_id = 11147;

    std::map<std::string, std::string> reason_map;
    reason_map["missing_yield_to"] = "49";
    reason_map["turn_direction"] = "right";

    const auto expected_issue = construct_issue_from_code(
      issue_code(test_target_, 4), expected_regulatory_element_id, reason_map);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_regulatory_element_id);
  }
}

TEST_F(TestRightOfWayWithTrafficLights, UnnecessaryYield)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_traffic_light_unnecessary_yield.osm");

  lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_regulatory_element_id = 11147;

    std::map<std::string, std::string> reason_map;
    reason_map["unnecessary_yield_to"] = "50";
    reason_map["turn_direction"] = "right";

    const auto expected_issue = construct_issue_from_code(
      issue_code(test_target_, 5), expected_regulatory_element_id, reason_map);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_regulatory_element_id);
  }
}
