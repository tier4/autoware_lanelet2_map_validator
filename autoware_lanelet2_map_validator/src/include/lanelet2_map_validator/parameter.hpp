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

#ifndef LANELET2_MAP_VALIDATOR__PARAMETER_HPP_
#define LANELET2_MAP_VALIDATOR__PARAMETER_HPP_

#include <ament_index_cpp/get_package_share_directory.hpp>

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace lanelet::autoware::validation
{
class ParameterLoader
{
public:
  explicit ParameterLoader(const std::string & validator_name)
  {
    try {
      const std::string package_share_directory =
        ament_index_cpp::get_package_share_directory("autoware_lanelet2_map_validator");
      const std::string full_path = package_share_directory + "/config/" + filename_;

      YAML::Node root = YAML::LoadFile(full_path);
      if (root[validator_name]) {
        parameters_ = root[validator_name];
      } else {
        std::cerr << "Parameters of " << validator_name << " not defined!!" << std::endl;
      }
    } catch (const std::exception & e) {
      std::cerr << e.what() << std::endl;
      throw std::invalid_argument("Failed to load parameter yaml file!!");
    }
  }

  explicit ParameterLoader(
    const std::string & validator_name, const std::string & fullpath_filename)
  {
    try {
      YAML::Node root = YAML::LoadFile(fullpath_filename);
      if (root[validator_name]) {
        parameters_ = root[validator_name];
      } else {
        std::cerr << "Parameters of " << validator_name << " not defined!!" << std::endl;
      }
    } catch (const std::exception & e) {
      std::cerr << e.what() << std::endl;
      throw std::invalid_argument("Failed to load parameter yaml file!!");
    }
  }

  template <typename T>
  std::optional<T> get_parameter(const std::string & param_name) const
  {
    if (parameters_[param_name]) {
      try {
        return parameters_[param_name].as<T>();
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
  T get_parameter_or(const std::string & param_name, const T & default_value) const
  {
    if (parameters_[param_name]) {
      try {
        return parameters_[param_name].as<T>();
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

private:
  YAML::Node parameters_;
  inline static const std::string filename_ = "params.yaml";
};

}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__PARAMETER_HPP_
