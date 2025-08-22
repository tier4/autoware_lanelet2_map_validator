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

#include "lanelet2_map_validator/validators/intersection/right_of_way_for_virtual_traffic_lights.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestRightOfWayForVirtualTrafficLightsValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::RightOfWayForVirtualTrafficLightsValidator::name());
};

TEST_F(TestRightOfWayForVirtualTrafficLightsValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::RightOfWayForVirtualTrafficLightsValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestRightOfWayForVirtualTrafficLightsValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::RightOfWayForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(
  TestRightOfWayForVirtualTrafficLightsValidator, MissingRightOfWayReference)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_virtual_traffic_light_missing_reference.osm");

  lanelet::autoware::validation::RightOfWayForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 2);

  if (issues.size() == 2) {
    const lanelet::Id expected_lanelet_id_1 = 50;
    const lanelet::Id expected_lanelet_id_2 = 51;

    const auto expected_issue_1 =
      construct_issue_from_code(issue_code(test_target_, 1), expected_lanelet_id_1);
    const auto expected_issue_2 =
      construct_issue_from_code(issue_code(test_target_, 1), expected_lanelet_id_2);

    bool found_issue_50 = false;
    bool found_issue_51 = false;

    for (const auto & issue : issues) {
      if (issue.id == expected_lanelet_id_1) {
        const auto difference = compare_an_issue(expected_issue_1, issue);
        EXPECT_TRUE(difference.empty()) << difference;
        found_issue_50 = true;
      } else if (issue.id == expected_lanelet_id_2) {
        const auto difference = compare_an_issue(expected_issue_2, issue);
        EXPECT_TRUE(difference.empty()) << difference;
        found_issue_51 = true;
      }
    }

    EXPECT_TRUE(found_issue_50) << "Issue should be found for lanelet ID 50";
    EXPECT_TRUE(found_issue_51) << "Issue should be found for lanelet ID 51";
  }
}

TEST_F(
  TestRightOfWayForVirtualTrafficLightsValidator, MultipleRightOfWayElements)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_virtual_traffic_light_with_multiple_elements.osm");

  lanelet::autoware::validation::RightOfWayForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_lanelet_id = 50;
    const auto expected_issue =
      construct_issue_from_code(issue_code(test_target_, 2), expected_lanelet_id);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_lanelet_id);
  }
}

TEST_F(TestRightOfWayForVirtualTrafficLightsValidator, MultipleRole)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_virtual_traffic_light_with_multiple_role.osm");

  lanelet::autoware::validation::RightOfWayForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_regulatory_element_id = 11149;
    const auto expected_issue =
      construct_issue_from_code(issue_code(test_target_, 3), expected_regulatory_element_id);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_regulatory_element_id);
  }
}

TEST_F(TestRightOfWayForVirtualTrafficLightsValidator, WrongRole)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_virtual_traffic_light_with_wrong_role.osm");

  lanelet::autoware::validation::RightOfWayForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_regulatory_element_id = 11149;
    const auto expected_issue =
      construct_issue_from_code(issue_code(test_target_, 4), expected_regulatory_element_id);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_regulatory_element_id);
  }
}

TEST_F(
  TestRightOfWayForVirtualTrafficLightsValidator, ConflictingLaneletNotYield)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_virtual_traffic_light_missing_yield_role.osm");

  lanelet::autoware::validation::RightOfWayForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_regulatory_element_id = 11149;
    std::map<std::string, std::string> substitutions;
    substitutions["conflicting_lanelet_id"] = "52";
    const auto expected_issue = construct_issue_from_code(
      issue_code(test_target_, 5), expected_regulatory_element_id, substitutions);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_regulatory_element_id);
  }
}

TEST_F(
  TestRightOfWayForVirtualTrafficLightsValidator, UnnecessaryYieldRelationship)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_virtual_traffic_light_with_unnecessary_yield.osm");

  lanelet::autoware::validation::RightOfWayForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  if (issues.size() == 1) {
    const lanelet::Id expected_regulatory_element_id = 11149;
    std::map<std::string, std::string> substitutions;
    substitutions["unnecessary_yield_to"] = "54";
    const auto expected_issue = construct_issue_from_code(
      issue_code(test_target_, 6), expected_regulatory_element_id, substitutions);
    const auto difference = compare_an_issue(expected_issue, issues[0]);
    EXPECT_TRUE(difference.empty()) << difference;
    EXPECT_EQ(issues[0].id, expected_regulatory_element_id)
      << "Issue should be for regulatory element ID 11077";
  }
}

TEST_F(
  TestRightOfWayForVirtualTrafficLightsValidator,
  CorrectRightOfWayConfiguration)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_virtual_traffic_light.osm");

  lanelet::autoware::validation::RightOfWayForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
