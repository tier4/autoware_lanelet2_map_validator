# single_virtual_traffic_light_reference

## Validator name

mapping.intersection.single_virtual_traffic_light_reference

## Feature

Feature explanation here.

| Issue Code                                          | Message                                                                           | Severity | Primitive | Description                                                                       | Approach                                                  |
| --------------------------------------------------- | --------------------------------------------------------------------------------- | -------- | --------- | --------------------------------------------------------------------------------- | --------------------------------------------------------- |
| Intersection.SingleVirtualTrafficLightReference-001 | A lanelet cannot refer to more than one virtual_traffic_light regulatory element. | Error    | Lanelet   | A lanelet can only have one virtual_traffic_light regulatory element to refer to. | Remove the reference to redundant virtual_traffic_lights. |

## Parameters

None.

## Related source codes

- single_virtual_traffic_light_reference.cpp
- single_virtual_traffic_light_reference.hpp
