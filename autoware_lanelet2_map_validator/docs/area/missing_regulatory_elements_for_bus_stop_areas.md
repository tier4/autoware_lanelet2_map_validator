# missing_regulatory_elements_for_bus_stop_areas

## Validator name

mapping.area.missing_regulatory_elements_for_bus_stop_areas

## Feature

This validator checks whether each `bus_stop_area` type polygon has a relevant regulatory element that refers to it.
Required information for a bus stop area is written in the [Vector Map Requirements vm-06-06](https://docs.pilot.auto/en/reference-design/common/map-requirements/vector-map-requirements/category_area/#vm-06-06-bus-stop-area).

The output issue marks "polygon" as the **primitive**, and the polygon ID is written together as **ID**.

| Issue Code                                        | Message                                               | Severity | Primitive | Description                                                                                                             | Approach                                                                              |
| ------------------------------------------------- | ----------------------------------------------------- | -------- | --------- | ----------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------- |
| Area.MissingRegulatoryElementsForBusStopAreas-001 | "No regulatory element refers to this bus stop area." | Error    | Polygon   | There is a `bus_stop_area` type polygon that hasn't been referred to by any `bus_stop_area` subtype regulatory element. | Create a regulatory element with subtype `bus_stop_area` that refers to this polygon. |

## Parameters

None.

## Related source codes

- missing_regulatory_elements_for_bus_stop_areas.cpp
- missing_regulatory_elements_for_bus_stop_areas.hpp
