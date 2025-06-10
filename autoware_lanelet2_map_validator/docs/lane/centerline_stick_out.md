# centerline_stick_out

## Validator name

mapping.lane.centerline_stick_out

## Feature

This validator checks whether custom centerline of lanelets have valid geometry such as...

- All points of the centerline is inside the belonging lanelet
- All points are not apart from the lanelet plane.

The validator will output the following issues with the corresponding primitive ID.

| Issue Code                  | Message                                                                                            | Severity | Primitive  | Description                                                                                                  | Approach                                                                                                                                               |
| --------------------------- | -------------------------------------------------------------------------------------------------- | -------- | ---------- | ------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Lane.CenterlineStickOut-001 | This centerline contains points outside of the lanelet. (Point IDs: {point_ids})                   | Error    | Linestring | All points of a centerline should be inside or on the border of the belonging lanelet but some points don't. | Make sure all points of the centerline is inside the lanelet.                                                                                          |
| Lane.CenterlineStickOut-002 | TThis centerline contains points that are distant from the lanelet plane. (Point IDs: {point_ids}) | Error    | Linestring | All points of a centerline should be on the lanelet plane but some points don't.                             | Make sure all points of the centerline is on the lanelet plane. Be careful the points are not apart from the plane especially in the height direction. |

## Parameters

| Parameter        | Default Value | Description                                                                                                                                                                                                                                                                                                    |
| ---------------- | ------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| dimension_mode   | 3D            | Select from a string "2D" or "3D". If "3D" is selected, this validator will check both `Lane.CenterlineStickOut-001` and `Lane.CenterlineStickOut-002`. If "2D" is selected, this validator will only check about `Lane.CenterlineStickOut-001`.                                                               |
| planar_threshold | 0.01          | The threshold to validate whether a point is outside of the lanelet polygon **in meters**. Used to validate issue `Lane.CenterlineStickOut-001`. This threshold is provided since restrict validation whether a point is inside the lanelet is difficult when the point is on the border of multiple lanelets. |
| height_threshold | 0.1           | The threshold to validate whether a point is distant from the lanelet plane **in meters**. Used to validate issue `Lane.CenterlineStickOut-002`. This threshold is provided since restrict validation whether a point is inside the lanelet is difficult when the point is on the border of multiple lanelets. |

## Related source codes

- centerline_stick_out.cpp
- centerline_stick_out.hpp
