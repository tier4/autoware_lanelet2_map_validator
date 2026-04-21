# Static build overlay files

The files in this directory are frozen snapshots used to produce the pre-built
release binary from [build-and-upload-binaries.yaml](../workflows/build-and-upload-binaries.yaml).

At release time, the workflow copies this directory over the checked-out tag,
overwriting the corresponding files in
`autoware_lanelet2_map_validator/gui/` and `autoware_lanelet2_map_validator/src/`.

## Policy

- Mainline changes under `autoware_lanelet2_map_validator/gui/` and
  `autoware_lanelet2_map_validator/src/main.cpp` /
  `autoware_lanelet2_map_validator/src/common/registration.cpp` are
  **intentionally NOT reflected** in the release binary.
- Update files here explicitly when the static build needs new behavior
  (for example, adding a new PyInstaller hidden import, registering a new
  validator into the static registration list, or bundling a new resource).
- The package layout under this directory mirrors the package subtree, so a
  plain recursive copy from `.github/static_build/` into
  `autoware_lanelet2_map_validator/` is all that is needed.

## Contents

| File                          | Purpose                                                  |
| ----------------------------- | -------------------------------------------------------- |
| `gui/gui.py`                  | GUI entry point built into the standalone binary         |
| `gui/gui_helper.py`           | Helpers (path resolution for bundled / non-bundled mode) |
| `gui/map_validator_gui.spec`  | PyInstaller spec for the standalone GUI                  |
| `src/main.cpp`                | CLI entry point wired for static linking                 |
| `src/common/registration.cpp` | Static validator registration (no dynamic plugin load)   |
