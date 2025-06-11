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

#include "lanelet2_map_validator/validators/lane/centerline_geometry.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <Eigen/Dense>

#include <boost/geometry.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/Point.h>
#include <lanelet2_core/geometry/Polygon.h>

#include <map>
#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<CenterlineGeometryValidator> reg;
}

lanelet::validation::Issues CenterlineGeometryValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_centerline_geometry(map));

  return issues;
}

lanelet::validation::Issues CenterlineGeometryValidator::check_centerline_geometry(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & lane : map.laneletLayer) {
    if (!lane.hasCustomCenterline()) {
      continue;
    }

    const lanelet::ConstLineString3d centerline3d = lane.centerline3d();

    const lanelet::BasicPolygon2d lane_polygon2d = lane.polygon2d().basicPolygon();
    const lanelet::BasicPolygon3d lane_polygon3d = lane.polygon3d().basicPolygon();

    lanelet::ConstPoints3d sticking_out_points;
    for (const lanelet::ConstPoint3d & point : centerline3d) {
      // skip edge points
      if (point == centerline3d.front() || point == centerline3d.back()) {
        continue;
      }
      if (boost::geometry::distance(point.basicPoint2d(), lane_polygon2d) > planar_threshold_) {
        sticking_out_points.push_back(point);
      }
    }
    if (!sticking_out_points.empty()) {
      std::map<std::string, std::string> point_ids_map;
      point_ids_map["point_ids"] = primitives_to_ids_string(sticking_out_points);
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 1), centerline3d.id(), point_ids_map));
    }

    // quit validation if this is 2D mode
    if (dimension_mode_ == twoD) {
      continue;
    }

    // estimate the lanelet plane
    Eigen::Vector3d centroid(0, 0, 0);
    for (const auto & point : lane_polygon3d) {
      centroid += point;
    }
    centroid /= lane_polygon3d.size();

    Eigen::MatrixXd centered(lane_polygon3d.size(), 3);
    for (size_t i = 0; i < lane_polygon3d.size(); ++i) {
      centered.row(i) = lane_polygon3d[i] - centroid;
    }

    const Eigen::Matrix3d cov = centered.transpose() * centered;

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
    if (solver.info() != Eigen::Success) {
      throw std::runtime_error("Eigen decomposition failed in centerline_geometry");
    }

    Eigen::Vector3d normal = solver.eigenvectors().col(0).normalized();

    // validate the point height based from the estimated lanelet plane
    lanelet::ConstPoints3d distant_points;
    for (const lanelet::ConstPoint3d & point : centerline3d) {
      if (std::abs((point.basicPoint() - centroid).dot(normal)) > height_threshold_) {
        distant_points.push_back(point);
      }
    }
    if (!distant_points.empty()) {
      std::map<std::string, std::string> point_ids_map;
      point_ids_map["point_ids"] = primitives_to_ids_string(distant_points);
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 2), centerline3d.id(), point_ids_map));
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
