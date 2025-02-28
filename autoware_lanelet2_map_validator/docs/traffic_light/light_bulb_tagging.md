# light_bulb_tagging

## Validator name

mapping.traffic_light.light_bulb_tagging

## Feature

This validator checks each point representing a light bulb of a traffic light has valid tagging.
Each light bulb MUST have a `color` tag which is either `red`, `yellow`, or `green`.
A light bulb CAN also have an `arrow` tag but it MUST be either `up`, `left`, `right`, `up_right`, or `up_left`.

| Issue Code                        | Message                                                                             | Severity | Primitive | Description                                                                 | Approach                                                                            |
| --------------------------------- | ----------------------------------------------------------------------------------- | -------- | --------- | --------------------------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| TrafficLight.LightBulbTagging-001 | "A point representing a light bulb should have a color tag."                        | Error    | Point     | Points consisting a `light_bulbs` linestring should have a `color` tag set. | Add a `color` tag to the point and set its color (red, yellow or green).            |
| TrafficLight.LightBulbTagging-002 | "The color of a light bulb should be either red, yellow, or green."                 | Error    | Point     | The `color` tag of the point has an invalid color.                          | The color must be set to `red`, `yellow` or `green`.                                |
| TrafficLight.LightBulbTagging-003 | "The arrow of a light bulb should be either up, right, left, up_right, or up_left." | Error    | Point     | The `arrow` tag of the point has an invalid arrow direction.                | The arrow direction must be set to `up`, `left`, `right`, `up_right`, or `up_left`. |

## Related source codes

- light_bulb_tagging.cpp
- light_bulb_tagging.hpp
