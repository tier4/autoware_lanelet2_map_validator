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

"""Map visualizer widget using matplotlib and lanelet2."""
import os
import sys

from PySide6.QtCore import Qt
from PySide6.QtCore import Signal
from PySide6.QtWidgets import QHBoxLayout
from PySide6.QtWidgets import QLabel
from PySide6.QtWidgets import QPushButton
from PySide6.QtWidgets import QSizePolicy
from PySide6.QtWidgets import QSlider
from PySide6.QtWidgets import QVBoxLayout
from PySide6.QtWidgets import QWidget
from gui_helper import create_appropriate_projector
from lanelet2.io import load
import matplotlib
from matplotlib.figure import Figure
from matplotlib_widget import MatplotlibWidget

matplotlib.use("Agg")


# Ensure local modules are importable
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)


class MapVisualizerWidget(QWidget):
    """Map visualizer widget using matplotlib and lanelet2."""

    element_clicked = Signal(str, str)

    def __init__(self):
        super().__init__()
        self.lanelet_map = None
        self.current_file = None
        self.highlight_artists = []
        self.background = None
        self.setup_ui()

    def setup_ui(self):
        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        control_panel = QHBoxLayout()

        self.map_info_label = QLabel("No map loaded")
        control_panel.addWidget(self.map_info_label)

        control_panel.addStretch()

        zoom_layout = QHBoxLayout()
        zoom_layout.setSpacing(5)

        zoom_label = QLabel("Zoom:")
        zoom_layout.addWidget(zoom_label)

        self.zoom_slider = QSlider(Qt.Orientation.Horizontal)
        self.zoom_slider.setMinimum(10)
        self.zoom_slider.setMaximum(1500)
        self.zoom_slider.setValue(100)
        self.zoom_slider.setFixedWidth(120)
        self.zoom_slider.setToolTip("Drag to set zoom level (10% - 1500%)")
        zoom_layout.addWidget(self.zoom_slider)

        self.zoom_label = QLabel("100%")
        self.zoom_label.setFixedWidth(35)
        self.zoom_label.setStyleSheet("font-weight: bold;")
        zoom_layout.addWidget(self.zoom_label)

        control_panel.addLayout(zoom_layout)

        instructions_label = QLabel(
            "Mouse: Drag to pan • Scroll to zoom • Double-click to auto-fit"
        )
        instructions_label.setStyleSheet("color: gray; font-size: 10px;")
        control_panel.addWidget(instructions_label)

        self.reset_view_button = QPushButton("Reset View")
        self.reset_view_button.clicked.connect(self.reset_view)
        self.reset_view_button.setEnabled(False)
        control_panel.addWidget(self.reset_view_button)

        layout.addLayout(control_panel)

        self.figure = Figure(figsize=(8, 6), facecolor="white", dpi=100)
        self.figure.patch.set_facecolor("white")

        self.figure.subplots_adjust(left=0, right=1, top=1, bottom=0, wspace=0, hspace=0)

        self.canvas = MatplotlibWidget(self.figure)
        self.ax = self.figure.add_subplot(111, facecolor="white")
        self.ax.set_aspect("equal")
        self.ax.grid(True, alpha=0.3)

        self.ax.set_position([0, 0, 1, 1])

        self.canvas.ax = self.ax
        self.canvas.set_axes(self.ax)

        self.canvas.zoomChanged.connect(self.update_zoom_display)
        self.zoom_slider.valueChanged.connect(self.on_zoom_slider_changed)

        self.ax.spines["top"].set_visible(False)
        self.ax.spines["right"].set_visible(False)
        self.ax.spines["bottom"].set_visible(False)
        self.ax.spines["left"].set_visible(False)
        self.ax.set_xticks([])
        self.ax.set_yticks([])

        self.canvas.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        self.canvas.setMinimumSize(600, 400)

        self.canvas.setContentsMargins(0, 0, 0, 0)

        layout.addWidget(self.canvas)

        self.setStyleSheet("QWidget { margin: 0px; padding: 0px; }")
        self.setLayout(layout)

    def resizeEvent(self, event):
        """Handle resize events to update the figure to fill the widget space."""
        super().resizeEvent(event)
        if event.size().isValid() and hasattr(self, "canvas"):
            # Get the canvas widget size
            canvas_size = self.canvas.size()
            if canvas_size.width() > 0 and canvas_size.height() > 0:
                # Convert to inches (100 DPI)
                fig_width = max(canvas_size.width() / 100.0, 1.0)
                fig_height = max(canvas_size.height() / 100.0, 1.0)

                try:
                    self.figure.set_size_inches(fig_width, fig_height, forward=False)
                    self.figure.set_dpi(100)
                    # Ensure figure fills entire space
                    self.figure.subplots_adjust(
                        left=0, right=1, top=1, bottom=0, wspace=0, hspace=0
                    )
                    if hasattr(self, "ax") and self.ax:
                        self.ax.set_position([0, 0, 1, 1])
                    # Trigger a redraw
                    if hasattr(self, "lanelet_map") and self.lanelet_map:
                        self.canvas.draw_idle()
                except Exception as e:
                    print(f"Error resizing figure in MapVisualizerWidget: {e}")

    def update_zoom_display(self, zoom_percent):
        """Update the zoom slider and label when zoom changes."""
        self.zoom_label.setText(f"{zoom_percent}%")

        self.zoom_slider.blockSignals(True)
        self.zoom_slider.setValue(zoom_percent)
        self.zoom_slider.blockSignals(False)

    def on_zoom_slider_changed(self, value):
        """Handle zoom slider value changes."""
        if self.lanelet_map and hasattr(self.canvas, "set_zoom_level"):
            self.canvas.set_zoom_level(value)
            self.zoom_label.setText(f"{value}%")

    def load_map_file(self, file_path: str, projector_type: str = "utm"):
        """Load a specific OSM map file with given projector type."""
        try:
            projector = create_appropriate_projector(file_path, projector_type)
            self.lanelet_map = load(file_path, projector)
            self.current_file = file_path

            self.map_info_label.setText(
                f"Loaded: {os.path.basename(file_path)} (with Autoware extension)"
            )

            self.reset_view_button.setEnabled(True)

            self.visualize_map()

        except Exception as e:
            print(f"Error loading map: {e}")
            self.map_info_label.setText(f"Error loading map: {str(e)}")

            self.ax.clear()
            self.ax.text(
                0.5,
                0.5,
                f"Failed to load map:\n\n{str(e)}\n\nPlease check that your "
                f"map file is valid and\nthat all required dependencies are "
                f"installed.",
                transform=self.ax.transAxes,
                ha="center",
                va="center",
                fontsize=12,
                color="red",
                wrap=True,
            )
            self.ax.set_xlim(0, 1)
            self.ax.set_ylim(0, 1)
            self.canvas.draw()

    def visualize_map(self):
        """Visualize the loaded lanelet map."""
        if not self.lanelet_map:
            return

        self.ax.clear()
        self.ax.set_aspect("equal")
        self.ax.grid(True, alpha=0.3)

        self.figure.patch.set_facecolor("white")
        self.ax.set_facecolor("white")
        self.figure.subplots_adjust(left=0, right=1, top=1, bottom=0, wspace=0, hspace=0)
        self.ax.set_position([0, 0, 1, 1])

        self.ax.spines["top"].set_visible(False)
        self.ax.spines["right"].set_visible(False)
        self.ax.spines["bottom"].set_visible(False)
        self.ax.spines["left"].set_visible(False)
        self.ax.set_xticks([])
        self.ax.set_yticks([])

        for lanelet in self.lanelet_map.laneletLayer:
            self.draw_lanelet(lanelet)

        for area in self.lanelet_map.areaLayer:
            self.draw_area(area)

        try:
            for polygon in self.lanelet_map.polygonLayer:
                self.draw_polygon(polygon)
        except Exception as e:
            print(f"Error iterating through polygonLayer: {e}")

        for linestring in self.lanelet_map.lineStringLayer:
            self.draw_linestring(linestring)

        for reg_elem in self.lanelet_map.regulatoryElementLayer:
            self.draw_regulatory_element(reg_elem)

        self.original_xlim = self.ax.get_xlim()
        self.original_ylim = self.ax.get_ylim()

        self.canvas.save_background()

        self.canvas._original_xlim = self.original_xlim
        self.canvas._original_ylim = self.original_ylim
        self.update_zoom_display(100)

        self.update_map_display()

    def update_map_display(self):
        """Update the map display by drawing on the canvas."""
        try:
            self.figure.patch.set_facecolor("white")
            self.figure.subplots_adjust(left=0, right=1, top=1, bottom=0, wspace=0, hspace=0)
            self.ax.set_position([0, 0, 1, 1])

            self.canvas.draw()
        except Exception as e:
            print(f"Error updating map display: {e}")
            self.map_info_label.setText(f"Error displaying map: {str(e)}")

    def draw_lanelet(self, lanelet):
        """Draw a lanelet with its boundaries."""
        try:
            left_boundary = lanelet.leftBound
            right_boundary = lanelet.rightBound

            left_x = [pt.x for pt in left_boundary]
            left_y = [pt.y for pt in left_boundary]
            self.ax.plot(left_x, left_y, "b-", linewidth=1, alpha=0.8)

            right_x = [pt.x for pt in right_boundary]
            right_y = [pt.y for pt in right_boundary]
            self.ax.plot(right_x, right_y, "r-", linewidth=1, alpha=0.8)

            centerline = lanelet.centerline
            center_x = [pt.x for pt in centerline]
            center_y = [pt.y for pt in centerline]
            self.ax.plot(center_x, center_y, "g--", linewidth=1, alpha=0.5)

        except Exception as e:
            print(f"Error drawing lanelet {lanelet.id}: {e}")

    def draw_area(self, area):
        """Draw an area polygon."""
        try:
            outer_bounds = list(area.outerBound())
            if not outer_bounds:
                print(f"Area {area.id} has no outer bounds")
                return

            outer_bound = outer_bounds[0]

            x_coords = [pt.x for pt in outer_bound]
            y_coords = [pt.y for pt in outer_bound]

            if x_coords and y_coords:
                if x_coords[0] != x_coords[-1] or y_coords[0] != y_coords[-1]:
                    x_coords.append(x_coords[0])
                    y_coords.append(y_coords[0])

                self.ax.fill(
                    x_coords, y_coords, alpha=0.3, color="orange", edgecolor="red", linewidth=1
                )
                print(f"Drew area {area.id} with {len(x_coords)} points")

        except Exception as e:
            print(f"Error drawing area {area.id}: {e}")

    def draw_polygon(self, polygon):
        """Draw a polygon from the polygon layer with a thinner color."""
        try:
            x_coords = [pt.x for pt in polygon]
            y_coords = [pt.y for pt in polygon]

            if not x_coords or not y_coords:
                return

            if x_coords[0] != x_coords[-1] or y_coords[0] != y_coords[-1]:
                x_coords.append(x_coords[0])
                y_coords.append(y_coords[0])

            self.ax.fill(
                x_coords, y_coords, alpha=0.4, color="lightblue", edgecolor="blue", linewidth=0.7
            )

        except Exception as e:
            print(f"Error drawing polygon {polygon.id}: {e}")

    def draw_linestring(self, linestring):
        """Draw a linestring (e.g., stop lines, traffic light lines)."""
        try:
            x_coords = [pt.x for pt in linestring]
            y_coords = [pt.y for pt in linestring]

            if x_coords and y_coords:
                self.ax.plot(x_coords, y_coords, "m-", linewidth=2, alpha=0.8)

        except Exception as e:
            print(f"Error drawing linestring {linestring.id}: {e}")

    def draw_regulatory_element(self, reg_elem):
        """Draw regulatory element markers as a dot in the middle of the view."""
        try:
            xlim = self.ax.get_xlim()
            ylim = self.ax.get_ylim()

            position = None
            if hasattr(reg_elem, "trafficLights") and callable(getattr(reg_elem, "trafficLights")):
                tl_refs = reg_elem.trafficLights()
                if tl_refs and len(tl_refs) > 0:
                    tl_ref = tl_refs[0]
                    if len(tl_ref) > 0:
                        pt = tl_ref[0]
                        position = (pt.x, pt.y)
            elif hasattr(reg_elem, "refLines") and callable(getattr(reg_elem, "refLines")):
                ref_lines = reg_elem.refLines()
                if ref_lines and len(ref_lines) > 0:
                    ref_line = ref_lines[0]
                    if hasattr(ref_line, "centerline"):
                        pt = ref_line.centerline[0]
                        position = (pt.x, pt.y)
                    elif len(ref_line) > 0:
                        pt = ref_line[0]
                        position = (pt.x, pt.y)
            elif hasattr(reg_elem, "stopLine") and callable(getattr(reg_elem, "stopLine")):
                stop_line = reg_elem.stopLine()
                if stop_line:
                    if len(stop_line) > 0:
                        pt = stop_line[0]
                        position = (pt.x, pt.y)

            if position is None:
                x = (xlim[0] + xlim[1]) / 2
                y = (ylim[0] + ylim[1]) / 2
                position = (x, y)

            self.ax.plot(position[0], position[1], "bo", markersize=4, alpha=0.7)

        except Exception as e:
            print(f"Error drawing regulatory element {reg_elem.id}: {e}")

    def clear_highlights(self):
        """Clear all previous highlights using fast blitting."""
        if self.highlight_artists:
            for artist in self.highlight_artists:
                if artist in self.ax.get_children():
                    artist.remove()

            self.highlight_artists.clear()

            if self.background:
                self.canvas.restore_region(self.background)
                self.canvas.blit(self.ax.bbox)
                self.update_display()

    def highlight_element(self, element_type: str, element_id: str):
        """Highlight a specific map element."""
        if not self.lanelet_map:
            return

        try:
            self.clear_highlights()

            element_id_int = int(element_id)

            if element_type.lower() == "lanelet":
                lanelet = self.lanelet_map.laneletLayer.get(element_id_int)
                if lanelet:
                    self.highlight_lanelet(lanelet)
            elif element_type.lower() == "area":
                area = self.lanelet_map.areaLayer.get(element_id_int)
                if area:
                    self.highlight_area(area)
            elif element_type.lower() == "polygon":
                self.highlight_polygon_by_id(element_id_int)
            elif element_type.lower() == "regulatoryelement":
                reg_elem = self.lanelet_map.regulatoryElementLayer.get(element_id_int)
                if reg_elem:
                    self.highlight_regulatory_element(reg_elem)
            elif element_type.lower() == "linestring":
                linestring = self.lanelet_map.lineStringLayer.get(element_id_int)
                if linestring:
                    self.highlight_linestring(linestring)
            elif element_type.lower() == "point":
                point = self.lanelet_map.pointLayer.get(element_id_int)
                if point:
                    self.highlight_point(point)

            self.update_map_display()

        except Exception as e:
            print(f"Error highlighting element {element_type} {element_id}: {e}")

    def highlight_lanelet(self, lanelet):
        """Highlight a specific lanelet using fast blitting."""
        try:
            left_bound = lanelet.leftBound
            right_bound = lanelet.rightBound

            left_x = [pt.x for pt in left_bound]
            left_y = [pt.y for pt in left_bound]
            right_x = [pt.x for pt in right_bound]
            right_y = [pt.y for pt in right_bound]

            left_line = self.ax.plot(
                left_x, left_y, "r-", linewidth=3, alpha=0.8, label=f"Lanelet {lanelet.id}"
            )[0]
            right_line = self.ax.plot(right_x, right_y, "r-", linewidth=3, alpha=0.8)[0]

            self.highlight_artists.extend([left_line, right_line])

            if left_x and left_y and right_x and right_y:
                center_x = (sum(left_x) + sum(right_x)) / (len(left_x) + len(right_x))
                center_y = (sum(left_y) + sum(right_y)) / (len(left_y) + len(right_y))

                text = self.ax.text(
                    center_x,
                    center_y,
                    f"Lanelet {lanelet.id}",
                    fontsize=12,
                    ha="center",
                    va="center",
                    bbox={
                        "boxstyle": "round,pad=0.3",
                        "facecolor": "red",
                        "alpha": 0.8,
                        "edgecolor": "darkred",
                    },
                    color="white",
                    weight="bold",
                )

                self.highlight_artists.append(text)

            self.highlight_update()

        except Exception as e:
            print(f"Error highlighting lanelet: {e}")
            self.canvas.draw()

    def highlight_area(self, area):
        """Highlight a specific area using fast blitting."""
        try:
            outer_bound = area.outerBound
            x_coords = [pt.x for pt in outer_bound]
            y_coords = [pt.y for pt in outer_bound]

            if x_coords and y_coords:
                x_coords.append(x_coords[0])
                y_coords.append(y_coords[0])

                polygon = self.ax.fill(
                    x_coords,
                    y_coords,
                    alpha=0.6,
                    color="red",
                    edgecolor="darkred",
                    linewidth=3,
                    label=f"Area {area.id}",
                )[0]

                self.highlight_artists.append(polygon)

                centroid_x = sum(x_coords[:-1]) / len(x_coords[:-1])
                centroid_y = sum(y_coords[:-1]) / len(y_coords[:-1])
                text = self.ax.text(
                    centroid_x,
                    centroid_y,
                    f"Area {area.id}",
                    fontsize=12,
                    ha="center",
                    va="center",
                    bbox={
                        "boxstyle": "round,pad=0.3",
                        "facecolor": "red",
                        "alpha": 0.8,
                        "edgecolor": "darkred",
                    },
                    color="white",
                    weight="bold",
                )

                self.highlight_artists.append(text)

                self.highlight_update()

        except Exception as e:
            print(f"Error highlighting area: {e}")
            self.canvas.draw()

    def highlight_regulatory_element(self, reg_elem):
        """Highlight a regulatory element with a dot and text label using fast blitting."""
        try:
            xlim = self.ax.get_xlim()
            ylim = self.ax.get_ylim()
            position = None

            try:
                if hasattr(reg_elem, "trafficLights") and callable(
                    getattr(reg_elem, "trafficLights")
                ):
                    tl_refs = reg_elem.trafficLights()
                    if tl_refs and len(tl_refs) > 0:
                        # Traffic light references are LineString objects
                        tl_ref = tl_refs[0]
                        if len(tl_ref) > 0:
                            pt = tl_ref[0]  # Get first point of the linestring
                            position = (pt.x, pt.y)
                elif hasattr(reg_elem, "refLines") and callable(getattr(reg_elem, "refLines")):
                    ref_lines = reg_elem.refLines()
                    if ref_lines and len(ref_lines) > 0:
                        ref_line = ref_lines[0]
                        if hasattr(ref_line, "centerline"):
                            # It's a Lanelet
                            pt = ref_line.centerline[0]
                            position = (pt.x, pt.y)
                        elif len(ref_line) > 0:
                            # It's a LineString
                            pt = ref_line[0]
                            position = (pt.x, pt.y)
                elif hasattr(reg_elem, "stopLine") and callable(getattr(reg_elem, "stopLine")):
                    stop_line = reg_elem.stopLine()
                    if stop_line and len(stop_line) > 0:
                        # Stop line is a LineString
                        pt = stop_line[0]  # Get first point of the linestring
                        position = (pt.x, pt.y)
            except Exception:
                pass

            if position is None:
                x = (xlim[0] + xlim[1]) / 2
                y = (ylim[0] + ylim[1]) / 2
                position = (x, y)

            marker = self.ax.plot(position[0], position[1], "ro", markersize=10, alpha=0.9)[0]

            text = self.ax.text(
                position[0],
                position[1] + 2,
                f"RegElem {reg_elem.id}",
                fontsize=12,
                ha="center",
                va="bottom",
                bbox={
                    "boxstyle": "round,pad=0.3",
                    "facecolor": "red",
                    "alpha": 0.8,
                    "edgecolor": "darkred",
                },
                color="white",
                weight="bold",
            )

            self.highlight_artists.extend([marker, text])

            self.highlight_update()

        except Exception as e:
            print(f"Error highlighting regulatory element: {e}")
            self.canvas.draw()

    def highlight_linestring(self, linestring):
        """Highlight a specific linestring using fast blitting."""
        try:
            print(f"Highlighting linestring {linestring.id}")
            x_coords = [pt.x for pt in linestring]
            y_coords = [pt.y for pt in linestring]

            if x_coords and y_coords:
                line = self.ax.plot(
                    x_coords,
                    y_coords,
                    "r-",
                    linewidth=4,
                    alpha=0.9,
                    label=f"LineString {linestring.id}",
                )[0]

                start_marker = self.ax.plot(
                    x_coords[0], y_coords[0], "ro", markersize=8, alpha=0.9
                )[0]
                end_marker = self.ax.plot(
                    x_coords[-1], y_coords[-1], "rs", markersize=8, alpha=0.9
                )[0]

                self.highlight_artists.extend([line, start_marker, end_marker])

                mid_idx = len(x_coords) // 2
                text = self.ax.text(
                    x_coords[mid_idx],
                    y_coords[mid_idx],
                    f"LS {linestring.id}",
                    fontsize=10,
                    ha="center",
                    va="center",
                    bbox={
                        "boxstyle": "round,pad=0.3",
                        "facecolor": "red",
                        "alpha": 0.8,
                        "edgecolor": "darkred",
                    },
                    color="white",
                    weight="bold",
                )

                self.highlight_artists.append(text)

                self.highlight_update()

        except Exception as e:
            print(f"Error highlighting linestring: {e}")
            self.canvas.draw()

    def highlight_point(self, point):
        """Highlight a specific point using fast blitting."""
        try:
            print(f"Highlighting point {point.id}")

            marker = self.ax.plot(
                point.x, point.y, "ro", markersize=12, alpha=0.9, label=f"Point {point.id}"
            )[0]

            self.highlight_artists.append(marker)

            text = self.ax.text(
                point.x,
                point.y,
                f"Point {point.id}",
                fontsize=10,
                ha="center",
                va="bottom",
                bbox={
                    "boxstyle": "round,pad=0.3",
                    "facecolor": "red",
                    "alpha": 0.8,
                    "edgecolor": "darkred",
                },
                color="white",
                weight="bold",
            )

            self.highlight_artists.append(text)

            self.highlight_update()

        except Exception as e:
            print(f"Error highlighting point: {e}")
            self.canvas.draw()

    def highlight_polygon_by_id(self, polygon_id):
        """Highlight a polygon by its ID."""
        try:
            polygon = self.lanelet_map.polygonLayer.get(polygon_id)
            if polygon:
                self.highlight_polygon(polygon)
            else:
                area = self.lanelet_map.areaLayer.get(polygon_id)
                if area:
                    self.highlight_area(area)
                else:
                    print(f"Polygon with ID {polygon_id} not found")
        except Exception as e:
            print(f"Error highlighting polygon by ID {polygon_id}: {e}")

    def highlight_polygon(self, polygon):
        """Highlight a specific polygon from the polygon layer using fast blitting."""
        try:
            self.clear_highlights()

            x_coords = [pt.x for pt in polygon]
            y_coords = [pt.y for pt in polygon]

            if not x_coords or not y_coords:
                print(f"Polygon {polygon.id} has no points")
                return

            if x_coords[0] != x_coords[-1] or y_coords[0] != y_coords[-1]:
                x_coords.append(x_coords[0])
                y_coords.append(y_coords[0])

            poly = self.ax.fill(
                x_coords, y_coords, alpha=0.6, color="red", edgecolor="darkred", linewidth=2
            )[0]

            self.highlight_artists.append(poly)

            center_x = sum(x_coords) / len(x_coords)
            center_y = sum(y_coords) / len(y_coords)

            text = self.ax.text(
                center_x,
                center_y,
                f"Polygon {polygon.id}",
                fontsize=12,
                ha="center",
                va="center",
                bbox={
                    "boxstyle": "round,pad=0.3",
                    "facecolor": "red",
                    "alpha": 0.8,
                    "edgecolor": "darkred",
                },
                color="white",
                weight="bold",
            )

            self.highlight_artists.append(text)

            self.highlight_update()

        except Exception as e:
            print(f"Error highlighting polygon: {e}")
            self.canvas.draw()

    def reset_view(self):
        """Reset the view to show the entire map."""
        if self.lanelet_map:
            if hasattr(self, "original_xlim") and hasattr(self, "original_ylim"):
                self.ax.set_xlim(self.original_xlim)
                self.ax.set_ylim(self.original_ylim)
                self.canvas.draw()
                self.update_zoom_display(100)
            else:
                self.visualize_map()

    def highlight_update(self):
        """Fast update using blitting - only redraws highlight elements."""
        try:
            if self.background and hasattr(self, "ax") and self.ax:
                self.canvas.restore_region(self.background)

                for artist in self.highlight_artists:
                    self.ax.draw_artist(artist)

                self.canvas.blit(self.ax.bbox)
                self.update_display()
            else:
                self.canvas.draw()
        except Exception as e:
            print(f"Error in fast highlight update: {e}")
            self.canvas.draw()

    def update_display(self):
        """Fast display update - delegates to canvas."""
        try:
            self.canvas.update_display()
        except Exception as e:
            print(f"Error in fast display update: {e}")
            self.canvas.draw()

    def jump_to_element(self, element_type: str, element_id: str):
        """Jump to and highlight a specific element."""
        self.highlight_element(element_type, element_id)
