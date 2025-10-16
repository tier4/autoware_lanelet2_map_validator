# regulatory_element_details_for_traffic_signs

## Validator name

mapping.stop_line.regulatory_element_details_for_traffic_signs

## Feature

This validator checks whether the details in the `traffic_sign` subtype regulatory elements are valid.
Required information for traffic signs is written in the [Autoware documentation](https://autowarefoundation.github.io/autoware-documentation/main/design/autoware-architecture/map/map-requirements/vector-map-requirements-overview/category_stop_line/#vm-02-02-stop-sign).
This validator checks five types of issues related to traffic sign regulatory elements.

The output issue marks "linestring" or "regulatory element" as the **primitive**, and the lanelet ID is written together as **ID**.

| Issue Code                                           | Message                                                                                                                         | Severity | Primitive          | Description                                                                                                                                   | Approach                                                                                                                           |
| ---------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------- | -------- | ------------------ | --------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| StopLine.RegulatoryElementDetailsForTrafficSigns-001 | "This traffic_sign regulatory element is missing refers."                                                                       | Error    | regulatory element | There is a `traffic_sign` subtype regulatory element that has no `refers` linestrings                                                         | Add `refers` to the regulatory element that refers to the id of the traffic sign linestring.                                       |
| StopLine.RegulatoryElementDetailsForTrafficSigns-002 | "This refers linestring either does not have traffic_sign type or stop_sign subtype."                                           | Error    | linestring         | There is a `traffic_sign` subtype regulatory element whose `refers` is not a `traffic_sign` type and `stop_sign` subtype linestring.          | Check that the `refers` in the regulatory element is a `traffic_sign` type linestring with `stop_sign` subtype.                    |
| StopLine.RegulatoryElementDetailsForTrafficSigns-003 | "This traffic_sign regulatory element is missing ref_line."                                                                     | Error    | regulatory element | There is a `traffic_sign` subtype regulatory element that has no `ref_line`s                                                                  | Add `ref_line` to the regulatory element that refers to the id of the stop line linestring.                                        |
| StopLine.RegulatoryElementDetailsForTrafficSigns-004 | "This ref_line linestring does not have stop_line subtype."                                                                     | Error    | linestring         | There is a `traffic_sign` subtype regulatory element whose `ref_line` is not a `stop_line` subtype linestring.                                | Check that the `ref_line` in the regulatory element is a linestring with `stop_line` subtype.                                      |
| StopLine.RegulatoryElementDetailsForTrafficSigns-005 | "This traffic_sign regulatory element has a bounding box that exceeds the threshold, possibly containing unrelated primitives." | Warning  | regulatory element | The bounding box of all primitives in the regulatory element exceeds the maximum size threshold, indicating potentially unrelated primitives. | Review the regulatory element and remove any unrelated linestrings that are geometrically distant from the main traffic sign area. |

## Parameters

| Parameter Name        | Type   | Default Value | Description                                                                                                                   |
| --------------------- | ------ | ------------- | ----------------------------------------------------------------------------------------------------------------------------- |
| max_bounding_box_size | double | 20.0          | Maximum allowed diagonal size (in meters) of the bounding box containing all primitives in a traffic sign regulatory element. |

## Related source codes

- regulatory_element_details_for_traffic_signs.cpp
- regulatory_element_details_for_traffic_signs.hpp
