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

#ifndef LANELET2_MAP_VALIDATOR__CONFIG_STORE_HPP_
#define LANELET2_MAP_VALIDATOR__CONFIG_STORE_HPP_

#include <nlohmann/json.hpp>

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <stdexcept>
#include <string>

namespace lanelet::autoware::validation
{

class ValidatorConfigStore
{
public:
  static void initialize(
    const std::string & params_yaml_file, const std::string & issues_info_json_file)
  {
    yaml_ = YAML::LoadFile(params_yaml_file);

    std::ifstream json_ifs(issues_info_json_file);
    if (!json_ifs.is_open()) {
      throw std::runtime_error("Failed to open JSON file: " + issues_info_json_file);
    }
    json_ifs >> json_;
  }

  static const YAML::Node & parameters() { return yaml_; }
  static const nlohmann::json & issues_info() { return json_; }

private:
  static inline YAML::Node yaml_;
  static inline nlohmann::json json_;
};

}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__CONFIG_STORE_HPP_
