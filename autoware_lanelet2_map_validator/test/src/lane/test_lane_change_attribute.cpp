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

#include "lanelet2_map_validator/validators/lane/lane_change_attribute.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestLaneChangeAttributeValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::LaneChangeAttributeValidator::name());
};

TEST_F(TestLaneChangeAttributeValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::LaneChangeAttributeValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestLaneChangeAttributeValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::LaneChangeAttributeValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestLaneChangeAttributeValidator, MissingLeftLaneChangeAttribute)  // NOLINT for gtest
{
  load_target_map("lane/lane_change_attribute_missing_attribute.osm");

  lanelet::autoware::validation::LaneChangeAttributeValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> bound_type_map;
  bound_type_map["bound_type"] = "maybe";
  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 1), 10, bound_type_map);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestLaneChangeAttributeValidator, InvalidLaneChangeValues)  // NOLINT for gtest
{
  load_target_map("lane/lane_change_attribute_with_wrong_value.osm");

  lanelet::autoware::validation::LaneChangeAttributeValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> left_bound_map;
  left_bound_map["bound_type"] = "left";
  left_bound_map["invalid_value"] = "maybe";
  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 2), 10, left_bound_map);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}
