// Copyright 2024 Autoware Foundation
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

#ifndef MAP_VALIDATION_TESTER_HPP_
#define MAP_VALIDATION_TESTER_HPP_

#include "lanelet2_map_validator/config_store.hpp"
#include "lanelet2_map_validator/map_loader.hpp"

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <autoware_lanelet2_extension/projection/mgrs_projector.hpp>

#include <gtest/gtest.h>
#include <lanelet2_io/Io.h>
#include <lanelet2_validation/Issue.h>

#include <memory>
#include <string>
#include <vector>

class MapValidationTester : public ::testing::Test
{
public:
  MapValidationTester()
  {
    std::string package_share_directory =
      ament_index_cpp::get_package_share_directory("autoware_lanelet2_map_validator");
    std::string parameters_file = package_share_directory + "/config/params.yaml";
    lanelet::autoware::validation::ValidatorConfigStore::initialize(parameters_file);
  }

protected:
  void load_target_map(std::string file_name)
  {
    const auto projector = std::make_unique<lanelet::projection::MGRSProjector>();

    std::string package_share_directory =
      ament_index_cpp::get_package_share_directory("autoware_lanelet2_map_validator");
    const std::string map_file_path = package_share_directory + "/data/map/" + file_name;

    std::tie(map_, loading_issues_) = lanelet::autoware::validation::loadAndValidateMap(
      "mgrs", map_file_path, lanelet::validation::ValidationConfig());

    EXPECT_NE(map_, nullptr);
  }

  bool is_same_issue(
    const lanelet::validation::Issue & issue1, const lanelet::validation::Issue & issue2)
  {
    return issue1.id == issue2.id && issue1.severity == issue2.severity &&
           issue1.primitive == issue2.primitive && issue1.message == issue2.message;
  }

  // They don't have to be in the same order
  bool are_same_issues(
    const lanelet::validation::Issues & issues1, const lanelet::validation::Issues & issues2)
  {
    if (issues1.size() != issues2.size()) {
      return false;
    }

    for (const auto & issue1 : issues1) {
      bool check = std::any_of(
        issues2.begin(), issues2.end(), [this, &issue1](const lanelet::validation::Issue & target) {
          return is_same_issue(issue1, target);
        });
      if (!check) {
        return false;
      }
    }

    return true;
  }

  lanelet::LaneletMapPtr map_{nullptr};
  std::vector<lanelet::validation::DetectedIssues> loading_issues_;
};

#endif  // MAP_VALIDATION_TESTER_HPP_
