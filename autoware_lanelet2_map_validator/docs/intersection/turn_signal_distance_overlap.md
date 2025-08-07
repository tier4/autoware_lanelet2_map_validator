# turn_signal_distance_overlap

## Validator name

mapping.intersection.turn_signal_distance_overlap

## Feature

This validator checks whether the value of the `turn_signal_distance` in a lanelet is correct or not.
All lanelets that have a `turn_direction` have a `turn_signal_distance` to determine when to start the vehicle's turn signal.
One may have explicitly defined as a tag in the lanelet map and one may have not defined explicitly and use the default value defined in the Autoware system.
However, there is a problem that if there is a lanelet with a `turn_direction = left` previous to another lanelet that has a `turn_direction = right` and they are within `turn_signal_distance` meters, Autoware will not use the turn signals correctly.
This validator will check lanelets that have `turn_direction`s whether they have previous lanelets with the opposite `turn_direction` within the `turn_signal_distance`.
If the `turn_signal_distance` is not set to a turning lanelet, the validator will use the default value set in the parameter file.

| Issue Code                                 | Message                                                                                                                                            | Severity | Primitive | Description                                                                                  | Approach                                                                                                                                                                         |
| ------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------- | -------- | --------- | -------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Intersection.TurnSignalDistanceOverlap-001 | A different lanelet with a "turn_direction" tag is found before turn_signal_distance meters. (previous lanelets with a turn direction: {prev_ids}) | Error    | Lanelet   | There are previous lanelets with `turn_direction` tags within `turn_signal_distance` meters. | Set the `turn_signal_distance` shorter so that it will not reach to the previous turning lanelet. If it is soon before the current lanelet, set the `turn_signal_distance` to 0. |

## Parameters

| Parameter                    | Default Value | Description                                                                                                                |
| ---------------------------- | ------------- | -------------------------------------------------------------------------------------------------------------------------- |
| default_turn_signal_distance | 15.0 [m]      | The default `turn_signal_distance` to lanelets that not have been set explicitly. A non-negative double value is expected. |

## Related source codes

- turn_signal_distance_overlap.cpp
- turn_signal_distance_overlap.hpp
