# right_of_way_for_virtual_traffic_lights

## Validator name

mapping.intersection.right_of_way_for_virtual_traffic_lights

## Feature

This validator ensures that lanelets with virtual traffic lights have properly configured right-of-way regulatory elements. It checks that:

1. Lanelets with virtual traffic lights must have corresponding right-of-way regulatory elements
2. Each lanelet should have exactly one right-of-way regulatory element
3. The right-of-way regulatory element must have exactly one lanelet assigned to the right_of_way role
4. The lanelet itself must be assigned as the right_of_way role in its regulatory element
5. All conflicting lanelets (as determined by the routing graph) must be assigned as yield roles in the right-of-way regulatory element
6. Only lanelets that actually conflict with the right-of-way lanelet should be assigned as yield roles

This validator uses lanelet2's routing graph to identify conflicting relationships between lanelets and ensures proper right-of-way assignment for intersection management.

The validator outputs the following issues with the corresponding ID of the primitive:

| Issue Code                                         | Message                                                                                                     | Severity | Primitive          | Description                                                                                            | Approach                                                                                      |
| -------------------------------------------------- | ----------------------------------------------------------------------------------------------------------- | -------- | ------------------ | ------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------- |
| Intersection.RightOfWayForVirtualTrafficLights-001 | Lanelet with virtual_traffic_light missing right_of_way reference                                           | Error    | Lanelet            | A lanelet has virtual traffic light regulatory elements but no right-of-way regulatory element         | Add a right_of_way regulatory element to the lanelet with virtual traffic lights              |
| Intersection.RightOfWayForVirtualTrafficLights-002 | Multiple right_of_way regulatory elements in the same lanelet                                               | Error    | Lanelet            | A lanelet has more than one right-of-way regulatory element assigned                                   | Keep only one right_of_way regulatory element per lanelet, merge or remove duplicate elements |
| Intersection.RightOfWayForVirtualTrafficLights-003 | Right_of_way regulatory element should have exactly one right_of_way role                                   | Error    | Regulatory Element | The right-of-way regulatory element doesn't have exactly one lanelet assigned to the right_of_way role | Assign exactly one lanelet to the right_of_way role in the regulatory element                 |
| Intersection.RightOfWayForVirtualTrafficLights-004 | Right_of_way regulatory element doesn't set this lanelet as right_of_way role                               | Error    | Regulatory Element | The lanelet is not assigned as the right_of_way role in its own right-of-way regulatory element        | Set the lanelet itself as the right_of_way role in its regulatory element                     |
| Intersection.RightOfWayForVirtualTrafficLights-005 | Conflicting lanelet (ID: {conflicting_lanelet_id}) not set as yield role in right_of_way regulatory element | Error    | Regulatory Element | A lanelet that conflicts with the right-of-way lanelet is not assigned as yield role                   | Add the conflicting lanelet as a yield role in the right-of-way regulatory element            |
| Intersection.RightOfWayForVirtualTrafficLights-006 | Unnecessary yield relationship to lanelet {unnecessary_yield_to} in right_of_way regulatory element         | Error    | Regulatory Element | A lanelet is assigned as yield role but doesn't actually conflict with the right-of-way lanelet        | Remove the unnecessary yield relationship from the right-of-way regulatory element            |

## Parameters

None.

## Related source codes

- right_of_way_for_virtual_traffic_lights.cpp
- right_of_way_for_virtual_traffic_lights.hpp
