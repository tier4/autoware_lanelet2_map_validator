# right_of_way_with_traffic_lights

## Validator name

mapping.intersection.right_of_way_with_traffic_lights

## Feature

This validator implements the VM-03-10 map requirement for "Right of Way with Signal". It checks whether lanelets that have both a `turn_direction` tag and refer to a `traffic_light` subtype regulatory element are properly configured with `right_of_way` regulatory elements.

The validation subjects are lanelets that satisfy both of the following conditions:

- Have the `turn_direction` tag
- Refer to a `traffic_light` subtype regulatory element

For each validation subject, the validator checks:

1. Whether the lanelet is referring to a `right_of_way` subtype regulatory element
2. Whether that `right_of_way` subtype regulatory element is properly referencing the lanelet in its right_of_way role
3. Whether the proper yield relationships are established based on the following rules:
   - Yield to conflicting lanelets with different signal timing
   - Left-turning vehicles should yield to opposing right-turning lanes
   - Straight lanes have priority over turning lanes
4. Whether there are any missing or unnecessary yield relationships defined

The validator outputs the following issues with the corresponding ID of the primitive.

| Issue Code                                   | Message                                                                                                                        | Severity | Primitive          | Description                                                                                                                                           | Approach                                                                                                  |
| -------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------ | -------- | ------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------- |
| Intersection.RightOfWayWithTrafficLights-001 | "Lanelet with turn_direction '\<TURN_DIRECTION\>' and traffic lights must be referenced by a right_of_way regulatory element." | Error    | Lanelet            | A lanelet that has both a turn_direction tag and references a traffic_light regulatory element must also reference a right_of_way regulatory element. | Add a reference to an appropriate right_of_way regulatory element to ensure proper intersection behavior. |
| Intersection.RightOfWayWithTrafficLights-002 | "More than one right_of_way regulatory element exist in the same lanelet."                                                     | Error    | Lanelet            | A lanelet should only reference a single right_of_way regulatory element to avoid ambiguity.                                                          | Remove redundant right_of_way regulatory elements, keeping only one per lanelet.                          |
| Intersection.RightOfWayWithTrafficLights-003 | "The right_of_way element does not reference the correct lanelet in its right_of_way role."                                    | Error    | Regulatory Element | The right_of_way regulatory element must properly reference the lanelet in its right_of_way role.                                                     | Update the right_of_way regulatory element to properly reference this lanelet in its right_of_way role.   |
| Intersection.RightOfWayWithTrafficLights-004 | "Missing required yield relationship to lanelet \<MISSING_YIELD_TO\> for \<TURN_DIRECTION\> turn at intersection."             | Error    | Regulatory Element | The right_of_way regulatory element is missing a required yield relationship to a conflicting lanelet.                                                | Add the missing yield relationship to ensure proper traffic flow and safety at the intersection.          |
| Intersection.RightOfWayWithTrafficLights-005 | "Unnecessary yield relationship to lanelet \<UNNECESSARY_YIELD_TO\> for \<TURN_DIRECTION\> turn at intersection."              | Error    | Regulatory Element | The right_of_way regulatory element contains an unnecessary yield relationship that doesn't match the expected traffic rules.                         | Remove the unnecessary yield relationship to ensure accurate right-of-way modeling.                       |

## Related source codes

- right_of_way_with_traffic_lights.hpp
- right_of_way_with_traffic_lights.cpp
