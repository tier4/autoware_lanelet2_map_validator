---
name: build-test
description: Use this skill to build this project.
context: fork
---

# build-test

If no arguments are provided, do both build and testing.

Recognize and memorize `colcon_root` directory by tracking back to parent folder until `autoware.repos` is found and assert that this repository is belongs to `colcon_root/src`.

## Build

`colcon build` should be called in a different context to avoid context rot.

Go to `colcon_root` directory and run build command as specified in `AGENTS.md` of this project.

### Report

- pass / fail
- if build failed,
  - which package failed
  - which files and which positions caused the compilation error

Error messages from Boost.Geometry template assertion tend to be very long, but it should contain the source file name and lexical location of the cause.

## Test

Call

`colcon test --packages-select autoware_lanelet2_map_validator`

### Report

- pass / failed tests
