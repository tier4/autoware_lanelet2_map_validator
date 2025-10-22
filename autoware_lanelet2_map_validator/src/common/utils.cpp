// Copyright 2024-2025 TIER IV, Inc.
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

#include "lanelet2_map_validator/utils.hpp"

#include "lanelet2_map_validator/config_store.hpp"

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/intersection.hpp>

#include <fmt/args.h>
#include <fmt/core.h>
#include <lanelet2_core/geometry/Polygon.h>

#include <map>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
double polygon_overlap_ratio(
  lanelet::BasicPolygon2d & base_polygon, lanelet::BasicPolygon2d & another_polygon)
{
  boost::geometry::correct(base_polygon);
  boost::geometry::correct(another_polygon);

  std::vector<lanelet::BasicPolygon2d> overlaps;
  boost::geometry::intersection(base_polygon, another_polygon, overlaps);
  double overlapping_area = 0.0;
  for (const auto & portion : overlaps) {
    overlapping_area += boost::geometry::area(portion);
  }

  return overlapping_area / boost::geometry::area(base_polygon);
}
}  // namespace lanelet::autoware::validation

std::string snake_to_upper_camel(const std::string & snake_case)
{
  std::string camel_case;
  bool capitalize_next = true;

  for (char ch : snake_case) {
    if (ch == '_') {
      capitalize_next = true;
    } else if (ch == '.') {
      camel_case += ch;
      capitalize_next = true;
    } else {
      camel_case += capitalize_next ? std::toupper(ch) : ch;
      capitalize_next = false;
    }
  }
  return camel_case;
}

std::string issue_code(const std::string & name, const int number)
{
  if (number < 0 || number > 999) {
    throw std::out_of_range("Number for issue code must be between 0 and 999 inclusive.");
  }

  // Set three digits number string
  std::string id_num = std::to_string(number);
  while (id_num.length() < 3) {
    id_num = "0" + id_num;
  }

  // Remove the first word from name
  size_t pos = name.find('.');
  std::string name_without_prefix = name.substr(pos + 1);

  return snake_to_upper_camel(name_without_prefix) + '-' + id_num;
}

std::string append_issue_code_prefix(
  const std::string & name, const int number, const std::string & message)
{
  return "[" + issue_code(name, number) + "] " + message;
}

lanelet::validation::Issue construct_issue_from_code(
  const std::string & issue_code, const lanelet::Id primitive_id,
  const std::map<std::string, std::string> & substitutions)
{
  lanelet::validation::Issue result;
  const nlohmann::json issues_info =
    lanelet::autoware::validation::ValidatorConfigStore::issues_info()[issue_code];
  std::string severity_str = issues_info["severity"].get<std::string>();
  std::string primitive_str = issues_info["primitive"].get<std::string>();

  if (severity_str == "Error") {
    result.severity = lanelet::validation::Severity::Error;
  } else if (severity_str == "Warning") {
    result.severity = lanelet::validation::Severity::Warning;
  } else if (severity_str == "info") {
    result.severity = lanelet::validation::Severity::Info;
  } else {
    throw std::invalid_argument("Invalid severity defined in the issues info!!");
  }

  if (primitive_str == "point") {
    result.primitive = lanelet::validation::Primitive::Point;
  } else if (primitive_str == "linestring") {
    result.primitive = lanelet::validation::Primitive::LineString;
  } else if (primitive_str == "polygon") {
    result.primitive = lanelet::validation::Primitive::Polygon;
  } else if (primitive_str == "lanelet") {
    result.primitive = lanelet::validation::Primitive::Lanelet;
  } else if (primitive_str == "area") {
    result.primitive = lanelet::validation::Primitive::Area;
  } else if (primitive_str == "regulatory element") {
    result.primitive = lanelet::validation::Primitive::RegulatoryElement;
  } else if (primitive_str == "primitive") {
    result.primitive = lanelet::validation::Primitive::Primitive;
  } else {
    throw std::invalid_argument("Invalid primitive defined in the issues info!!");
  }

  result.id = primitive_id;

  fmt::dynamic_format_arg_store<fmt::format_context> arg_store;
  for (const auto & [key, value] : substitutions) {
    arg_store.push_back(fmt::arg(key.c_str(), value));
  }
  const std::string language = lanelet::autoware::validation::ValidatorConfigStore::language();
  const std::string main_message =
    fmt::vformat(issues_info["message"][language].get<std::string>(), arg_store);
  result.message = "[" + issue_code + "] " + main_message;

  return result;
}
