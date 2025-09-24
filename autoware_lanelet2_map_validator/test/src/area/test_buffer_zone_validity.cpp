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

#include "lanelet2_map_validator/validators/area/buffer_zone_validity.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestBufferZoneValidity : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::BufferZoneValidity::name());
};

TEST_F(TestBufferZoneValidity, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::BufferZoneValidity checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBufferZoneValidity, ValidBufferZone)  // NOLINT for gtest
{
  load_target_map("area/buffer_zone_validity.osm");

  lanelet::autoware::validation::BufferZoneValidity checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBufferZoneValidity, UnsharedPointsIssue)  // NOLINT for gtest
{
  load_target_map("area/buffer_zone_validity_with_point_not_shared.osm");

  lanelet::autoware::validation::BufferZoneValidity checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  const lanelet::Id expected_polygon_id = 20005;
  std::map<std::string, std::string> point_ids_map;
  point_ids_map["point_ids"] = "20008";

  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 1), expected_polygon_id, point_ids_map);
  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
  EXPECT_EQ(issues[0].id, expected_polygon_id);
}

TEST_F(TestBufferZoneValidity, InvalidGeometryIssue)  // NOLINT for gtest
{
  load_target_map("area/buffer_zone_validity_with_non_valid_geometry.osm");

  lanelet::autoware::validation::BufferZoneValidity checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  const lanelet::Id expected_polygon_id = 20005;
  std::map<std::string, std::string> reason_map;
  reason_map["boost_geometry_message"] = "Geometry has invalid self-intersections";

  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 2), expected_polygon_id, reason_map);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
  EXPECT_EQ(issues[0].id, expected_polygon_id);
}
