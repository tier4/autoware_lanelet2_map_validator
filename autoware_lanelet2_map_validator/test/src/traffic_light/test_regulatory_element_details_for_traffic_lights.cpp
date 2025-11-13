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

#include "lanelet2_map_validator/validators/traffic_light/regulatory_element_details_for_traffic_lights.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestRegulatoryElementDetailsForTrafficLights : public MapValidationTester
{
protected:
  const std::string test_target_ = std::string(
    lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator::name());
};

TEST_F(TestRegulatoryElementDetailsForTrafficLights, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestRegulatoryElementDetailsForTrafficLights, MissingRefers)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_regulatory_element_without_refers.osm");

  // traffic_light-type regulatory elements that has no refers will not be loaded from the start
  // and should be mentioned in the loading_errors

  bool found_error_on_loading = false;
  int target_primitive_id = 1025;
  std::string target_message =
    "\t- Error parsing primitive " + std::to_string(target_primitive_id) +
    ": Creating a regulatory element of type traffic_light failed: No traffic light defined!";

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Point, lanelet::InvalId,
    target_message);

  for (const auto & detected_issues : loading_issues_) {
    for (const auto & issue : detected_issues.issues) {
      if (is_same_issue(issue, expected_issue)) {
        found_error_on_loading = true;
        break;
      }
    }
  }

  EXPECT_TRUE(found_error_on_loading);
}

TEST_F(TestRegulatoryElementDetailsForTrafficLights, MissingRefLine)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_regulatory_element_without_ref_line.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 1), 1025);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementDetailsForTrafficLights, WrongRefersType)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_with_wrong_refers_type.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 2), 416);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementDetailsForTrafficLights, WrongRefLineType)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_with_wrong_ref_line_type.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 3), 414);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementDetailsForTrafficLights, WrongLightBulbsType)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_with_wrong_light_bulbs_type.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 4), 415);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementDetailsForTrafficLights, MissingTrafficLightId)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_without_traffic_light_id.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 5), 415);
  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 7), 1025);
  const auto expected_issues = {expected_issue1, expected_issue2};

  const auto difference = compare_issues(expected_issues, issues);

  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(
  TestRegulatoryElementDetailsForTrafficLights,
  LightBulbsMissingFromRegulatoryElement)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_without_light_bulbs_in_regulatory_element.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 6), 1025);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementDetailsForTrafficLights, LightBulbsNotEnough)  // NOLINT for gtest
{
  load_target_map("traffic_light/crosswalk_with_traffic_lights_but_not_enough_light_bulbs.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 6), 131);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementDetailsForTrafficLights, LightBulbsNotOneByOne)  // NOLINT for gtest
{
  load_target_map("traffic_light/crosswalk_with_traffic_lights_but_light_bulbs_not_one_by_one.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 7), 131);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRegulatoryElementDetailsForTrafficLights, CorrectDetails)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_with_regulatory_element.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestRegulatoryElementDetailsForTrafficLights, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(
  TestRegulatoryElementDetailsForTrafficLights, BoundingBoxExceedsThreshold)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_out_of_bound.osm");

  lanelet::autoware::validation::RegulatoryElementsDetailsForTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 8), 1025);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}
