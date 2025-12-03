# roundabout_configuration

## Validator name

mapping.intersection.roundabout_configuration

## Feature

This validator validates VM-03-19 roundabout configuration requirements. It ensures that:

1. All possible routes from entry to exit lanelets consist only of entry, exit, or internal lanelets
2. All internal lanelets are properly connected between entry and exit lanelets
3. Internal lanelets without conflicting lanelets have the `enable_exit_turn_signal` tag set to true

## Related Map Requirement

[VM-03-19: Roundabout](https://docs.pilot.auto/en/reference-design/common/map-requirements/vector-map-requirements/category_intersection/#vm-03-19-roundabout)

| Issue Code                               | Message                                                                                                                     | Severity | Primitive          | Description                                                                           | Approach                                                                                                                             |
| ---------------------------------------- | --------------------------------------------------------------------------------------------------------------------------- | -------- | ------------------ | ------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------ |
| Intersection.RoundaboutConfiguration-001 | Route from entry lanelet {entry_lanelet_id} to exit lanelet {exit_lanelet_id} contains invalid lanelet {invalid_lanelet_id} | Error    | regulatory_element | A route contains a lanelet that is not classified as entry, exit, or internal         | Verify all routes from entry to exit only contain entry, exit, or internal lanelets                                                  |
| Intersection.RoundaboutConfiguration-002 | Internal lanelet {internal_lanelet_id} does not connect entry and exit lanelets properly                                    | Error    | regulatory_element | The internal lanelet's endpoints don't match the endpoints of entry and exit lanelets | Verify that internal lanelets have start/end points that align with the end point of entry lanelets and start point of exit lanelets |
| Intersection.RoundaboutConfiguration-003 | Internal lanelet {internal_lanelet_id} without conflicts is missing enable_exit_turn_signal tag                             | Error    | regulatory_element | Internal lanelet has no conflicting lanelets but lacks the required tag               | Add enable_exit_turn_signal="true" tag to internal lanelets without conflicts                                                        |
| Intersection.RoundaboutConfiguration-004 | Internal lanelet {internal_lanelet_id} without conflicts has enable_exit_turn_signal set to '{tag_value}' instead of 'true' | Error    | regulatory_element | Internal lanelet has no conflicting lanelets but the tag value is not set to true     | Set enable_exit_turn_signal="true" for internal lanelets without conflicts                                                           |

## Parameters

None.

## Notes

- The "three meter rule" mentioned in VM-03-19 is not implemented in this validator

## Related source codes

- roundabout_configuration.cpp
- roundabout_configuration.hpp
