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

#include "map_validation_tester.hpp"
#include "lanelet2_map_validator/validators/area/missing_regulatory_elements_for_bus_stop_areas.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestMissingRegulatoryElementsForBusStopAreas : public MapValidationTester
{
private:
};

TEST_F(TestMissingRegulatoryElementsForBusStopAreas, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name = lanelet::autoware::validation::MissingRegulatoryElementsForBusStopAreas::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestMissingRegulatoryElementsForBusStopAreas, ValidBusStopArea)  // NOLINT for gtest
{
  load_target_map("area/bus_stop_area.osm");

  lanelet::autoware::validation::MissingRegulatoryElementsForBusStopAreas checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestMissingRegulatoryElementsForBusStopAreas, InvalidRefers)  // NOLINT for gtest
{
  load_target_map("area/bus_stop_area_with_invalid_refers.osm");

  lanelet::autoware::validation::MissingRegulatoryElementsForBusStopAreas checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  EXPECT_EQ(issues[0].id, 2101); 
}

TEST_F(TestMissingRegulatoryElementsForBusStopAreas, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::MissingRegulatoryElementsForBusStopAreas checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
