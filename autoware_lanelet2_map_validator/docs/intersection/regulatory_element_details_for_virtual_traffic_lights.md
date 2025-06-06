# regulatory_element_details_for_virtual_traffic_lights

## Validator name

mapping.intersection.regulatory_element_details_for_virtual_traffic_lights

## Feature

This validator checks whether the regulatory element of `virtual_traffic_light` is constructed correctly.
Note that wrong numbers of `start_line`s and `end_line`s wil be detected during the map loading, not in the validation process.

The validator outputs the following issue with the corresponding ID of the primitive.

| Issue Code                                                       | Message                                                                                            | Severity | Primitive          | Description              | Approach                                                                                                                  |
| ---------------------------------------------------------------- | -------------------------------------------------------------------------------------------------- | -------- | ------------------ | ------------------------ | ------------------------------------------------------------------------------------------------------------------------- |
| Intersection.RegulatoryElementDetailsForVirtualTrafficLights-001 | The start_line of a virtual_traffic_light regulatory element must be a "virtual" type.             | Error    | Linestring         | (No further explanation) | Set the type of the start_line linestring to `virtual`.                                                                   |
| Intersection.RegulatoryElementDetailsForVirtualTrafficLights-002 | A virtual_traffic_light regulatory element must only have a single ref_line.                       | Error    | Regulatory Element | (No further explanation) | Set exactly one `ref_line` to the regulatory element. Split to multiple elements if there are multiple stop lines needed. |
| Intersection.RegulatoryElementDetailsForVirtualTrafficLights-003 | The ref_line of a virtual_traffic_light regulatory element must be a "stop_line" type.             | Error    | Linestring         | (No further explanation) | Set the type of the ref_line (stop_line) linestring to `stop_line`.                                                       |
| Intersection.RegulatoryElementDetailsForVirtualTrafficLights-004 | The end_line of a virtual_traffic_light regulatory element must be a "virtual" type.               | Error    | Linestring         | (No further explanation) | Set the type of the end_line linestring to `virtual`.                                                                     |
| Intersection.RegulatoryElementDetailsForVirtualTrafficLights-005 | A virtual_traffic_light regulatory element must have a refers.                                     | Error    | Regulatory Element | (No further explanation) | Create an `intersection_coordination` type linestring and set it as `refers` in the regulatory element.                   |
| Intersection.RegulatoryElementDetailsForVirtualTrafficLights-006 | The refers of a virtual_traffic_light regulatory element must be any of [{available_refers}] type. | Error    | Linestring         | (No further explanation) | Set the type of the refers linestring to `intersection_coordination`.                                                     |

## Parameters

| Parameter             | Default Value               | Description                                                                                                                                              |
| --------------------- | --------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- |
| available_refers_type | [intersection_coordination] | A string array of possible `refers` type that are expected. If the type of `refers` is not the one from this array, this validator will output an error. |

## Related source codes

- regulatory_element_details_for_virtual_traffic_lights.cpp
- regulatory_element_details_for_virtual_traffic_lights.hpp
