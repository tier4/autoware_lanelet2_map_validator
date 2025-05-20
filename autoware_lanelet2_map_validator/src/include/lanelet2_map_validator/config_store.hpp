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

#include "lanelet2_map_validator/utils.hpp"

#include <nlohmann/json.hpp>

#include <lanelet2_validation/Issue.h>
#include <yaml-cpp/yaml.h>

#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>

namespace lanelet::autoware::validation
{

class ValidatorConfigStore
{
public:
  static void initialize(
    const std::string & params_yaml_file, const std::string & issues_info_json_file,
    const std::string & language)
  {
    yaml_ = YAML::LoadFile(params_yaml_file);

    std::ifstream json_ifs(issues_info_json_file);
    if (!json_ifs.is_open()) {
      throw std::runtime_error("Failed to open JSON file: " + issues_info_json_file);
    }
    json_ifs >> json_;

    language_ = language;
  }
  static const YAML::Node & parameters() { return yaml_; }
  static const nlohmann::json & issues_info() { return json_; }
  static const std::string & language() { return language_; }

private:
  static inline YAML::Node yaml_;
  static inline nlohmann::json json_;
  static inline std::string language_;
};

template <typename T>
std::optional<T> get_parameter(const YAML::Node parent_node, const std::string & param_name)
{
  if (parent_node[param_name]) {
    try {
      return parent_node[param_name].as<T>();
    } catch (const std::exception & e) {
      std::cerr << "Type mismatch for parameter \"" << param_name << "\": " << e.what()
                << std::endl;
      std::cerr << "nullopt is returned." << std::endl;
    }
  }

  std::cerr << "Couldn't find parameter \"" << param_name << "\". Return nullopt instead."
            << std::endl;
  return std::nullopt;
}

template <typename T>
T get_parameter_or(
  const YAML::Node & parent_node, const std::string & param_name, const T & default_value)
{
  if (parent_node[param_name]) {
    try {
      return parent_node[param_name].as<T>();
    } catch (const std::exception & e) {
      std::cerr << "Type mismatch for parameter \"" << param_name << "\": " << e.what()
                << std::endl;
      std::cerr << "Default value is used." << std::endl;
    }
  }

  std::cerr << "Couldn't find parameter \"" << param_name << "\". Use default value instead."
            << std::endl;
  return default_value;
}

}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__CONFIG_STORE_HPP_
