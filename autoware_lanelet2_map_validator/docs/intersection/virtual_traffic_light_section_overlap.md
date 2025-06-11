# virtual_traffic_light_section_overlap

## Validator name

mapping.intersection.virtual_traffic_light_section_overlap

## Feature

This validator checks whether there are no overlaps between paths of virtual traffic lights because Autoware may cause errors if it finds a `start_line` before reaching the expected `end_line`. **Note that this validator can only validate cases when two lanelet paths can be merged as a line graph for now,** since there are cases overlapping are acceptable when two lanelet paths have forks and it is difficult to detect such cases. Read [Algorithm](#algorithm) for further understanding.

The validator will output the following issue with the corresponding primitive ID.

| Issue Code                                         | Message                                                                                          | Severity | Primitive          | Description                                                                                                                         | Approach                                                            |
| -------------------------------------------------- | ------------------------------------------------------------------------------------------------ | -------- | ------------------ | ----------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------- |
| Intersection.VirtualTrafficLightSectionOverlap-001 | The virtual traffic light path has overlaps with that of regulatory element (ID: {reg_elem_id}). | Error    | Regulatory Element | Within a start_line to end_line path of a virtual traffic light, the validator found a start_line of another virtual traffic light. | Avoid start_line to end_line paths having overlaps with other ones. |

## Algorithm

### Flow Chart

The flow chart of this validation is present as below. Note that this validator can validate the overlap of lanelet paths only if the two lanelet paths can construct a line graph. Diverging paths cannot be validated.

![virtual_traffic_light_section_overlap_flow](../../media/virtual_traffic_light_section_overlap_flow.drawio.svg)

### Known Issues

When we have two virtual traffic lights A and B overlapping, there are two possible outcomes.

1. A single issue "A is overlapping with B" will be output.
2. Two issues "A is overlapping with B" and "B is overlapping with A" will be output.

This ambiguity comes from the algorithm of **"Nearby virtual traffic lights will be found if the its end_line is found in the path's bounding box"**. The size of the bounding box and where the `end_line` exists matters that the `end_line` of A might be inside the bounding box of B, while the opposite isn't (This is case 1).

## Parameters

| Parameter                | Default Value               | Description                                                                                                                                                                                                                                                                                                                                    |
| ------------------------ | --------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| validation_target_refers | [intersection_coordination] | A string array of `refers` type that should be validated by this validator. Since virtual traffic light may have different structures depending on its `refers` type, users can specify regulatory elements with its `refers` type for this validation. If the type of `refers` is not the one from this array, the validation will be skipped |

## Related source codes

- virtual_traffic_light_section_overlap.cpp
- virtual_traffic_light_section_overlap.hpp
