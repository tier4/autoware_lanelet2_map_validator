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

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <nlohmann/json.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{

class ParameterLoadingTest : public MapValidationTester
{
protected:
};

TEST_F(ParameterLoadingTest, CheckDefaultParamsFileHasValidValidators)
{
  const auto params = ValidatorConfigStore::parameters();
  load_target_map("sample_map.osm");  // Needed to provoke validator registration

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks("");  // cspell:disable-line

  const std::string package_share_directory =
    ament_index_cpp::get_package_share_directory("autoware_lanelet2_map_validator");
  const std::string full_path = package_share_directory + "/config/params.yaml";
  YAML::Node root = YAML::LoadFile(full_path);

  for (const auto & child : root) {
    std::string child_name = child.first.as<std::string>();
    EXPECT_TRUE(std::find(validators.begin(), validators.end(), child_name) != validators.end());
  }
}

}  // namespace lanelet::autoware::validation
