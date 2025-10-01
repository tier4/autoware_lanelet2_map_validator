# body_height

## Validator name

mapping.traffic_light.body_height

## Feature

This validator checks whether traffic light linestrings have proper height attributes and whether the height values are within acceptable ranges.

| Issue Code                  | Message                                                          | Severity | Primitive  | Description                                                                | Approach                                                                                   |
| --------------------------- | ---------------------------------------------------------------- | -------- | ---------- | -------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------ |
| TrafficLight.BodyHeight-001 | Traffic light linestring is missing required 'height' attribute. | Error    | linestring | The traffic light linestring does not have the required `height` attribute | Add a `height` attribute to the traffic light linestring with an appropriate numeric value |
| TrafficLight.BodyHeight-002 | Traffic light height value is outside the valid range.           | Error    | linestring | The height value is either below `min_height` or above `max_height`        | Adjust the height value to be within the valid range defined by the parameters             |

## Parameters

The validator uses the following parameters defined in `params.yaml`:

- `min_height` (default: 0.2): Minimum acceptable height value for traffic lights
- `max_height` (default: 2.0): Maximum acceptable height value for traffic lights

## Related source codes

- body_height.cpp
- body_height.hpp
