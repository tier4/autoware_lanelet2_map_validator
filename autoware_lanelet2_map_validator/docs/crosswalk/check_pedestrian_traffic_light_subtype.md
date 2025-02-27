# check_pedestrian_traffic_light_subtype

## Validator name

mapping.crosswalk.pedestrian_traffic_light_subtype

## Feature

This validator checks whether all traffic light linestrings referred by `crosswalk` subtype lanelets has a `red_green` subtype.
Other attributes that both pedestrian traffic lights and normal traffic lights have in common should be validated in [recommended prerequisites](#recommended-prerequisites).

| Issue Code                                  | Message                                                            | Severity | Description                                                                                                                    | Approach                                                        |
| ------------------------------------------- | ------------------------------------------------------------------ | -------- | ------------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------- |
| Crosswalk.PedestrianTrafficLightSubtype-001 | Subtype of pedestrian traffic lights should be set as "red_green". | Error    | All traffic lights referred by crosswalk lanelets should be pedestrian traffic lights whose linestring subtype is "red_green". | Set the subtype of the traffic light linestring to "red_green". |

## Recommended prerequisites

- mapping.traffic_light.missing_referrers
- mapping.traffic_light.missing_regulatory_elements

## Related source codes

- check_pedestrian_traffic_light_subtype.cpp
- check_pedestrian_traffic_light_subtype.hpp
