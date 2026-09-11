---
name: develop-validator
description: Use this skill to develop validators
allowed-tools: Read Grep WebFetch
---

# develop-validator

## Identify vector map specification

Analyze the request based on [the vector map requirements written in the Autoware Documentation](https://docs.autoware.org/main/design/autoware-architecture-v1/components/map/map-requirements/vector-map-requirements-overview/) and identify following items:

- Which categories does the request relate to
- Based on the categories and [requirement - validators relation](../../../autoware_lanelet2_map_validator/map_requirements/autoware_requirement_set.json), which validators are responsbile for the request

If no related categories can be found, the request is out of the scope of validation, so terminate.

If no related validators are found, create a new one as specified in [how_to_contribute.md](../../../autoware_lanelet2_map_validator/docs/how_to_contribute.md).

## Fix / add validators

First of all, create or read the document of the target validators under [validator implementation docs](../../../autoware_lanelet2_map_validator/docs/).

Other validators in the same category should be useful when implementing new one.

For newly added validator, corresonding test lanelet map (`.osm`) should be created by the agent or by the user to test both positive cases and negative cases.

## Test

use `/build-test` to check that the changes work as expected and do not break existing features.
