#!/usr/bin/env python3
# Copyright 2026 TIER IV, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Generate map_requirements/pilot-auto/**/*.json from group manifests + fragments.

Layout
------
* ``index.json`` — lists ``groups/*.json`` (per vehicle family). Current validator
  bundle JSON for Pilot-Auto lives under ``pilot-auto/v1.0.4/``; older bundles stay
  under ``pilot-auto/v1.0.2/``, ``pilot-auto/v1.0.3/``, etc.
* Each group file documents ``vehicle_group``, an ordered ``requirement_bundle_versions``
  list (the ``version`` field stored in each shipped JSON), and ``outputs`` for that
  family only. ``extends`` targets use paths relative to ``pilot-auto/`` and may point
  at another group's file (e.g. delivery vs cargo).

Manifest rows (inside each group file ``outputs``)
--------------------------------------------------
* Full row: ``{"path", "version", "requirement_ids": [...]}``.
* Derived row: ``{"path", "version"?, "extends": "<path>", "patches": [...]}``.
  If ``version`` is omitted or empty, it is inherited from the ``extends`` target.

Patches (applied in array order; then requirement ids are sorted for output)
----------------------------------------------------------------------------
* ``{"remove_ids": ["vm-01-02"]}`` — drop ids from the working list.
* ``{"append_ids": ["vm-99-01"]}`` — append ids, then the full list is sorted.

Requirement ids are sorted by Pilot ``vm-xx-yy`` (major ``xx``, minor ``yy``).
Composite ids such as ``vm-01-03_vm-01-04`` use the first eight characters
(``vm-01-03``) for ordering.

Commands: ``generate`` (default); ``bootstrap`` scans ``pilot-auto`` and writes
``index.json`` + ``groups/*.json`` + fragments; ``compact`` deduplicates ``extends``
/ patches across the merged outputs and rewrites the group files.
"""

from __future__ import annotations

import argparse
from collections import defaultdict
import json
from pathlib import Path
import re
import sys
from typing import Any

# Relative paths under tools/pilot_auto_map_requirements/
# Subdirectory under map_requirements/pilot-auto/ for the current shipped bundle
# (must match JSON top-level "version" / bundle era; also written to index.json).
CURRENT_REQUIREMENT_BUNDLE_DIRECTORY = "v1.0.6"

GROUP_INDEX_PATH = "index.json"
GROUP_FILE_BY_ID: dict[str, str] = {
    "common": "groups/common.json",
    "cargo_transport": "groups/cargo_transport.json",
    "delivery_robot": "groups/delivery_robot.json",
    "robo_taxi": "groups/robo_taxi.json",
    "shuttle_bus": "groups/shuttle_bus.json",
}
GROUP_LOAD_ORDER = [
    "groups/common.json",
    "groups/cargo_transport.json",
    "groups/delivery_robot.json",
    "groups/robo_taxi.json",
    "groups/shuttle_bus.json",
]

_VM_HEAD = re.compile(r"^vm-(\d+)-(\d+)")


def _vm_major_minor(req_id: str) -> tuple[int, int]:
    """Parse vm-xx-yy from the first 8 characters (covers vm-01-03_vm-01-04 -> vm-01-03)."""
    head = req_id[:8] if len(req_id) >= 8 else req_id
    m = _VM_HEAD.match(head)
    if m:
        return int(m.group(1)), int(m.group(2))
    return (99_999, 99_999)


def _sort_requirement_ids(ids: list[str]) -> list[str]:
    """Sort by vm major (xx) then minor (yy), then full id string."""
    return sorted(ids, key=lambda rid: (_vm_major_minor(rid), rid))


def _tool_dir() -> Path:
    return Path(__file__).resolve().parent


def _default_pilot_auto() -> Path:
    return (
        _tool_dir().parent.parent
        / "autoware_lanelet2_map_validator"
        / "map_requirements"
        / "pilot-auto"
    ).resolve()


def _vehicle_group_from_output_path(output_path: str) -> str:
    name = Path(output_path).name
    if name.startswith("common"):
        return "common"
    if name.startswith("cargo_transport"):
        return "cargo_transport"
    if name.startswith("delivery_robot"):
        return "delivery_robot"
    if name.startswith("robo_taxi"):
        return "robo_taxi"
    if name.startswith("shuttle_bus"):
        return "shuttle_bus"
    raise ValueError(f"Cannot infer vehicle_group from output path {output_path!r}")


def _fragment_path(fragments_dir: Path, requirement_id: str) -> Path:
    safe = requirement_id.replace("/", "_")
    return fragments_dir / f"{safe}.json"


def _load_json(path: Path) -> dict:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def _dump_requirement_set(version: str, requirements: list[dict]) -> str:
    obj = {"version": version, "requirements": requirements}
    return json.dumps(obj, indent=2, ensure_ascii=False) + "\n"


def _load_fragment(fragments_dir: Path, requirement_id: str) -> dict:
    fp = _fragment_path(fragments_dir, requirement_id)
    if not fp.is_file():
        raise FileNotFoundError(f"Missing fragment for id {requirement_id!r}: {fp}")
    data = _load_json(fp)
    if data.get("id") != requirement_id:
        raise ValueError(
            f"Fragment id mismatch in {fp}: expected {requirement_id!r}, got {data.get('id')!r}"
        )
    return {"id": requirement_id, "validators": data["validators"]}


def _remove_ids_to_get_subsequence(want: list[str], source: list[str]) -> list[str] | None:
    si = 0
    removed: list[str] = []
    for x in source:
        if si < len(want) and x == want[si]:
            si += 1
        else:
            removed.append(x)
    if si == len(want):
        return removed
    return None


def _apply_patches(base_ids: list[str], patches: list[dict[str, Any]]) -> list[str]:
    cur = list(base_ids)
    for p in patches:
        if not p:
            continue
        keys = set(p.keys())
        if keys == {"remove_ids"}:
            remove = set(p["remove_ids"])
            cur = [x for x in cur if x not in remove]
            continue
        if keys == {"append_ids"}:
            cur = cur + list(p["append_ids"])
            continue
        raise ValueError(f"Unsupported patch keys {keys}: {p!r}")
    return _sort_requirement_ids(cur)


def _manifest_entries_by_path(manifest: dict) -> dict[str, dict]:
    return {e["path"]: e for e in manifest["outputs"]}


def _resolve_requirement_ids(
    path: str,
    entries_by_path: dict[str, dict],
    memo: dict[str, list[str]],
    visiting: set[str],
) -> list[str]:
    if path in memo:
        return memo[path]
    if path in visiting:
        raise ValueError(f"Cycle in manifest extends graph involving {path!r}")
    visiting.add(path)
    try:
        entry = entries_by_path[path]
        if "requirement_ids" in entry:
            ids = _sort_requirement_ids(list(entry["requirement_ids"]))
        else:
            parent = entry["extends"]
            base = _resolve_requirement_ids(parent, entries_by_path, memo, visiting)
            patches = entry.get("patches") or []
            ids = _apply_patches(base, patches)
        memo[path] = ids
        return ids
    finally:
        visiting.remove(path)


def _resolve_version(path: str, entries_by_path: dict[str, dict], memo: dict[str, str]) -> str:
    if path in memo:
        return memo[path]
    entry = entries_by_path[path]
    v = entry.get("version")
    if v is not None and v != "":
        memo[path] = v
        return memo[path]
    if "extends" not in entry:
        raise ValueError(f"Output {path!r} has no version and no extends")
    v = _resolve_version(entry["extends"], entries_by_path, memo)
    memo[path] = v
    return v


def _effective_version(entry: dict, entries_by_path: dict[str, dict]) -> str:
    v = entry.get("version")
    if v is not None and v != "":
        return v
    if "extends" not in entry:
        raise ValueError("Entry has neither version nor extends")
    return _resolve_version(entry["extends"], entries_by_path, {})


def _load_merged_manifest(tool_dir: Path) -> dict:
    index_path = tool_dir / GROUP_INDEX_PATH
    if index_path.is_file():
        index = _load_json(index_path)
        outputs: list[dict] = []
        for rel in index.get("groups", []):
            group_path = tool_dir / rel
            if not group_path.is_file():
                raise FileNotFoundError(f"Group file listed in index but missing: {group_path}")
            group = _load_json(group_path)
            outputs.extend(group["outputs"])
        return {"outputs": outputs}
    legacy = tool_dir / "manifest.json"
    if legacy.is_file():
        return _load_json(legacy)
    raise FileNotFoundError(
        f"Neither {index_path} nor {legacy} found under {tool_dir}. Run bootstrap first."
    )


def _write_index_and_groups(tool_dir: Path, outputs: list[dict]) -> None:
    by_group: dict[str, list[dict]] = defaultdict(list)
    for o in outputs:
        by_group[_vehicle_group_from_output_path(o["path"])].append(o)

    for gid, relpath in GROUP_FILE_BY_ID.items():
        outs = sorted(by_group.get(gid, []), key=lambda x: x["path"])
        versions_seen: list[str] = []
        seen = set()
        for o in outs:
            v = o.get("version") or ""
            if v and v not in seen:
                seen.add(v)
                versions_seen.append(v)
        bundle_versions = sorted(versions_seen)
        body = {
            "vehicle_group": gid,
            "requirement_bundle_versions": bundle_versions,
            "outputs": outs,
        }
        out_path = tool_dir / relpath
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(json.dumps(body, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    index_body = {
        "current_requirement_bundle_directory": CURRENT_REQUIREMENT_BUNDLE_DIRECTORY,
        "groups": GROUP_LOAD_ORDER,
    }
    (tool_dir / GROUP_INDEX_PATH).write_text(
        json.dumps(index_body, indent=2, ensure_ascii=False) + "\n"
    )


def _compact_outputs(manifest: dict) -> list[dict]:
    entries_by_path = _manifest_entries_by_path(manifest)
    resolve_memo: dict[str, list[str]] = {}

    def flat_ids(p: str) -> list[str]:
        return _resolve_requirement_ids(p, entries_by_path, resolve_memo, set())

    paths = sorted(entries_by_path.keys())
    path_to_ids = {p: tuple(flat_ids(p)) for p in paths}

    clusters: dict[tuple[str, ...], list[str]] = defaultdict(list)
    for p, t in path_to_ids.items():
        clusters[t].append(p)

    new_by_path: dict[str, dict] = {}
    for _key, members in clusters.items():
        canonical = min(members)
        for p in members:
            if p == canonical:
                new_by_path[p] = {
                    "path": p,
                    "version": entries_by_path[p]["version"],
                    "requirement_ids": list(path_to_ids[p]),
                }
            else:
                new_by_path[p] = {
                    "path": p,
                    "version": entries_by_path[p]["version"],
                    "extends": canonical,
                    "patches": [],
                }

    canon_paths = sorted(
        {min(v) for v in clusters.values()}, key=lambda p: (-len(path_to_ids[p]), p)
    )

    def try_remove_parent(
        child_path: str, child_ids: tuple[str, ...]
    ) -> tuple[str, list[dict]] | None:
        best: tuple[str, list[str], int] | None = None
        for parent_path, parent_ids in path_to_ids.items():
            if parent_path == child_path:
                continue
            if len(parent_ids) <= len(child_ids):
                continue
            removed = _remove_ids_to_get_subsequence(list(child_ids), list(parent_ids))
            if removed is None:
                continue
            score = len(parent_ids) - len(child_ids)
            candidate = (parent_path, removed, score)
            if best is None or score < best[2] or (score == best[2] and parent_path < best[0]):
                best = candidate
        if best is None:
            return None
        parent_path, removed, _s = best
        return parent_path, [{"remove_ids": removed}]

    for c in canon_paths:
        child_ids = path_to_ids[c]
        if c not in new_by_path or "extends" in new_by_path[c]:
            continue
        r = try_remove_parent(c, child_ids)
        if r:
            parent_path, patches = r
            new_by_path[c] = {
                "path": c,
                "version": entries_by_path[c]["version"],
                "extends": parent_path,
                "patches": patches,
            }

    return [new_by_path[p] for p in sorted(new_by_path.keys())]


def cmd_generate(tool_dir: Path, fragments_dir: Path, pilot_auto: Path, check: bool) -> int:
    manifest = _load_merged_manifest(tool_dir)
    entries_by_path = _manifest_entries_by_path(manifest)
    memo: dict[str, list[str]] = {}
    errors = 0
    for entry in manifest["outputs"]:
        rel = entry["path"]
        out_path = pilot_auto / rel
        try:
            ids = _resolve_requirement_ids(rel, entries_by_path, memo, set())
            version = _effective_version(entry, entries_by_path)
        except (ValueError, KeyError, FileNotFoundError) as ex:
            print(f"ERROR {rel}: {ex}", file=sys.stderr)
            errors += 1
            continue
        requirements = [_load_fragment(fragments_dir, rid) for rid in ids]
        text = _dump_requirement_set(version, requirements)
        if check:
            if not out_path.is_file():
                print(f"MISSING {out_path}", file=sys.stderr)
                errors += 1
                continue
            existing = out_path.read_text(encoding="utf-8")
            if existing != text:
                print(f"DRIFT {out_path}", file=sys.stderr)
                errors += 1
        else:
            out_path.parent.mkdir(parents=True, exist_ok=True)
            out_path.write_text(text, encoding="utf-8")
    return errors


def cmd_bootstrap(tool_dir: Path, fragments_dir: Path, pilot_auto: Path) -> None:
    if not pilot_auto.is_dir():
        raise SystemExit(f"pilot-auto directory not found: {pilot_auto}")

    id_to_req: dict[str, dict] = {}
    outputs: list[dict] = []

    for json_path in sorted(pilot_auto.rglob("*.json")):
        rel = json_path.relative_to(pilot_auto).as_posix()
        data = _load_json(json_path)
        version = data["version"]
        raw_ids: list[str] = []
        for req in data["requirements"]:
            rid = req["id"]
            raw_ids.append(rid)
            canonical = {"id": rid, "validators": req["validators"]}
            if rid in id_to_req:
                prev = json.dumps(id_to_req[rid], sort_keys=True)
                cur = json.dumps(canonical, sort_keys=True)
                if prev != cur:
                    raise SystemExit(
                        f"Requirement {rid!r} has conflicting definitions:\n  {json_path}\n"
                        f"  differs from earlier occurrence (bootstrap assumes one fragment per id)."
                    )
            else:
                id_to_req[rid] = canonical
        ids = _sort_requirement_ids(raw_ids)
        outputs.append({"path": rel, "version": version, "requirement_ids": ids})

    fragments_dir.mkdir(parents=True, exist_ok=True)
    for rid, req in sorted(id_to_req.items(), key=lambda x: x[0]):
        fp = _fragment_path(fragments_dir, rid)
        fragment_obj = {"id": rid, "validators": req["validators"]}
        fp.write_text(
            json.dumps(fragment_obj, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
        )

    _write_index_and_groups(tool_dir, outputs)
    print(
        f"Wrote {len(id_to_req)} fragments, index.json, and {len(GROUP_FILE_BY_ID)} group files "
        f"with {len(outputs)} outputs -> {tool_dir}"
    )


def cmd_compact(tool_dir: Path) -> None:
    merged = _load_merged_manifest(tool_dir)
    new_outputs = _compact_outputs(merged)
    _write_index_and_groups(tool_dir, new_outputs)
    roots = sum(1 for o in new_outputs if "requirement_ids" in o)
    chains = sum(1 for o in new_outputs if "extends" in o)
    print(f"Compacted group manifests -> {roots} roots, {chains} derived rows under {tool_dir}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--tool-dir",
        type=Path,
        default=None,
        help="Directory containing index.json, groups/, fragments/ (default: this script's directory)",
    )
    parser.add_argument(
        "--pilot-auto",
        type=Path,
        default=None,
        help="Path to map_requirements/pilot-auto (default: under autoware_lanelet2_map_validator)",
    )
    parser.add_argument(
        "--fragments",
        type=Path,
        default=None,
        help="Path to fragments directory (default: <tool-dir>/fragments)",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Verify generated JSON matches files on disk; exit 1 if not",
    )
    parser.add_argument(
        "command",
        nargs="?",
        default="generate",
        choices=("generate", "bootstrap", "compact"),
        help="generate (default); bootstrap builds index+groups+fragments; compact deduplicates extends",
    )
    args = parser.parse_args()

    tool_dir = (args.tool_dir or _tool_dir()).resolve()
    pilot_auto = (args.pilot_auto or _default_pilot_auto()).resolve()
    fragments_dir = (args.fragments or tool_dir / "fragments").resolve()

    if args.command == "bootstrap":
        cmd_bootstrap(tool_dir, fragments_dir, pilot_auto)
        return 0

    if args.command == "compact":
        try:
            cmd_compact(tool_dir)
        except FileNotFoundError as e:
            print(e, file=sys.stderr)
            return 1
        return 0

    try:
        err = cmd_generate(tool_dir, fragments_dir, pilot_auto, args.check)
    except FileNotFoundError as e:
        print(e, file=sys.stderr)
        return 1
    return 1 if err else 0


if __name__ == "__main__":
    raise SystemExit(main())
