# pedestrian_lane

## Validator name

mapping.lane.pedestrian_lane

## Feature

This validator checks the validity of pedestrian_lane subtype lanelets. It validates:

1. Whether the lanelet has at least one adjacent lanelet
2. If there is only one adjacent lanelet, checks whether it is a road subtype lanelet
3. If there is an empty side (no adjacent lanelet), checks whether the bound linestring has a road_border type

The validator uses the "validator" traffic rule to determine adjacency relationships through the routing graph.

| Issue Code              | Message                                                            | Severity | Primitive  | Description                                                                                                                                       | Approach                                                                              |
| ----------------------- | ------------------------------------------------------------------ | -------- | ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------- |
| Lane.PedestrianLane-001 | Pedestrian lane must have at least one adjacent lanelet.           | Error    | Lanelet    | The pedestrian lane has no adjacent lanelets on either side, which violates the requirement that pedestrian lanes should be adjacent to roadways. | Add appropriate adjacent lanelets or reconsider the pedestrian lane placement.        |
| Lane.PedestrianLane-002 | Adjacent lanelet must be a road subtype lanelet.                   | Error    | Lanelet    | The pedestrian lane has only one adjacent lanelet, but it is not of road subtype. Pedestrian lanes should be adjacent to road lanelets.           | Ensure the adjacent lanelet has subtype="road" or add appropriate road lanelets.      |
| Lane.PedestrianLane-003 | The bound linestring on the empty side must have road_border type. | Error    | LineString | When a pedestrian lane has only one adjacent lanelet, the boundary on the empty side should be marked as road_border to indicate the road edge.   | Set the type attribute of the boundary linestring on the empty side to "road_border". |

## Parameters

None.

## Related source codes

- pedestrian_lane.cpp
- pedestrian_lane.hpp
