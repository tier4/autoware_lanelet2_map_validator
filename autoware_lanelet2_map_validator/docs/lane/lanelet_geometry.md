# lanelet_geometry

## Validator name

mapping.lane.lanelet_geometry

## Feature

This validator checks the geometric properties of lanelets. It ensures that the left bound and right bound of each lanelet do not share any points, which is a fundamental requirement for lanelet geometry validity.

| Issue Code               | Message                                                                                                               | Severity | Primitive | Description                                                                                                                                                                                                          | Approach                                                                                                                                                  |
| ------------------------ | --------------------------------------------------------------------------------------------------------------------- | -------- | --------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Lane.LaneletGeometry-001 | This lanelet has shared points between left and right bounds.                                                         | Error    | lanelet   | The left and right bounds of a lanelet should not share any points. Shared points indicate a degenerate or malformed lanelet geometry.                                                                               | Verify that the left and right bounds are defined with distinct points and do not intersect at any vertices.                                              |
| Lane.LaneletGeometry-002 | This lanelet is detected as not smooth by Isolation Forest detector (score: {anomaly_score}, threshold: {threshold}). | Warning  | lanelet   | The lanelet geometry appears to have irregular or non-smooth characteristics based on curvature and angle analysis. This may indicate mapping errors, abrupt changes in road geometry, or other geometric anomalies. | Review the lanelet geometry for smoothness issues, check for mapping artifacts, or verify if the geometry accurately represents the real road conditions. |

## Parameters

| Parameter Name               | Type   | Default Value | Description                                                                                                                |
| ---------------------------- | ------ | ------------- | -------------------------------------------------------------------------------------------------------------------------- |
| `isolation_forest_threshold` | double | 0.62          | Threshold for the Isolation Forest anomaly detection. Lanelets with scores above this threshold are flagged as non-smooth. |

## Isolation Forest Anomaly Detection

### Overview

The Isolation Forest algorithm is used to detect lanelets with irregular or non-smooth geometry. It analyzes geometric features extracted from each lanelet and identifies outliers that deviate from typical lanelet patterns.

### Feature Calculation

The validator extracts four geometric features from each lanelet:

1. **`log_norm_energy_L`** - Logarithm of normalized curvature energy of the left bound
2. **`log_norm_energy_R`** - Logarithm of normalized curvature energy of the right bound
3. **`mean_angle`** - Average of squared turning angles across both bounds
4. **`angle_diff`** - Average entrance-exit angle difference for both bounds

#### Detailed Feature Calculations

**Normalized Curvature Energy (`log_norm_energy_L/R`)**:

- For each segment in the bound, calculate the turning angle θ between consecutive line segments
- Compute curvature κ = θ / average_segment_length
- Sum up κ² × segment_length for all segments
- Normalize by total path length: `energy = Σ(κ² × length) / total_length`
- Apply logarithm with small epsilon: `log_norm_energy = log(energy + 1e-8)`

**Mean Angle (`mean_angle`)**:

- Calculate squared turning angles (θ²) at each vertex of both left and right bounds
- Average across all vertices: `mean_angle = Σ(θ²) / number_of_vertices`

**Angle Difference (`angle_diff`)**:

- Calculate entrance direction vector (first two points)
- Calculate exit direction vector (last two points)
- Compute angle between entrance and exit vectors using dot product
- Average the angle differences from left and right bounds

### Model Training

The Isolation Forest model (`isolation_forest_model.json`) is pre-trained on a dataset of typical lanelet geometries. To retrain the model with additional map data:

#### 1. Data Collection

```python
# Extract features from your map dataset
import numpy as np
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler

# Collect features from all lanelets in your training maps
features = []  # List of [log_norm_energy_L, log_norm_energy_R, mean_angle, angle_diff]

# Load your training data
X = np.array(features)
```

#### 2. Model Training

```python
# Standardize features
scaler = StandardScaler()
X_scaled = scaler.fit_transform(X)

# Train Isolation Forest
isolation_forest = IsolationForest(
    n_estimators=100,
    contamination=0.1,  # Expected fraction of outliers
    random_state=42
)
isolation_forest.fit(X_scaled)
```

#### 3. Model Export

```python
import json

# Export model parameters to JSON format
model_data = {
    "psi": len(X),  # Training sample size
    "feature_names": ["log_norm_energy_L", "log_norm_energy_R", "mean_angle", "angle_diff"],
    "scaler_mean": scaler.mean_.tolist(),
    "scaler_scale": scaler.scale_.tolist(),
    "trees": []  # Export decision trees (implementation specific)
}

# Save to iforest_model.json
with open("isolation_forest_model.json", "w") as f:
    json.dump(model_data, f, indent=2)
```

#### 4. Model Validation

- Test the model on known good and problematic lanelets
- Typical threshold values range from 0.5 to 0.8 depending on desired sensitivity

### Usage Notes

- Model files should be placed in the `config/` directory of the package

## Related source codes

- lanelet_geometry.cpp
- lanelet_geometry.hpp
