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

#include "lanelet2_map_validator/validators/lane/centerline_stick_out.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <boost/geometry.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/Point.h>
#include <lanelet2_core/geometry/Polygon.h>

#include <iostream>
#include <map>
#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<CenterlineStickOutValidator> reg;
}

lanelet::validation::Issues CenterlineStickOutValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_centerline_stick_out(map));

  return issues;
}

lanelet::validation::Issues CenterlineStickOutValidator::check_centerline_stick_out(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & lane : map.laneletLayer) {
    if (!lane.hasCustomCenterline()) {
      continue;
    }

    const lanelet::ConstLineString3d centerline3d = lane.centerline3d();
    std::cout << "Lane " << lane.id() << " has custom centerline " << centerline3d.id()
              << std::endl;

    const lanelet::BasicPolygon2d lane_polygon2d = lane.polygon2d().basicPolygon();
    const lanelet::BasicPolygon3d lane_polygon3d = lane.polygon3d().basicPolygon();

    std::cout << "[";
    for (const auto & point : lane_polygon2d) {
      std::cout << point[0] << ", " << point[1] << ";" << std::endl;
    }
    std::cout << "]" << std::endl;

    lanelet::ConstPoints3d sticking_out_points;
    for (const lanelet::ConstPoint3d & point : centerline3d) {
      std::cout << "Point " << point.id() << std::endl;
      std::cout << "[" << point.x() << ", " << point.y() << "]" << std::endl;
      if (!boost::geometry::covered_by(point.basicPoint2d(), lane_polygon2d)) {
        sticking_out_points.push_back(point);
        std::cout << "    stick out" << std::endl;
      }
    }
    if (!sticking_out_points.empty()) {
      std::map<std::string, std::string> point_ids_map;
      point_ids_map["point_ids"] = primitives_to_ids_string(sticking_out_points);
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 1), centerline3d.id(), point_ids_map));
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
