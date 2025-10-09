# regulatory_element_details_for_crosswalks

## Validator name

mapping.crosswalk.regulatory_element_details

## Feature

This validator checks whether the details in the `crosswalk` subtype regulatory elements are valid.
Required information for a crosswalk is written in the [Autoware documentation](https://autowarefoundation.github.io/autoware-documentation/main/design/autoware-architecture/map/map-requirements/vector-map-requirements-overview/category_crosswalk/#vm-05-01-crosswalks-across-the-road).
This validator checks nine types of issues.

The output issue marks "lanelet", "linestring" or "regulatory_element" as the **primitive**, and the regulatory element ID is written together as **ID**.

| Issue Code                             | Message                                                                            | Severity | Primitive          | Description                                                                                                       | Approach                                                                                                                        |
| -------------------------------------- | ---------------------------------------------------------------------------------- | -------- | ------------------ | ----------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| Crosswalk.RegulatoryElementDetails-001 | "Regulatory element of crosswalk must have lanelet of crosswalk(refers)."          | Error    | regulatory element | There is a `crosswalk` subtype regulatory element that has no `refers`es.                                         | Write `refers` referring to a `crosswalk` subtype lanelet in the regulatory element                                             |
| Crosswalk.RegulatoryElementDetails-002 | "Regulatory element of crosswalk must have only one lanelet of crosswalk(refers)." | Error    | regulatory element | There is a `crosswalk` subtype regulatory element that has multiple `refers`es.                                   | A `crosswalk` subtype regulatory element can have only one `refers`. Remove the `refers` that is not a crosswalk lanelet.       |
| Crosswalk.RegulatoryElementDetails-003 | "Regulatory element of crosswalk does not have stop line(ref_line)."               | Info     | regulatory element | There is a `crosswalk` subtype regulatory element that has no `ref_line`s                                         | Generally, there should be a stop line for the crosswalk. Be sure that the stop line exists or doesn't.                         |
| Crosswalk.RegulatoryElementDetails-009 | "Regulatory element of crosswalk should have only one stop line(ref_line)."        | Error    | regulatory element | There is a `crosswalk` subtype regulatory element that has multiple `ref_line`s which should be only one.         | Remove other `ref_lines` and create regulatory elements that refers to the same `crosswalk subtype` lanelet for each stop line. |
| Crosswalk.RegulatoryElementDetails-006 | "Refers of crosswalk regulatory element must have type of crosswalk."              | Error    | lanelet            | There is a `crosswalk` subtype regulatory element whose `refers` is not a `crosswalk` subtype lanelet.            | Check that the `refers` is a `crosswalk` subtype lanelet                                                                        |
| Crosswalk.RegulatoryElementDetails-007 | "ref_line of crosswalk regulatory element must have type of stopline."             | Error    | linestring         | There is a `crosswalk` subtype regulatory element whose `ref_line` is not a `stop_line` type linestring.          | Check that the `ref_line` is a `stop_line` type linestring                                                                      |
| Crosswalk.RegulatoryElementDetails-010 | "Attribute participant:pedestrian not found from refers."                          | Error    | lanelet            | The `refers` lanelet (crosswalk lanelet) doesn't have an attribute `participant:pedestrian`.                      | Add the attribute `participant:pedestrian` to the lanelet and set the value to `yes` or `true`.                                 |
| Crosswalk.RegulatoryElementDetails-011 | "Attribute participant:pedestrian of refers is not set to "yes" or "true"."        | Error    | lanelet            | The attribute `participant:pedestrian` of the `refers` lanelet (crosswalk lanelet) is not set to `yes` or `true`. | Set the attribute `participant:pedestrian` of the crosswalk lanelet to `yes` or `true`.                                         |
| Crosswalk.RegulatoryElementDetails-012 | "This crosswalk regulatory element has a bounding box that exceeds the threshold, possibly containing unrelated primitives." | Warning  | regulatory element | The bounding box of all primitives in the regulatory element exceeds the configured threshold, indicating possible unrelated primitives. | Review the regulatory element to ensure all referenced primitives (refers, ref_lines, crosswalk_polygons) are actually related to this crosswalk. Remove any unrelated primitives. |

## Related source codes

- regulatory_element_details_for_crosswalks.hpp
- regulatory_element_details_for_crosswalks.cpp

## Discontinued issues

Some issues are currently discontinued.

| Issue Code                             | Message                                                                                  | Severity | Primitive          | Description                                                                                                                                       | Approach                                                                                                                                                                  |
| -------------------------------------- | ---------------------------------------------------------------------------------------- | -------- | ------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Crosswalk.RegulatoryElementDetails-004 | "Regulatory element of crosswalk is nice to have crosswalk_polygon."                     | Warning  | regulatory element | There is a `crosswalk` subtype regulatory element that has no `crosswalk_polygon`s. (Removed in version 1.1.0)                                    | It is recommended to surround a crosswalk with a `crosswalk_polygon`. Create one and add a `crosswalk_polygon` role member to the regulatory element with the polygon ID. |
| Crosswalk.RegulatoryElementDetails-005 | "Regulatory element of crosswalk must have only one crosswalk_polygon."                  | Error    | regulatory element | There is a `crosswalk` subtype regulatory element that has multiple `crosswalk_polygon`s. (Removed in version 1.1.0)                              | Only one `crosswalk_polygon` is allowed per crosswalk. Remove the unnecessary ones.                                                                                       |
| Crosswalk.RegulatoryElementDetails-008 | "Crosswalk polygon of crosswalk regulatory element must have type of crosswalk_polygon." | Error    | polygon            | There is a `crosswalk` subtype regulatory element whose `crosswalk_polygon` is not a `crosswalk_polygon` type polygon. (Removed in version 1.1.0) | Check that the `crosswalk_polygon` mentioned in the regulatory element refers to a `crosswalk_polygon` type area.                                                         |
