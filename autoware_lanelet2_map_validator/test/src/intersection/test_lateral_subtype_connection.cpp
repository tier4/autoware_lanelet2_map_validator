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

#include "lanelet2_map_validator/validators/intersection/lateral_subtype_connection.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestLateralSubtypeConnectionValidator : public MapValidationTester
{
private:
};

TEST_F(TestLateralSubtypeConnectionValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::LateralSubtypeConnectionValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestLateralSubtypeConnectionValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::LateralSubtypeConnectionValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestLateralSubtypeConnectionValidator, ValidBorderSharing)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_base.osm");

  lanelet::autoware::validation::LateralSubtypeConnectionValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestLateralSubtypeConnectionValidator, InvalidSubtypeAdjacency)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_with_wrong_subtype.osm");

  lanelet::autoware::validation::LateralSubtypeConnectionValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_GT(issues.size(), 0);
  EXPECT_EQ(issues[0].id, 97) << "Issue should be for lanelet 96";
}
