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

#include "lanelet2_map_validator/validators/lane/centerline_geometry.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestCenterlineGeometryValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::CenterlineGeometryValidator::name());
};

TEST_F(TestCenterlineGeometryValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::CenterlineGeometryValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestCenterlineGeometryValidator, CenterlineEndingTooEarly)  // NOLINT for gtest
{
  load_target_map("lane/centerline_ending_too_early.osm");

  lanelet::autoware::validation::CenterlineGeometryValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> id_map;
  id_map["point_id"] = "125";
  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 1), 129, id_map);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestCenterlineGeometryValidator, CenterlineOverpassing)  // NOLINT for gtest
{
  load_target_map("lane/centerline_overpassing_lanes.osm");

  lanelet::autoware::validation::CenterlineGeometryValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> ids_map;
  ids_map["point_ids"] = "89, 90, 91, 92, 93";
  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 2), 100, ids_map);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

// TEST_F(TestCenterlineGeometryValidator, CenterlineWithWrongHeight)  // NOLINT for gtest
// {
//   load_target_map("lane/centerline_with_strange_height_point.osm");

//   lanelet::autoware::validation::CenterlineGeometryValidator checker;
//   const auto & issues = checker(*map_);

//   std::map<std::string, std::string> ids_map;
//   ids_map["point_ids"] = "120";
//   const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 3), 129,
//   ids_map);

//   EXPECT_EQ(issues.size(), 1);

//   const auto difference = compare_an_issue(expected_issue, issues[0]);
//   EXPECT_TRUE(difference.empty()) << difference;
// }

TEST_F(TestCenterlineGeometryValidator, CorrectCenterline)  // NOLINT for gtest
{
  load_target_map("lane/lanes_with_correct_centerlines.osm");

  lanelet::autoware::validation::CenterlineGeometryValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestCenterlineGeometryValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::CenterlineGeometryValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
