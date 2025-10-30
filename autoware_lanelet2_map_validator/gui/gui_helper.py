#!/usr/bin/env python3

# Copyright 2025 TIER IV, Inc.
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

"""GUI helper functions and classes for the Lanelet2 map validator."""

from pathlib import Path
import subprocess
from typing import List
from typing import Tuple
import xml.etree.ElementTree as ET

from PySide6.QtCore import Signal
from PySide6.QtWidgets import QLineEdit
from autoware_lanelet2_extension_python.projection import MGRSProjector
from lanelet2.io import Origin
from lanelet2.projection import UtmProjector


class DropLineEdit(QLineEdit):
    """A QLineEdit that accepts file drops."""

    fileDropped = Signal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setAcceptDrops(True)

    def dragEnterEvent(self, event):
        """Handle drag enter events."""
        if event.mimeData().hasUrls():
            event.acceptProposedAction()

    def dropEvent(self, event):
        """Handle drop events."""
        urls = event.mimeData().urls()
        if urls:
            path = urls[0].toLocalFile()
            self.setText(path)
            self.fileDropped.emit(path)


def get_utm_zone_from_longitude(longitude: float) -> int:
    """Calculate UTM zone from longitude."""
    return int((longitude + 180) / 6) + 1


def get_map_bounds_from_osm(osm_file: str) -> Tuple[float, float, float, float]:
    """Extract bounding box from OSM file to determine appropriate UTM zone."""
    try:
        tree = ET.parse(osm_file)
        root = tree.getroot()

        lons = []
        lats = []

        # Extract coordinates from nodes
        for node in root.findall(".//node"):
            lat = float(node.get("lat"))
            lon = float(node.get("lon"))
            lats.append(lat)
            lons.append(lon)

        if lons and lats:
            return min(lons), min(lats), max(lons), max(lats)
        else:
            return None, None, None, None

    except Exception as e:
        print(f"Error parsing OSM file for bounds: {e}")
        return None, None, None, None


def create_appropriate_projector(osm_file: str, projector_type: str = "utm"):
    """Create an appropriate projector based on the map's location and selected projector type."""
    min_lon, min_lat, max_lon, max_lat = get_map_bounds_from_osm(osm_file)

    if min_lon is not None and max_lon is not None and min_lat is not None and max_lat is not None:
        # Use the center of the bounding box
        center_lon = (min_lon + max_lon) / 2
        center_lat = (min_lat + max_lat) / 2

        if projector_type == "mgrs":
            try:
                return MGRSProjector(Origin(center_lat, center_lon))
            except Exception:
                utm_zone = get_utm_zone_from_longitude(center_lon)
                return UtmProjector(Origin(center_lat, center_lon), utm_zone, center_lat >= 0)
        elif projector_type == "utm":
            utm_zone = get_utm_zone_from_longitude(center_lon)
            return UtmProjector(Origin(center_lat, center_lon), utm_zone, center_lat >= 0)
        elif projector_type == "transverse_mercator":
            utm_zone = get_utm_zone_from_longitude(center_lon)
            return UtmProjector(Origin(center_lat, center_lon), utm_zone, center_lat >= 0)
        else:
            utm_zone = get_utm_zone_from_longitude(center_lon)
            print(f"Defaulting to UTM zone {utm_zone}")
            return UtmProjector(Origin(center_lat, center_lon), utm_zone, center_lat >= 0)

    else:
        return UtmProjector(Origin(0, 0))


def run_lanelet2_validator(
    projector: str,
    map_file: Path,
    input_requirements: Path = None,
    exclusion_file: Path = None,
    parameters_path: Path = None,
    output_dir: Path = None,
    language: str = "en",
    validator_filter: str = "",
) -> Tuple[int, List[str], List[str]]:
    """Run the Lanelet2 validator with the specified parameters."""
    import os
    import sys

    if getattr(sys, "frozen", False):
        exe_dir = os.path.dirname(sys.executable)
    else:
        exe_dir = os.path.dirname(os.path.abspath(__file__))
    validator_exe = os.path.join(exe_dir, "autoware_lanelet2_map_validator")
    cmd = [
        validator_exe,
        "-p",
        projector,
        "-m",
        str(map_file),
        "-l",
        language,
    ]

    if exclusion_file:
        cmd += ["-x", str(exclusion_file)]
    if output_dir:
        cmd += ["-o", str(output_dir)]
    if input_requirements:
        cmd += ["-i", str(input_requirements)]
    if parameters_path:
        cmd += ["--parameters", str(parameters_path)]
    if validator_filter:
        cmd += ["-v", validator_filter]

    print(f"Debug: Running command: {' '.join(cmd)}")

    result = subprocess.run(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=False
    )

    out_lines = result.stdout.splitlines()
    err_lines = result.stderr.splitlines()
    return result.returncode, out_lines, err_lines


def get_available_validators() -> list:
    """Run the bundled validator executable with --print and return the list of available validators."""
    import os
    import sys

    if getattr(sys, "frozen", False):
        exe_dir = os.path.dirname(sys.executable)
    else:
        exe_dir = os.path.dirname(os.path.abspath(__file__))
    validator_exe = os.path.join(exe_dir, "autoware_lanelet2_map_validator")
    cmd = [validator_exe, "--print"]
    try:
        result = subprocess.run(
            cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True
        )
        out_lines = result.stdout.splitlines()
        validators = []
        for line in out_lines:
            line = line.strip()
            if not line:
                continue
            header = line.lower().rstrip(":")
            if header == "The following checks are available":
                continue
            if "." in line or "_" in line or line.islower():
                validators.append(line)
        return validators
    except Exception as e:
        print(f"Error fetching available validators: {e}")
        return []
