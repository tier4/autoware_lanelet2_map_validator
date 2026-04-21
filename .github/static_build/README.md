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
- When mainline adds a GUI feature that needs to reach the release binary, the
  overlay copies of `gui.py` / `gui_helper.py` / `map_validator_gui.spec` must
  be refreshed by 3-way merging mainline changes into the snapshot. See the
  "Refresh procedure" section below.
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
| `gui/pyi_rth_distutils.py`                            | PyInstaller runtime hook shimming `distutils` on Python 3.12 (referenced by the spec)                             |
| `src/main.cpp`                                        | CLI entry point wired for static linking                                                                          |
| `src/common/registration.cpp`                         | Static validator registration (no dynamic plugin load)                                                            |
| `external/autoware_lanelet2_extension/CMakeLists.txt` | Replaces the `autoware_lanelet2_extension/autoware_lanelet2_extension/CMakeLists.txt` of the cloned external repo |

## Refresh procedure (when mainline GUI changes need to reach the release)

The GUI overlay files were originally derived from commit
`c8192e2311785a21894d7311f079b28caa541ca5` on a helper branch (the last
known-good "static build works" snapshot). When mainline evolves and you need
those changes in the release binary, do a 3-way merge for each overlay file:

```bash
# base   = c8192e2^   (before static-build adaptations)
# ours   = c8192e2    (static-build adaptations)
# theirs = current mainline
for f in gui.py gui_helper.py; do
  p=autoware_lanelet2_map_validator/gui/$f
  git show c8192e2^:$p   > /tmp/base-$f
  git show c8192e2:$p    > /tmp/ours-$f
  git show HEAD:$p       > /tmp/theirs-$f
  git merge-file -p --diff3 /tmp/ours-$f /tmp/base-$f /tmp/theirs-$f \
    > .github/static_build/gui/$f  # resolve conflicts by hand if any
done
```

`map_validator_gui.spec` has diverged too aggressively for an automated merge;
keep the hand-crafted spec here in sync with mainline's spec design, but retain
these overlay-specific adaptations:

- Bundle the `autoware_lanelet2_extension_python` module's `.so` and `.py`
  files (mainline's spec does not, but `gui_helper.py` imports from it).
- Keep `name='autoware_lanelet2_map_validator_gui'` — the release workflow's
  packaging step looks for that filename.
