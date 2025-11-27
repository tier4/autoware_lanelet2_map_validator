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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__LANELET_GEOMETRY_HPP_
#define LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__LANELET_GEOMETRY_HPP_

#include "lanelet2_map_validator/config_store.hpp"

#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <nlohmann/json.hpp>

using nlohmann::json;

// Isolation Forest structures and predictor
struct IFNode {
    bool   is_leaf;
    int    feature;
    double threshold;
    int    left;
    int    right;
    int    n_samples;
};

struct IFTree {
    std::vector<IFNode> nodes;
};

class IsolationForestPredictor {
public:
    bool loadModel(const std::string& path);
    double scoreSamples(const std::vector<double>& x) const;
    double decisionFunction(const std::vector<double>& x) const;
    int nFeatures() const { return n_features_; }
    const std::vector<std::string>& featureNames() const { return feature_names_; }

private:
    int n_features_ = 0;
    int psi_ = 256;
    double offset_ = 0.0;
    std::vector<std::string> feature_names_;
    std::vector<double> scaler_mean_; 
    std::vector<double> scaler_scale_;
    std::vector<IFTree> trees_;

    double anomalyScorePaper(const std::vector<double>& x) const;
    double averagePathLength(const std::vector<double>& x) const;
    double pathLength(const IFTree& tree,
                      const std::vector<double>& x,
                      int node_idx,
                      double depth) const;
    static double cFactor(int n);
};

namespace lanelet::autoware::validation
{
class LaneletGeometryValidator : public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name() { return "mapping.lane.lanelet_geometry"; }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

  LaneletGeometryValidator()
  {
    const auto parameters = ValidatorConfigStore::parameters()[name()];
    isolation_forest_threshold_ = get_parameter_or<double>(parameters, "isolation_forest_threshold", 0.62);
  }

private:
  lanelet::validation::Issues check_lanelet_geometry(const lanelet::LaneletMap & map);
  lanelet::validation::Issues check_lanelet_anomaly_if(
    const lanelet::LaneletMap & map,
    const std::string & model_path,
    double anomaly_threshold = 0.6);

  double isolation_forest_threshold_;
  mutable std::unique_ptr<IsolationForestPredictor> if_model_ = nullptr;
  mutable bool if_model_loaded_ = false;
};
}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__LANELET_GEOMETRY_HPP_
