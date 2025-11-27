// Copyright 2025 TIER IV, Inc.
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

#include "lanelet2_map_validator/validators/lane/lanelet_geometry.hpp"

#include "lanelet2_map_validator/utils.hpp"
#include <set>
#include <cmath>
#include <algorithm>
#include <map>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>

using nlohmann::json;

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<LaneletGeometryValidator> reg;

struct AngleLength {
  double theta = 0.0;
  double seg_length_before = 0.0;
  double seg_length_after = 0.0;
};

struct LaneletStats {
  double mean_angle = 0.0;
  double angle_diff = 0.0;
  double log_nenergy_L = 0.0;
  double log_nenergy_R = 0.0;
};


static std::vector<AngleLength> computeAnglesLengths(const lanelet::ConstLineString3d & line)
{
  std::vector<AngleLength> result;
  for (size_t i = 1; i + 1 < line.size(); ++i) {
    auto p0 = line[i - 1].basicPoint();
    auto p1 = line[i].basicPoint();
    auto p2 = line[i + 1].basicPoint();

    double dx1 = p1.x() - p0.x(), dy1 = p1.y() - p0.y();
    double dx2 = p2.x() - p1.x(), dy2 = p2.y() - p1.y();

    double norm1 = std::hypot(dx1, dy1);
    double norm2 = std::hypot(dx2, dy2);

    if (norm1 < 1e-9 || norm2 < 1e-9) continue;

    double v1x = dx1 / norm1, v1y = dy1 / norm1;
    double v2x = dx2 / norm2, v2y = dy2 / norm2;

    double dot = v1x * v2x + v1y * v2y;
    double theta = std::acos(std::max(-1.0, std::min(1.0, dot)));

    result.push_back({theta, norm1, norm2});
  }
  return result;
}

static double computeEntranceExitAngleDiff(const lanelet::ConstLineString3d & line)
{
  if (line.size() < 3) {
    return 0.0;
  }

  auto p0_entrance = line[0].basicPoint();
  auto p1_entrance = line[1].basicPoint();
  double dx_entrance = p1_entrance.x() - p0_entrance.x();
  double dy_entrance = p1_entrance.y() - p0_entrance.y();
  double norm_entrance = std::hypot(dx_entrance, dy_entrance);

  auto p0_exit = line[line.size() - 2].basicPoint();
  auto p1_exit = line[line.size() - 1].basicPoint();
  double dx_exit = p1_exit.x() - p0_exit.x();
  double dy_exit = p1_exit.y() - p0_exit.y();
  double norm_exit = std::hypot(dx_exit, dy_exit);

  if (norm_entrance < 1e-9 || norm_exit < 1e-9) {
    return 0.0;
  }

  double v_ent_x = dx_entrance / norm_entrance, v_ent_y = dy_entrance / norm_entrance;
  double v_ext_x = dx_exit / norm_exit, v_ext_y = dy_exit / norm_exit;

  double dot = v_ent_x * v_ext_x + v_ent_y * v_ext_y;
  dot = std::max(-1.0, std::min(1.0, dot));

  double angle_diff = std::acos(dot);
  if (std::isnan(angle_diff)) {
    return 0.0;
  }

  return angle_diff;
}

static double computeNormalizedCurvatureEnergy(const lanelet::ConstLineString3d & bound)
{
  auto angle_lengths = computeAnglesLengths(bound);

  if (angle_lengths.empty()) {
    return 0.0;
  }

  double sum_bending = 0.0;
  double total_len = 0.0;

  for (const auto & al : angle_lengths) {
    double avg_len = (al.seg_length_before + al.seg_length_after) / 2.0;
    if (avg_len < 1e-9) continue;

    double kappa = al.theta / avg_len;
    sum_bending += kappa * kappa * avg_len;
    total_len += avg_len;
  }

  if (total_len < 1e-9) {
    return 0.0;
  }

  return sum_bending / total_len;
}

