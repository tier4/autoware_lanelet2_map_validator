# Pilot-Auto map requirement sets (source tooling)

This directory defines how the shipped Pilot-Auto requirement JSON under `autoware_lanelet2_map_validator/map_requirements/pilot-auto/` is produced and checked. The ROS package installs only `map_requirements/`; this `tools/` tree is for maintainers and CI.

## Directory layout on disk

| Location                                             | Role                                                                                                                                            |
| ---------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- |
| `map_requirements/pilot-auto/v1.0.2/`, `v1.0.3/`, …  | Requirement-bundle directories kept for compatibility.                                                                                          |
| `tools/pilot_auto_map_requirements/index.json`       | Lists `groups/*.json` in merge order and sets `current_requirement_bundle_directory` (e.g. `v1.0.4`) for the current shipped subdirectory name. |
| `tools/pilot_auto_map_requirements/groups/*.json`    | One file per **vehicle_group** (`common`, `cargo_transport`, `delivery_robot`, `robo_taxi`, `shuttle_bus`).                                     |
| `tools/pilot_auto_map_requirements/fragments/*.json` | One fragment per requirement `id` (`id` + `validators`).                                                                                        |

## Group manifest schema

Each `groups/<name>.json` file contains:

- **`vehicle_group`**: Stable id for that family (matches the filename prefix of requirement files, e.g. `shuttle_bus`).
- **`requirement_bundle_versions`**: Distinct `version` strings that appear in that group’s shipped JSON (requirement-set / validator-bundle era). Listed in sorted order for a simple timeline.
- **`outputs`**: Entries merged globally at generation time. Each entry has:
  - **`path`**: Relative to `pilot-auto/` (e.g. `v1.0.4/shuttle_bus-v3_0_0.json`, `v1.0.3/shuttle_bus-v3_0_0.json`).
  - **`version`**: Written into the generated JSON’s top-level `version` field. May be omitted or empty to inherit from the `extends` target.
  - Either **`requirement_ids`** (full ordered list of `vm-xx-yy` ids) or **`extends`** + **`patches`**.

`extends` always references another output’s **`path`** (same path space). Cross-group references are allowed (for example delivery extending a cargo file).

### Patches

Only **`remove_ids`** and **`append_ids`** are supported. They are applied in array order; the resulting id list is then **sorted** for every output (see below).

| Patch                          | Effect                                    |
| ------------------------------ | ----------------------------------------- |
| `{"remove_ids": ["vm-01-02"]}` | Remove those ids from the working list.   |
| `{"append_ids": [...]}`        | Append ids, then the full list is sorted. |

### Requirement order in generated JSON

Shipped JSON lists `requirements` sorted by Pilot id **`vm-xx-yy`**: first by `xx`, then by `yy`. Composite ids such as `vm-01-03_vm-01-04` are ordered using the first eight characters (`vm-01-03`). The same sort is applied to `requirement_ids` in group files when you run **`bootstrap`**.

## Commands

Run from the package repository root (directory that contains `package.xml`), unless you pass `--tool-dir`.

### Regenerate shipped JSON

After editing `groups/*.json` or `fragments/*.json`:

```bash
python3 tools/pilot_auto_map_requirements/generate_pilot_auto_map_requirements.py
```

### Verify (optional, local)

This check is **not** run in repository CI; run it locally before you commit if you want to confirm sources and shipped JSON match. It fails if any file under `pilot-auto/` would change:

```bash
python3 tools/pilot_auto_map_requirements/generate_pilot_auto_map_requirements.py --check
```

### Bootstrap

Rebuilds `index.json`, all `groups/*.json` (flat `requirement_ids` per output), and all `fragments/` from the current `pilot-auto/**/*.json` on disk. Use when you intentionally resync sources from the tree or after large manual JSON edits.

```bash
python3 tools/pilot_auto_map_requirements/generate_pilot_auto_map_requirements.py bootstrap
```

### Compact

Dedupes identical requirement lists and adds `extends` / `remove_ids` edges where one list is a subsequence of a longer one, then **rewrites** all `groups/*.json`. Run after `bootstrap` if you want a diff-oriented manifest again, then run `--check`.

```bash
python3 tools/pilot_auto_map_requirements/generate_pilot_auto_map_requirements.py compact
```

### Options

- **`--tool-dir`**: Directory containing `index.json`, `groups/`, `fragments/` (default: this README’s directory).
- **`--pilot-auto`**: Override path to `map_requirements/pilot-auto`.
- **`--fragments`**: Override fragments directory.

## Fragments and validator consistency

Each requirement `id` must have the **same** `validators` in every output that includes it. If you add or change a validator for an id, edit the single `fragments/<id>.json` file and regenerate.

If two outputs ever needed different validators for the same id, `bootstrap` would fail; the tooling would need an extension (for example per-output fragment paths).

## Adding a new vehicle family

1. Register the group in `GROUP_FILE_BY_ID` and `GROUP_LOAD_ORDER` inside `generate_pilot_auto_map_requirements.py`.
2. Implement `_vehicle_group_from_output_path()` for the new filename prefix.
3. Add JSON under `pilot-auto/v1.0.4/` (or another `v1.0.x/` directory) and run **`bootstrap`**, then **`compact`** if desired, then **`--check`**. When the current bundle directory changes, update **`CURRENT_REQUIREMENT_BUNDLE_DIRECTORY`** in `generate_pilot_auto_map_requirements.py`, place files under `pilot-auto/<that name>/`, then run **`bootstrap`** (or hand-edit `groups/*.json` paths and `index.json` to match).

## Legacy `manifest.json`

If a file named `manifest.json` exists in this directory, the generator can fall back to it for migration only. Normal operation uses `index.json` plus `groups/*.json`.
