#!/usr/bin/env python3

import argparse
import json
import os


def parse_vm_id(vm_id: str):
    # Parse an id like 'vm-00-12' into (00, 12) as integers for sorting.
    try:
        _, first, second = vm_id.split("-")
        return int(first), int(second)
    except Exception:
        # fallback: push invalid IDs to the end
        return (9999, 9999)


def process_json_file(file_path, item_to_add, update: bool):
    with open(file_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    # Skip files without "requirements"
    if "requirements" not in data or not isinstance(data["requirements"], list):
        print(f"Skipping {file_path} (no 'requirements')")
        return

    target_id = item_to_add.get("id")
    replaced = False

    for i, req in enumerate(data["requirements"]):
        if isinstance(req, dict) and req.get("id") == target_id:
            if update:
                data["requirements"][i] = item_to_add
                replaced = True
                print(f"Updated id '{target_id}' in {file_path}")
            else:
                print(f"Skipping {file_path} (id '{target_id}' already exists)")
            break

    # If not found, append new item
    if not replaced and target_id not in [
        r.get("id") for r in data["requirements"] if isinstance(r, dict)
    ]:
        data["requirements"].append(item_to_add)
        print(f"Added new id '{target_id}' to {file_path}")

    # Sort by parsed vm-id
    data["requirements"].sort(key=lambda x: parse_vm_id(x.get("id", "")))

    # Write back
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)


def main():
    parser = argparse.ArgumentParser(
        description="Add an item (or items) into JSON 'requirements' fields."
    )
    parser.add_argument("--folder", required=True, help="Root folder to search for JSON files")
    parser.add_argument("--keyword", required=True, help="Keyword that must appear in the filename")
    parser.add_argument("--item", required=True, help="JSON file containing the item(s) to add")
    parser.add_argument(
        "--update", action="store_true", help="Update existing items with the same id"
    )

    args = parser.parse_args()

    # Load item(s) to add
    with open(args.item, "r", encoding="utf-8") as f:
        items = json.load(f)

    # Normalize to list
    if isinstance(items, dict):
        items = [items]

    # Walk through folder
    for root, _, files in os.walk(args.folder):
        for filename in files:
            if args.keyword in filename and filename.endswith(".json"):
                file_path = os.path.join(root, filename)
                for item in items:
                    process_json_file(file_path, item, args.update)


if __name__ == "__main__":
    main()
