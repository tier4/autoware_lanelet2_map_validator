# Static build overlay files

The files in this directory are frozen snapshots used to produce the pre-built
release binary from [build-and-upload-binaries.yaml](../workflows/build-and-upload-binaries.yaml).

At release time, the workflow checks out the release tag, then overlays this
directory's contents on top of the checked-out tree before invoking the build.

## Policy

- Mainline changes under `autoware_lanelet2_map_validator/gui/`,
  `autoware_lanelet2_map_validator/src/main.cpp`,
  `autoware_lanelet2_map_validator/src/common/registration.cpp`, or
  `autoware_lanelet2_map_validator/CMakeLists.txt` are **intentionally NOT
  reflected** in the release binary.
- Update files here explicitly when the static build needs new behavior
  (for example, adding a new PyInstaller hidden import, registering a new
  validator into the static registration list, bundling a new resource, or
  adjusting the static build's CMake configuration).
- The package layout mirrors the target subtree so the overlay step is a plain
  recursive copy. The only exception is `external/`, which holds overrides for
  repositories cloned on the fly during the build (their clone path does not
  exist at overlay time).

## Contents

| File                                                  | Purpose                                                                                                           |
| ----------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| `CMakeLists.txt`                                      | Replaces `autoware_lanelet2_map_validator/CMakeLists.txt` for the static/standalone build                         |
| `gui/gui.py`                                          | GUI entry point built into the standalone binary                                                                  |
| `gui/gui_helper.py`                                   | Helpers (path resolution for bundled / non-bundled mode)                                                          |
| `gui/map_validator_gui.spec`                          | PyInstaller spec for the standalone GUI                                                                           |
| `src/main.cpp`                                        | CLI entry point wired for static linking                                                                          |
| `src/common/registration.cpp`                         | Static validator registration (no dynamic plugin load)                                                            |
| `external/autoware_lanelet2_extension/CMakeLists.txt` | Replaces the `autoware_lanelet2_extension/autoware_lanelet2_extension/CMakeLists.txt` of the cloned external repo |