static LaneletStats computeLaneletStats(const lanelet::ConstLanelet & lanelet)
{
  LaneletStats stats;

  const auto & left_bound = lanelet.leftBound();
  const auto & right_bound = lanelet.rightBound();

  double angle_diff_L = computeEntranceExitAngleDiff(left_bound);
  double angle_diff_R = computeEntranceExitAngleDiff(right_bound);
  stats.angle_diff = (angle_diff_L + angle_diff_R) / 2.0;
  {
    auto angle_lengths_L = computeAnglesLengths(left_bound);
    auto angle_lengths_R = computeAnglesLengths(right_bound);
    
    double sum_theta2_L = 0.0;
    for (const auto & al : angle_lengths_L) {
      sum_theta2_L += al.theta * al.theta;
    }
    double mean_angle_L = !angle_lengths_L.empty() 
      ? sum_theta2_L / static_cast<double>(angle_lengths_L.size()) 
      : 0.0;
    
    double sum_theta2_R = 0.0;
    for (const auto & al : angle_lengths_R) {
      sum_theta2_R += al.theta * al.theta;
    }
    double mean_angle_R = !angle_lengths_R.empty() 
      ? sum_theta2_R / static_cast<double>(angle_lengths_R.size()) 
      : 0.0;
    
    stats.mean_angle = (mean_angle_L + mean_angle_R) / 2.0;
  }

  double energy_L = computeNormalizedCurvatureEnergy(left_bound);
  double energy_R = computeNormalizedCurvatureEnergy(right_bound);

  const double eps = 1e-8;
  stats.log_nenergy_L = std::log(energy_L + eps);
  stats.log_nenergy_R = std::log(energy_R + eps);

  return stats;
}

static std::vector<double> buildFeatures(const LaneletStats & stats)
{
  return {
    stats.log_nenergy_L,
    stats.log_nenergy_R,
    stats.mean_angle,
    stats.angle_diff
  };
}

}  // namespace

lanelet::validation::Issues LaneletGeometryValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_lanelet_geometry(map));
  lanelet::autoware::validation::appendIssues(issues, check_lanelet_anomaly_if(map, 
  "/home/radityagiovanni/autoware_3/autoware/src/autoware_lanelet2_map_validator/autoware_lanelet2_map_validator/config/iforest_model.json", 
  isolation_forest_threshold_));

  return issues;
}

lanelet::validation::Issues LaneletGeometryValidator::check_lanelet_geometry(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & lanelet : map.laneletLayer) {
    const auto & left_bound = lanelet.leftBound();
    const auto & right_bound = lanelet.rightBound();

    std::set<lanelet::Id> left_point_ids;
    std::set<lanelet::Id> right_point_ids;

    for (const auto & point : left_bound) {
      left_point_ids.insert(point.id());
    }

    for (const auto & point : right_bound) {
      right_point_ids.insert(point.id());
    }

    std::vector<lanelet::Id> shared_point_ids;
    for (const auto & left_id : left_point_ids) {
      if (right_point_ids.count(left_id) > 0) {
        shared_point_ids.push_back(left_id);
      }
    }

    // Issue-001: lanelet has shared points between left and right bounds
    if (!shared_point_ids.empty()) {
      std::map<std::string, std::string> substitution_map;

      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 1), lanelet.id(), substitution_map));
    }
  }

  return issues;
}

lanelet::validation::Issues LaneletGeometryValidator::check_lanelet_anomaly_if(
  const lanelet::LaneletMap & map,
  const std::string & model_path,
  double anomaly_threshold)
{
  lanelet::validation::Issues issues;
  if (!if_model_loaded_) {
    if_model_ = std::make_unique<IsolationForestPredictor>();
    if (!if_model_->loadModel(model_path)) {
      return issues;
    }
    if_model_loaded_ = true;
  }

  for (const auto & lanelet : map.laneletLayer) {
    LaneletStats stats = computeLaneletStats(lanelet);
    std::vector<double> x = buildFeatures(stats);
    double score_sk  = if_model_->scoreSamples(x);
    bool is_outlier = (score_sk > anomaly_threshold);  // Use sklearn's prediction directly

    if (is_outlier) {
      std::map<std::string, std::string> substitution_map;
      substitution_map["anomaly_score"]   = std::to_string(score_sk);
      substitution_map["threshold"]       = std::to_string(anomaly_threshold);

      issues.emplace_back(
        construct_issue_from_code(
          issue_code(this->name(), 2), lanelet.id(), substitution_map));
    }
  }

  return issues;
}

}  // namespace lanelet::autoware::validation

