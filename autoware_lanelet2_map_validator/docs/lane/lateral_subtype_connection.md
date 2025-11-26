# lateral_subtype_connection

## Validator name

mapping.lane.lateral_subtype_connection

## Feature

This validator ensures that adjacent lanelets have compatible subtypes for vehicle navigation. It checks that laterally adjacent lanelets (left and right) have subtypes that are suitable for vehicle traffic, preventing incompatible lane adjacencies that could cause routing or navigation issues.

The validator specifically focuses on:

- Checking left and right adjacent lanelets using the routing graph
- Ensuring adjacent lanelets have vehicle-suitable subtypes
- Preventing adjacency between vehicle lanes and non-vehicle-suitable lane types

| Issue Code                        | Message                                                                             | Severity | Primitive | Description                                                                                                                                                             | Approach                                                                                                                                    |
| --------------------------------- | ----------------------------------------------------------------------------------- | -------- | --------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------- |
| Lane.LateralSubtypeConnection-001 | Adjacent lanelet {adjacent_lanelet_id} has incompatible subtype for vehicle traffic | Error    | lanelet   | A lanelet is adjacent to another lanelet with an incompatible subtype. Vehicle-suitable subtypes include "road", "road_shoulder", "pedestrian_lane" and "bicycle_lane". | Verify that adjacent lanelets have compatible subtypes. If one lanelet is not suitable for vehicles, they should not be laterally adjacent. |

## Parameters

None.

## Implementation Details

### Algorithm

1. **Routing Graph Creation**: Creates a routing graph using vehicle traffic rules to identify adjacent relationships
2. **Adjacent Lanelet Detection**: For each lanelet in the map, finds left and right adjacent lanelets using the routing graph
3. **Subtype Validation**: Checks if both the current lanelet and adjacent lanelet have vehicle-suitable subtypes
4. **Duplicate Prevention**: Uses a set of processed pairs to avoid checking the same adjacency relationship twice

### Vehicle-Suitable Subtypes

The validator considers the following subtypes as vehicle-suitable:

- `road` (default when no subtype is specified)
- `road_shoulder`
- `pedestrian_lane`
- `bicycle_lane`

### Error Detection

An error is reported when:

- A lanelet with a vehicle-suitable subtype is adjacent to a lanelet with a non-vehicle-suitable subtype
- The adjacency relationship is determined by the routing graph's `adjacentLeft()` and `adjacentRight()` methods

## Related source codes

- lateral_subtype_connection.cpp
- lateral_subtype_connection.hpp
