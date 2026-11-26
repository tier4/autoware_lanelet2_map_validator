// Copyright 2026 TIER IV, Inc.
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

#include "lanelet2_map_validator/map_loader.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/primitives/GPSPoint.h>

#include <set>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{

class MgrsGridTest : public ::testing::Test
{
protected:
};

TEST_F(MgrsGridTest, EmptyPoints)  // NOLINT for gtest
{
  const std::vector<lanelet::GPSPoint> points{};

  EXPECT_TRUE(get_mgrs_grids(points).empty());
  EXPECT_TRUE(is_in_same_mgrs_grid(points));
}

TEST_F(MgrsGridTest, PointsInSameGrid)  // NOLINT for gtest
{
  const std::vector<lanelet::GPSPoint> points{{35.903, 139.9336}, {35.9, 139.9}};

  EXPECT_EQ(get_mgrs_grids(points), (std::set<std::string>{"54SVE"}));
  EXPECT_TRUE(is_in_same_mgrs_grid(points));
}

TEST_F(MgrsGridTest, AcrossHundredKilometerGridBoundary)  // NOLINT for gtest
{
  const std::vector<lanelet::GPSPoint> points{{35.9, 139.88}, {35.9, 139.9}};

  EXPECT_EQ(get_mgrs_grids(points), (std::set<std::string>{"54SUE", "54SVE"}));
  EXPECT_FALSE(is_in_same_mgrs_grid(points));
}

TEST_F(MgrsGridTest, AcrossUtmZoneBoundary)  // NOLINT for gtest
{
  const std::vector<lanelet::GPSPoint> points{{34.77, 137.99}, {34.77, 138.01}};

  EXPECT_EQ(get_mgrs_grids(points), (std::set<std::string>{"53SQU", "54STD"}));
  EXPECT_FALSE(is_in_same_mgrs_grid(points));
}

TEST_F(MgrsGridTest, AcrossLatitudeBandBoundary)  // NOLINT for gtest
{
  const std::vector<lanelet::GPSPoint> points{{39.99, 140.5}, {40.01, 140.5}};

  EXPECT_EQ(get_mgrs_grids(points), (std::set<std::string>{"54SVK", "54TVK"}));
  EXPECT_FALSE(is_in_same_mgrs_grid(points));
}

TEST_F(MgrsGridTest, IgnoreInvalidPoint)  // NOLINT for gtest
{
  const std::vector<lanelet::GPSPoint> points{{35.903, 139.9336}, {100.0, 140.0}};

  EXPECT_EQ(get_mgrs_grids(points), (std::set<std::string>{"54SVE"}));
  EXPECT_TRUE(is_in_same_mgrs_grid(points));
}

}  // namespace lanelet::autoware::validation