// ---- IsolationForestPredictor Implementation ----

bool IsolationForestPredictor::loadModel(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    json j;
    in >> j;

    n_features_    = j.at("n_features").get<int>();
    psi_           = j.at("psi").get<int>();
    offset_        = j.at("offset").get<double>();
    feature_names_ = j.at("feature_names").get<std::vector<std::string>>();
    
    if (j.contains("scaler_mean") && j.contains("scaler_scale")) {
        scaler_mean_  = j.at("scaler_mean").get<std::vector<double>>();
        scaler_scale_ = j.at("scaler_scale").get<std::vector<double>>();
    }

    trees_.clear();
    for (const auto& tjson : j.at("trees")) {
        IFTree t;
        for (const auto& njson : tjson.at("nodes")) {
            IFNode n;
            n.is_leaf   = njson.at("is_leaf").get<bool>();
            n.feature   = njson.at("feature").get<int>();
            n.threshold = njson.at("threshold").get<double>();
            n.left      = njson.at("left").get<int>();
            n.right     = njson.at("right").get<int>();
            n.n_samples = njson.at("n_samples").get<int>();
            t.nodes.push_back(n);
        }
        trees_.push_back(std::move(t));
    }
    return true;
}

double IsolationForestPredictor::anomalyScorePaper(const std::vector<double>& x) const {
    const double h = averagePathLength(x);
    const double cn = cFactor(psi_);
    if (cn <= 0.0) {
        return 1.0;
    }
    return std::pow(2.0, -h / cn);
}

double IsolationForestPredictor::scoreSamples(const std::vector<double>& x) const {
    std::vector<double> x_scaled = x;
    if (!scaler_mean_.empty() && !scaler_scale_.empty()) {
        for (size_t i = 0; i < x.size() && i < scaler_mean_.size(); ++i) {
            if (scaler_scale_[i] > 1e-10) {
                x_scaled[i] = (x[i] - scaler_mean_[i]) / scaler_scale_[i];
            }
        }
    }
    
    return anomalyScorePaper(x_scaled);
}

double IsolationForestPredictor::decisionFunction(const std::vector<double>& x) const {
    return scoreSamples(x) - offset_;
}

double IsolationForestPredictor::averagePathLength(const std::vector<double>& x) const {
    double total_path = 0.0;
    for (const auto& tree : trees_) {
        total_path += pathLength(tree, x, 0, 0.0);
    }
    return total_path / static_cast<double>(trees_.size());
}

double IsolationForestPredictor::pathLength(const IFTree& tree,
                      const std::vector<double>& x,
                      int node_idx,
                      double depth) const {
    const IFNode& node = tree.nodes[node_idx];

    if (node.is_leaf || node.left < 0 || node.right < 0) {
        int n_leaf = std::max(1, node.n_samples);
        return depth + cFactor(n_leaf);
    }

    const double next_depth = depth + 1.0;
    if (x[node.feature] < node.threshold) {
        return pathLength(tree, x, node.left, next_depth);
    } else {
        return pathLength(tree, x, node.right, next_depth);
    }
}

double IsolationForestPredictor::cFactor(int n) {
    if (n <= 1) {
        return 0.0;
    }
    if (n == 2) {
        return 1.0;
    }
    const double nn = static_cast<double>(n);
    const double H  = std::log(nn - 1.0) + 0.5772156649;
    return 2.0 * H - 2.0 * (nn - 1.0) / nn;
}