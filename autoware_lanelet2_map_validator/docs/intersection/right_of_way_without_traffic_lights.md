# right_of_way_without_traffic_lights

## Validator name

mapping.intersection.right_of_way_without_traffic_lights

## Feature

This validator ensures proper right-of-way relationships at intersections without traffic lights or virtual traffic lights. It validates that lanelets with turn directions have appropriate right-of-way regulatory elements with correct role assignments and yield relationships to prevent conflicts.
[Vector Map Requirements vm-03-11](https://docs.pilot.auto/en/reference-design/common/map-requirements/vector-map-requirements/category_intersection#vm-03-11-right-of-way-without-signal).

The validator focuses on intersections where traffic flow is managed through explicit right-of-way rules rather than traffic signals, ensuring that:

- Right-of-way regulatory elements are properly structured
- Conflicting lanelets have appropriate yield relationships
- No unnecessary yield relationships exist
- Priority rules are correctly implemented based on turn directions

| Issue Code                                      | Message                                                                                                                              | Severity | Primitive         | Description                                                                                                               | Approach                                                        |
| ----------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------ | -------- | ----------------- | ------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------- |
| Intersection.RightOfWayWithoutTrafficLights-001 | This right-of-way regulatory element has incorrect role count. Expected exactly one right_of_way role.                               | Error    | RegulatoryElement | Right-of-way regulatory elements must have exactly one lanelet assigned to the RightOfWay role to clearly define priority | Check that right_of_way_lanelets.size() == 1                    |
| Intersection.RightOfWayWithoutTrafficLights-002 | This right-of-way regulatory element doesn't set this lanelet as right_of_way role.                                                  | Error    | RegulatoryElement | The lanelet referencing the regulatory element must be assigned to the RightOfWay role                                    | Verify lanelet.id() matches one of the RightOfWay role lanelets |
| Intersection.RightOfWayWithoutTrafficLights-003 | Missing yield relationship to conflicting lanelet {conflicting_lanelet_id}. Turn direction: {turn_direction}, Lane type: {lane_type} | Error    | RegulatoryElement | Lanelets that conflict with higher-priority traffic must have yield relationships defined                                 | Check that all required conflicting lanelets are in Yield role  |
| Intersection.RightOfWayWithoutTrafficLights-004 | Unnecessary yield relationship to non-conflicting lanelet {unnecessary_yield_to}. Turn direction: {turn_direction}                   | Warning  | RegulatoryElement | Yield relationships should only exist for actual conflicting lanelets                                                     | Verify yield relationships correspond to actual conflicts       |

## Parameters

None.

## Related source codes

- right_of_way_without_traffic_lights.cpp
- right_of_way_without_traffic_lights.hpp
- utils.hpp (has_same_source utility function)
