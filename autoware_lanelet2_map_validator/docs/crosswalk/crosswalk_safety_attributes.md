# crosswalk_safety_attributes

## Validator name

mapping.crosswalk.safety_attributes

## Feature

This validator checks that crosswalk lanelets have valid safety attributes for deceleration. When safety attributes are present on a crosswalk, they must contain valid positive numerical values to ensure proper vehicle deceleration behavior.

The validator validates two optional attributes on crosswalk lanelets:

- `safety_slow_down_speed`: The recommended speed at which vehicles should decelerate for the crosswalk
- `safety_slow_down_distance`: The distance before the crosswalk where vehicles should begin decelerating

Both attributes, when present, must be positive numerical values.

| Issue Code                     | Message                                                                                              | Severity | Primitive | Description                                                                       | Approach                                                    |
| ------------------------------ | ---------------------------------------------------------------------------------------------------- | -------- | --------- | --------------------------------------------------------------------------------- | ----------------------------------------------------------- |
| Crosswalk.SafetyAttributes-001 | The safety_slow_down_speed attribute value '{attribute_value}' is not a positive numerical value.    | Error    | lanelet   | The safety_slow_down_speed attribute contains an invalid or non-positive value    | Ensure safety_slow_down_speed is a positive number (> 0)    |
| Crosswalk.SafetyAttributes-002 | The safety_slow_down_distance attribute value '{attribute_value}' is not a positive numerical value. | Error    | lanelet   | The safety_slow_down_distance attribute contains an invalid or non-positive value | Ensure safety_slow_down_distance is a positive number (> 0) |

## Parameters

None.

## Related source codes

- crosswalk_safety_attributes.cpp
- crosswalk_safety_attributes.hpp
