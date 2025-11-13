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

#include "lanelet2_map_validator/validators/area/detection_area.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestDetectionAreaValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::DetectionAreaValidator::name());
};

TEST_F(TestDetectionAreaValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::DetectionAreaValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestDetectionAreaValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::DetectionAreaValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestDetectionAreaValidator, ValidDetectionArea)  // NOLINT for gtest
{
  load_target_map("area/detection_area.osm");

  lanelet::autoware::validation::DetectionAreaValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestDetectionAreaValidator, DetectionAreaWithoutRoadLaneletRef)  // NOLINT for gtest
{
  load_target_map("area/detection_area_without_road_lanelet_ref.osm");

  lanelet::autoware::validation::DetectionAreaValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  const lanelet::Id expected_reg_elem_id = 2102;
  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 5), expected_reg_elem_id);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
  EXPECT_EQ(issues[0].id, expected_reg_elem_id);
}

TEST_F(TestDetectionAreaValidator, DetectionAreaWithMultiplePolygonRefer)  // NOLINT for gtest
{
  load_target_map("area/detection_area_with_multiple_polygon_refer.osm");

  lanelet::autoware::validation::DetectionAreaValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  const lanelet::Id expected_reg_elem_id = 2102;
  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 2), expected_reg_elem_id);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
  EXPECT_EQ(issues[0].id, expected_reg_elem_id);
}

TEST_F(TestDetectionAreaValidator, DetectionAreaWithInvalidRefLine)  // NOLINT for gtest
{
  load_target_map("area/detection_area_with_invalid_ref_line.osm");

  lanelet::autoware::validation::DetectionAreaValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  const lanelet::Id expected_reg_elem_id = 2102;
  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 4), expected_reg_elem_id);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
  EXPECT_EQ(issues[0].id, expected_reg_elem_id);
}

TEST_F(TestDetectionAreaValidator, DetectionAreaWithInvalidPolygonRefer)  // NOLINT for gtest
{
  load_target_map("area/detection_area_with_invalid_polygon_refer.osm");

  lanelet::autoware::validation::DetectionAreaValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::Id expected_reg_elem_id = 2102;
  const lanelet::Id expected_polygon_id = 2101;

  const auto expected_issue_1 =
    construct_issue_from_code(issue_code(test_target_, 1), expected_polygon_id);
  const auto expected_issue_3 =
    construct_issue_from_code(issue_code(test_target_, 3), expected_reg_elem_id);
  const auto expected_issues = {expected_issue_1, expected_issue_3};

  EXPECT_EQ(issues.size(), 2);
  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestDetectionAreaValidator, DetectionAreaWithNonRoadRefer)  // NOLINT for gtest
{
  load_target_map("area/detection_area_with_non_road_refer.osm");

  lanelet::autoware::validation::DetectionAreaValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  const lanelet::Id expected_reg_elem_id = 2102;
  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 6), expected_reg_elem_id);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
  EXPECT_EQ(issues[0].id, expected_reg_elem_id);
}
