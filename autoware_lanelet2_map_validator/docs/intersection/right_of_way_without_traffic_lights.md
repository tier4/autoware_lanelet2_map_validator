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

| Issue Code                                      | Message                                                                                                                                                                                                 | Severity | Primitive          | Description                                                                                                               | Approach                                                                                                                             |
| ----------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------- | ------------------ | ------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------ |
| Intersection.RightOfWayWithoutTrafficLights-001 | The right_of_way regulatory element should have exactly one right_of_way role.                                                                                                                          | Error    | RegulatoryElement  | Right-of-way regulatory elements must have exactly one lanelet assigned to the RightOfWay role to clearly define priority | Check that right_of_way_lanelets.size() == 1                                                                                         |
| Intersection.RightOfWayWithoutTrafficLights-002 | This regulatory element doesn't set the referrer lanelet as the right_of_way role.                                                                                                                      | Error    | RegulatoryElement  | The lanelet referencing the regulatory element must be assigned to the RightOfWay role                                    | Verify lanelet.id() matches one of the RightOfWay role lanelets                                                                      |
| Intersection.RightOfWayWithoutTrafficLights-003 | Conflicting lanelet {conflicting_lanelet_id} is not set as yield role.                                                                                                                                  | Error    | RegulatoryElement  | Lanelets that conflict with higher-priority traffic must have yield relationships defined                                 | Check that all required conflicting lanelets are in Yield role                                                                       |
| Intersection.RightOfWayWithoutTrafficLights-004 | Unnecessary lanelets {unnecessary_yield_to} are set as yield lanes. (Ignore this if the right_of_lane is a non-priority lane)                                                                           | Warning  | RegulatoryElement  | Yield relationships should only exist for actual conflicting lanelets                                                     | Verify yield relationships correspond to actual conflicts                                                                            |
| Intersection.RightOfWayWithoutTrafficLights-005 | Intersection {intersection_id} doesn't have any right_of_way regulatory element.                                                                                                                        | Error    | Polygon            | Any intersection must have at least one right_of_way regulatory element somewhere.                                        | Check the relations between lanelets inside that intersection_area and define a proper right_of_way regulatory element.              |
| Intersection.RightOfWayWithoutTrafficLights-006 | Lanelet {soft_conflicting_id} is slightly conflicting with the right_of_way lanelet and there is a chance that this should be a yield lanelet. ({percentage} % of the right_of_way lanelet is covered.) | info     | Regulatory Element | The specified lanelet has a slight chance that it should be set as a yield lanelet for this regulatory element.           | Check whether the specified lanelet should be a yield lanelet or not. If it is, add it as a yield lanelet to the regulatory element. |

## Parameters

None.

## Related source codes

- right_of_way_without_traffic_lights.cpp
- right_of_way_without_traffic_lights.hpp
- utils.hpp (has_same_source utility function)
