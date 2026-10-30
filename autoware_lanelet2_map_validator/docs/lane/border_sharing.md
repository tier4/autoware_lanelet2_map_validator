# border_sharing

🇬🇧 **English ver** | 🇯🇵 [日本語 ver](./border_sharing_ja.md)

## Validator name

mapping.lane.border_sharing

## Feature

This validator checks whether physically adjacent lanelets share the same linestring as their border, and whether lanelets separated by a fence don't share their border.
See Autoware Documentation [vm-01-03 Linestring sharing](https://autowarefoundation.github.io/autoware-documentation/main/design/autoware-architecture/map/map-requirements/vector-map-requirements-overview/category_lane/#vm-01-03-linestring-sharing) for more information.

The validator only uses what is written in the map file. It doesn't guess whether an object (a fence, a pole, a median strip, etc.) exists in the real world: an element in the map file is treated as existing, and an element not in the map file is treated as not existing.

The validator will output the following issues with the corresponding primitive ID.

| Issue Code             | Message                                                                              | Severity | Primitive | Description                                                                                                                                                                                                                     | Approach                                                                                                                                                                                               |
| ---------------------- | ------------------------------------------------------------------------------------ | -------- | --------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Lane.BorderSharing-001 | "Seems to be adjacent with Lanelet \<ID\> but doesn't share a border linestring."    | Error    | Lanelet   | The lanelet and lanelet \<ID\> are adjacent with no fence between them, but their borders are different linestrings. **Note that the same issue will be created against lanelet \<ID\> so two issues rise from one violation.** | Create a common border linestring of both lanelets and set the left bound or right bound of them to that linestring. If a fence lies between them in the real world, add the fence to the map instead. |
| Lane.BorderSharing-003 | "Shares a border linestring with Lanelet \<ID\> although a fence lies between them." | Error    | Lanelet   | The lanelet and lanelet \<ID\> share a border linestring, but a fence lies on the border. **Note that the same issue will be created against lanelet \<ID\> so two issues rise from one violation.**                            | Create separate border linestrings for the two lanelets. If no fence exists in the real world, remove the fence from the map or fix the type of the border instead.                                    |

## Algorithm

The validator examines every pair of lanelets (lanelet A and lanelet B) and a bound of each lanelet (bound L1 of lanelet A and bound L2 of lanelet B).

### Step 1: Pairs to be skipped

The pair is skipped (no issue) if any of the following conditions holds.

1. Neither lanelet A nor lanelet B has `subtype:road`. Such pairs don't affect the planning of Autoware.
2. L1 and L2 are not at the same position from end to end. "The same position" means that L1 and L2 use the same points in the same order (or in the reverse order), or that all points of L1 and L2 have the same coordinates including the height. Borders that are close but not exactly at the same position, and borders that are at the same position only in a part of their length, are not judged as adjacent.
3. Lanelet A and lanelet B are on the same side of the border (their areas overlap). The side is determined from whether the border is the left bound or the right bound of each lanelet and from the direction of its points.

### Step 2: Fence on the border

A fence lies on the border if L1 or L2 has `type:fence`, or if a linestring with `type:fence` is at the same position as at least one segment (two consecutive points) of L1 or L2. A fence at another position (e.g. inside a lanelet) is not considered.

| Fence on the border | L1 and L2 are the same linestring | Result                 |
| ------------------- | --------------------------------- | ---------------------- |
| No                  | Yes                               | OK                     |
| No                  | No                                | Lane.BorderSharing-001 |
| Yes                 | Yes                               | Lane.BorderSharing-003 |
| Yes                 | No                                | OK                     |

## Known limitations

To avoid reporting errors for maps made without mistakes, the validator doesn't detect the following cases.

- Borders drawn at slightly different positions, with a gap, or with an overlap. Lanelets separated by an object with width (e.g. a median strip) have the same shape in the map file.
- Borders with different heights only. Lanelets on the upper and lower levels of a road have the same shape in the map file.
- Borders at the same position only in a part of their length, e.g. when the adjacent lanelets are divided at different positions. Lanelets branching in an intersection have the same shape in the map file.
- Lanelets sharing a border without any object drawn in the map file between them, even if a pole, a median strip, or a fence exists in the real world.
- Pairs without a `subtype:road` lanelet.

## Parameters

None.

## Related source codes

- border_sharing.cpp
- border_sharing.hpp

## Discontinued issues

Some issues are currently discontinued.

| Issue Code             | Message                                                                                   | Severity | Primitive | Description                                                                                                                                                                                                                                                  | Approach                                                                                       |
| ---------------------- | ----------------------------------------------------------------------------------------- | -------- | --------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------- |
| Lane.BorderSharing-002 | "Seems to be adjacent with Lanelet \<multiple IDs\> but doesn't share a border linestring | Error    | Lanelet   | The validator judged that the lanelet and lanelet \<ID\> is adjacent but doesn't share border linestrings. This violation may be caused from incorrect lanelet division, and the issue will be made only for the largest lanelet. (Used until version 1.8.2) | Divide this lanelet properly and then set a common border linestring to each adjacent lanelet. |
