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

#include "lanelet2_map_validator/validators/area/no_parking_area.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestNoParkingAreaValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::NoParkingAreaValidator::name());
};

TEST_F(TestNoParkingAreaValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::NoParkingAreaValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestNoParkingAreaValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::NoParkingAreaValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestNoParkingAreaValidator, ValidNoParkingArea)  // NOLINT for gtest
{
  load_target_map("area/no_parking_area.osm");

  lanelet::autoware::validation::NoParkingAreaValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestNoParkingAreaValidator, RegulatoryElementNotReferredByRoadLanelet)  // NOLINT for gtest
{
  load_target_map("area/no_parking_area_without_road_lanelet_refer.osm");

  lanelet::autoware::validation::NoParkingAreaValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  const lanelet::Id expected_regulatory_element_id = 4050;
  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 4), expected_regulatory_element_id);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
  EXPECT_EQ(issues[0].id, expected_regulatory_element_id);
}

TEST_F(TestNoParkingAreaValidator, RegulatoryElementWithWrongPolygonType)  // NOLINT for gtest
{
  load_target_map("area/no_parking_area_without_polygon_refer.osm");

  lanelet::autoware::validation::NoParkingAreaValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::Id expected_reg_elem_id = 4050;
  const lanelet::Id expected_polygon_id = 4027;

  const auto expected_issue_1 =
    construct_issue_from_code(issue_code(test_target_, 1), expected_polygon_id);
  const auto expected_issue_3 =
    construct_issue_from_code(issue_code(test_target_, 3), expected_reg_elem_id);
  const auto expected_issues = {expected_issue_1, expected_issue_3};

  EXPECT_EQ(issues.size(), 2);
  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestNoParkingAreaValidator, RegulatoryElementWithMultiplePolygons)  // NOLINT for gtest
{
  load_target_map("area/no_parking_area_with_multiple_polygon_refer.osm");

  lanelet::autoware::validation::NoParkingAreaValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  const lanelet::Id expected_regulatory_element_id = 4050;
  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 2), expected_regulatory_element_id);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
  EXPECT_EQ(issues[0].id, expected_regulatory_element_id);
}
