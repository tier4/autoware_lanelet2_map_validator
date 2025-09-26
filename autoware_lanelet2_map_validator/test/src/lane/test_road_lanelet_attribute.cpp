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

#include "lanelet2_map_validator/validators/lane/road_lanelet_attribute.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestRoadLaneletAttributeValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::RoadLaneletAttributeValidator::name());
};

TEST_F(TestRoadLaneletAttributeValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::RoadLaneletAttributeValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestRoadLaneletAttributeValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::RoadLaneletAttributeValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestRoadLaneletAttributeValidator, ValidRoadLanelet)  // NOLINT for gtest
{
  load_target_map("lane/single_lanelet.osm");

  lanelet::autoware::validation::RoadLaneletAttributeValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestRoadLaneletAttributeValidator, WrongLocationAttributes)  // NOLINT for gtest
{
  load_target_map("lane/road_lanelet_wrong_location_attributes.osm");

  lanelet::autoware::validation::RoadLaneletAttributeValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 1), 24);

  const auto difference = compare_an_issue(expected_issue1, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRoadLaneletAttributeValidator, WrongOneWayAttributes)  // NOLINT for gtest
{
  load_target_map("lane/road_lanelet_wrong_one_way_attributes.osm");

  lanelet::autoware::validation::RoadLaneletAttributeValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 2), 24);

  const auto difference = compare_an_issue(expected_issue2, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}
