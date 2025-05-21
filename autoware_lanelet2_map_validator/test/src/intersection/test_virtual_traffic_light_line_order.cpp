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

#include "lanelet2_map_validator/validators/intersection/virtual_traffic_light_line_order.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestVirtualTrafficLightLineOrderValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator::name());
};

TEST_F(TestVirtualTrafficLightLineOrderValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, DerailedStartLine)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_derailed_start_line.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 1), 11101);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, EndLineCoveringWrongLanelet)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_end_line_covering_wrong_lanelet.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> id_map;
  id_map["id"] = "52";
  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 2), 11074, id_map);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, DerailedEndLine)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_derailed_end_line.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> id_map;
  id_map["id"] = "52";
  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 2), 11074, id_map);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, StartLineCoveringReferrer)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_start_line_covering_referrer.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 3), 11101);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, InvalidPath)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_invalid_path.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> id_map;
  id_map["id"] = "11086";
  const auto expected_issue1 =
    construct_issue_from_code(issue_code(test_target_, 4), 11074, id_map);
  id_map["id"] = "11089";
  const auto expected_issue2 =
    construct_issue_from_code(issue_code(test_target_, 4), 11074, id_map);
  id_map["id"] = "11091";
  const auto expected_issue3 =
    construct_issue_from_code(issue_code(test_target_, 4), 11074, id_map);

  const lanelet::validation::Issues expected_issues = {
    expected_issue1, expected_issue2, expected_issue3};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, DerailedStopLine)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_derailed_stop_line.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> id_map;
  id_map["id"] = "11086";
  const auto expected_issue1 =
    construct_issue_from_code(issue_code(test_target_, 5), 11104, id_map);
  id_map["id"] = "11089";
  const auto expected_issue2 =
    construct_issue_from_code(issue_code(test_target_, 5), 11104, id_map);
  id_map["id"] = "11091";
  const auto expected_issue3 =
    construct_issue_from_code(issue_code(test_target_, 5), 11104, id_map);

  const lanelet::validation::Issues expected_issues = {
    expected_issue1, expected_issue2, expected_issue3};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, StopLineNotOnPath)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_stop_line_out_of_path.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> id_map;
  id_map["id"] = "11086";
  const auto expected_issue1 =
    construct_issue_from_code(issue_code(test_target_, 5), 11107, id_map);
  id_map["id"] = "11089";
  const auto expected_issue2 =
    construct_issue_from_code(issue_code(test_target_, 5), 11107, id_map);
  id_map["id"] = "11091";
  const auto expected_issue3 =
    construct_issue_from_code(issue_code(test_target_, 5), 11107, id_map);

  const lanelet::validation::Issues expected_issues = {
    expected_issue1, expected_issue2, expected_issue3};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, MessyStopLine)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_messy_stop_line.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> id_map;
  id_map["id"] = "11086";
  const auto expected_issue1 =
    construct_issue_from_code(issue_code(test_target_, 6), 11074, id_map);
  id_map["id"] = "11089";
  const auto expected_issue2 =
    construct_issue_from_code(issue_code(test_target_, 6), 11074, id_map);
  id_map["id"] = "11091";
  const auto expected_issue3 =
    construct_issue_from_code(issue_code(test_target_, 6), 11074, id_map);

  const lanelet::validation::Issues expected_issues = {
    expected_issue1, expected_issue2, expected_issue3};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, OrderedStopStartEnd)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_ordered_stop_start_end.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> id_map;
  id_map["id"] = "11086";
  const auto expected_issue1 =
    construct_issue_from_code(issue_code(test_target_, 6), 11074, id_map);
  id_map["id"] = "11089";
  const auto expected_issue2 =
    construct_issue_from_code(issue_code(test_target_, 6), 11074, id_map);
  id_map["id"] = "11091";
  const auto expected_issue3 =
    construct_issue_from_code(issue_code(test_target_, 6), 11074, id_map);

  const lanelet::validation::Issues expected_issues = {
    expected_issue1, expected_issue2, expected_issue3};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, OrderedStartEndStop)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_ordered_start_end_stop.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> id_map;
  id_map["id"] = "11089";
  const auto expected_issue1 =
    construct_issue_from_code(issue_code(test_target_, 6), 11074, id_map);
  id_map["id"] = "11086";
  const auto expected_issue2 =
    construct_issue_from_code(issue_code(test_target_, 5), 11099, id_map);
  id_map["id"] = "11091";
  const auto expected_issue3 =
    construct_issue_from_code(issue_code(test_target_, 5), 11099, id_map);

  const lanelet::validation::Issues expected_issues = {
    expected_issue1, expected_issue2, expected_issue3};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, CorrectVirtualTrafficLight)  // NOLINT for gtest
{
  load_target_map("intersection/correct_virtual_traffic_light.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
