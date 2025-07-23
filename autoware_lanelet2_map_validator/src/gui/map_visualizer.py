#!/usr/bin/env python3

# Copyright 2025 Autoware Foundation
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

"""
Map visualizer widget using matplotlib and lanelet2.
"""
import matplotlib
matplotlib.use('Agg')
from matplotlib.figure import Figure

from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, 
    QSizePolicy, QSlider, QPushButton
)
from PySide6.QtCore import Qt, Signal

# Import lanelet2 components
from lanelet2.io import load

import sys
import os
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

from matplotlib_widget import MatplotlibWidget
from gui_helper import create_appropriate_projector


class MapVisualizerWidget(QWidget):
    """Map visualizer widget using matplotlib and lanelet2"""
    
    element_clicked = Signal(str, str)  # Signal for when a map element is clicked (element_type, element_id)
    
    def __init__(self):
        super().__init__()
        self.lanelet_map = None
        self.current_file = None
        self._highlight_artists = []  # For fast blitting
        self._background = None  # Background for fast blitting
        self.setup_ui()
        
    def setup_ui(self):
        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)  # Remove margins from main layout
        layout.setSpacing(0)  # Remove spacing
        
        # Control panel
        control_panel = QHBoxLayout()
        
        self.map_info_label = QLabel("No map loaded")
        control_panel.addWidget(self.map_info_label)
        
        control_panel.addStretch()
        
        # Zoom control section
        zoom_layout = QHBoxLayout()
        zoom_layout.setSpacing(5)
        
        zoom_label = QLabel("Zoom:")
        zoom_layout.addWidget(zoom_label)
        
        # Zoom slider (10% to 500%)
        self.zoom_slider = QSlider(Qt.Orientation.Horizontal)
        self.zoom_slider.setMinimum(10)  # 10%
        self.zoom_slider.setMaximum(500)  # 500%
        self.zoom_slider.setValue(100)  # 100% default
        self.zoom_slider.setFixedWidth(120)
        self.zoom_slider.setToolTip("Drag to set zoom level (10% - 500%)")
        zoom_layout.addWidget(self.zoom_slider)
        
        # Zoom percentage label
        self.zoom_label = QLabel("100%")
        self.zoom_label.setFixedWidth(35)
        self.zoom_label.setStyleSheet("font-weight: bold;")
        zoom_layout.addWidget(self.zoom_label)
        
        control_panel.addLayout(zoom_layout)
        
        # Instructions label
        instructions_label = QLabel("Mouse: Drag to pan • Scroll to zoom • Double-click to auto-fit")
        instructions_label.setStyleSheet("color: gray; font-size: 10px;")
        control_panel.addWidget(instructions_label)
        
        # Reset view button
        self.reset_view_button = QPushButton("Reset View")
        self.reset_view_button.clicked.connect(self.reset_view)
        self.reset_view_button.setEnabled(False)
        control_panel.addWidget(self.reset_view_button)
        
        layout.addLayout(control_panel)
        
        # Map display - create matplotlib widget with dynamic sizing
        # Start with a reasonable default size, will be resized in resizeEvent
        self.figure = Figure(figsize=(8, 6), facecolor='white', dpi=100)
        self.figure.patch.set_facecolor('white')
        
        # Configure figure to fill entire space with no margins
        self.figure.subplots_adjust(left=0, right=1, top=1, bottom=0, wspace=0, hspace=0)
        
        # Set up canvas with matplotlib backend
        self.canvas = MatplotlibWidget(self.figure)
        self.ax = self.figure.add_subplot(111, facecolor='white')
        self.ax.set_aspect('equal')
        self.ax.grid(True, alpha=0.3)
        
        # Make axes fill entire figure
        self.ax.set_position([0, 0, 1, 1])
        
        # Give the canvas access to the axes for zoom/pan functionality
        self.canvas.ax = self.ax
        self.canvas.set_axes(self.ax)  # Set axes reference in our custom widget
        
        # Connect zoom signals
        self.canvas.zoomChanged.connect(self.update_zoom_display)
        self.zoom_slider.valueChanged.connect(self.on_zoom_slider_changed)
        
        # Remove axis frame and ticks for a cleaner look
        self.ax.spines['top'].set_visible(False)
        self.ax.spines['right'].set_visible(False)
        self.ax.spines['bottom'].set_visible(False)
        self.ax.spines['left'].set_visible(False)
        self.ax.set_xticks([])
        self.ax.set_yticks([])
        
        # Make the canvas expand to fill all available space with no margins
        self.canvas.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        self.canvas.setMinimumSize(600, 400)
        
        # Set canvas layout margins to zero
        self.canvas.setContentsMargins(0, 0, 0, 0)
        
        layout.addWidget(self.canvas)
        
        # Ensure the widget itself has no style margins
        self.setStyleSheet("QWidget { margin: 0px; padding: 0px; }")
        self.setLayout(layout)
        
    def resizeEvent(self, event):
        """Handle resize events to update the figure to fill the widget space"""
        super().resizeEvent(event)
        if event.size().isValid() and hasattr(self, 'canvas'):
            # Get the canvas widget size
            canvas_size = self.canvas.size()
            if canvas_size.width() > 0 and canvas_size.height() > 0:
                # Update figure size to match canvas size exactly
                fig_width = max(canvas_size.width() / 100.0, 1.0)  # Convert to inches (100 DPI)
                fig_height = max(canvas_size.height() / 100.0, 1.0)
                
                try:
                    self.figure.set_size_inches(fig_width, fig_height, forward=False)
                    self.figure.set_dpi(100)
                    # Ensure figure fills entire space
                    self.figure.subplots_adjust(left=0, right=1, top=1, bottom=0, wspace=0, hspace=0)
                    if hasattr(self, 'ax') and self.ax:
                        self.ax.set_position([0, 0, 1, 1])
                    # Trigger a redraw
                    if hasattr(self, 'lanelet_map') and self.lanelet_map:
                        self.canvas.draw_idle()
                except Exception as e:
                    print(f"Error resizing figure in MapVisualizerWidget: {e}")
        
    def update_zoom_display(self, zoom_percent):
        """Update the zoom slider and label when zoom changes"""
        # Update label
        self.zoom_label.setText(f"{zoom_percent}%")
        
        # Update slider (temporarily disconnect to avoid recursion)
        self.zoom_slider.blockSignals(True)
        self.zoom_slider.setValue(zoom_percent)
        self.zoom_slider.blockSignals(False)
        
    def on_zoom_slider_changed(self, value):
        """Handle zoom slider value changes"""
        if self.lanelet_map and hasattr(self.canvas, '_set_zoom_level'):
            self.canvas._set_zoom_level(value)
            self.zoom_label.setText(f"{value}%")
        
    def load_map_file(self, file_path: str, projector_type: str = "utm"):
        """Load a specific OSM map file with given projector type"""
        try:
            # Load the map using lanelet2 with appropriate projector
            projector = create_appropriate_projector(file_path, projector_type)
            self.lanelet_map = load(file_path, projector)
            self.current_file = file_path
            
            # Update UI with autoware extension info
            self.map_info_label.setText(f"Loaded: {os.path.basename(file_path)} (with Autoware extension)")
            
            # Update UI
            self.reset_view_button.setEnabled(True)
            
            # Visualize the map
            self.visualize_map()
            
        except Exception as e:
            error_msg = str(e)
            print(f"Error loading map: {e}")
            self.map_info_label.setText(f"Error loading map: {str(e)}")
            
            self.ax.clear()
            self.ax.text(0.5, 0.5, f"Failed to load map:\n\n{str(e)}\n\nPlease check that your map file is valid and\nthat all required dependencies are installed.", 
                       transform=self.ax.transAxes, ha='center', va='center', 
                       fontsize=12, color='red', wrap=True)
            self.ax.set_xlim(0, 1)
            self.ax.set_ylim(0, 1)
            self.canvas.draw()
                
    def visualize_map(self):
        """Visualize the loaded lanelet map"""
        if not self.lanelet_map:
            return
            
        self.ax.clear()
        self.ax.set_aspect('equal')
        self.ax.grid(True, alpha=0.3)
        
        # Configure figure and axes to fill entire space
        self.figure.patch.set_facecolor('white')
        self.ax.set_facecolor('white')
        self.figure.subplots_adjust(left=0, right=1, top=1, bottom=0, wspace=0, hspace=0)
        self.ax.set_position([0, 0, 1, 1])
        
        # Remove axes frame and ticks for a cleaner look
        self.ax.spines['top'].set_visible(False)
        self.ax.spines['right'].set_visible(False)
        self.ax.spines['bottom'].set_visible(False)
        self.ax.spines['left'].set_visible(False)
        self.ax.set_xticks([])
        self.ax.set_yticks([])
                
        for lanelet in self.lanelet_map.laneletLayer:
            self.draw_lanelet(lanelet)
            
        # Draw areas (polygons)
        for area in self.lanelet_map.areaLayer:
            self.draw_area(area)
        
        try:
            for polygon in self.lanelet_map.polygonLayer:
                self.draw_polygon(polygon)
        except Exception as e:
            print(f"Error iterating through polygonLayer: {e}")
            
        # Draw linestrings (for stop lines, etc.)
        for linestring in self.lanelet_map.lineStringLayer:
            self.draw_linestring(linestring)
            
        # Draw regulatory elements (if any visualization needed)
        for reg_elem in self.lanelet_map.regulatoryElementLayer:
            self.draw_regulatory_element(reg_elem)
            
        # Store the original view bounds for reset functionality
        self.original_xlim = self.ax.get_xlim()
        self.original_ylim = self.ax.get_ylim()
        
        # Save the background for fast blitting optimization
        self.canvas._save_background()
        
        # Reset zoom tracking to original view
        self.canvas._original_xlim = self.original_xlim
        self.canvas._original_ylim = self.original_ylim
        self.update_zoom_display(100)  # Reset to 100%
            
        # Render the figure to an image and display it
        self.update_map_display()
        
    def update_map_display(self):
        """Update the map display by drawing on the canvas"""
        try:
            # Configure figure to fill entire space
            self.figure.patch.set_facecolor('white')
            self.figure.subplots_adjust(left=0, right=1, top=1, bottom=0, wspace=0, hspace=0)
            self.ax.set_position([0, 0, 1, 1])
            
            # Draw the figure to the canvas
            self.canvas.draw()
        except Exception as e:
            print(f"Error updating map display: {e}")
            # Show error message in the map info label
            self.map_info_label.setText(f"Error displaying map: {str(e)}")
        
    def draw_lanelet(self, lanelet):
        """Draw a lanelet with its boundaries"""
        try:
            # Extract coordinates from left and right boundaries
            left_boundary = lanelet.leftBound
            right_boundary = lanelet.rightBound
            
            # Left boundary in blue
            left_x = [pt.x for pt in left_boundary]
            left_y = [pt.y for pt in left_boundary]
            self.ax.plot(left_x, left_y, 'b-', linewidth=1, alpha=0.8)
            
            # Right boundary in red
            right_x = [pt.x for pt in right_boundary]
            right_y = [pt.y for pt in right_boundary]
            self.ax.plot(right_x, right_y, 'r-', linewidth=1, alpha=0.8)
            
            # Centerline in green
            centerline = lanelet.centerline
            center_x = [pt.x for pt in centerline]
            center_y = [pt.y for pt in centerline]
            self.ax.plot(center_x, center_y, 'g--', linewidth=1, alpha=0.5)
            
        except Exception as e:
            print(f"Error drawing lanelet {lanelet.id}: {e}")
            
    def draw_area(self, area):
        """Draw an area polygon"""
        try:
            # Get the outer bound of the area
            outer_bounds = list(area.outerBound())
            if not outer_bounds:
                print(f"Area {area.id} has no outer bounds")
                return
                
            outer_bound = outer_bounds[0]  # Take the first one for simplicity
            
            # Extract coordinates
            x_coords = [pt.x for pt in outer_bound]
            y_coords = [pt.y for pt in outer_bound]
            
            # Close the polygon
            if x_coords and y_coords:
                if x_coords[0] != x_coords[-1] or y_coords[0] != y_coords[-1]:
                    x_coords.append(x_coords[0])
                    y_coords.append(y_coords[0])
                
                # Draw the area
                self.ax.fill(x_coords, y_coords, alpha=0.3, color='orange', edgecolor='red', linewidth=1)
                print(f"Drew area {area.id} with {len(x_coords)} points")
                           
        except Exception as e:
            print(f"Error drawing area {area.id}: {e}")
            
    def draw_polygon(self, polygon):
        """Draw a polygon from the polygon layer with a thinner color"""
        try:
            # Extract coordinates directly from the polygon
            # A Polygon3d object is directly iterable over its points
            x_coords = [pt.x for pt in polygon]
            y_coords = [pt.y for pt in polygon]
            
            if not x_coords or not y_coords:
                return
                
            # Close the polygon if not already closed
            if x_coords[0] != x_coords[-1] or y_coords[0] != y_coords[-1]:
                x_coords.append(x_coords[0])
                y_coords.append(y_coords[0])
            
            # Draw the polygon with alpha ~0.4 and a thin border
            self.ax.fill(x_coords, y_coords, alpha=0.4, color='lightblue', edgecolor='blue', linewidth=0.7)
            
        except Exception as e:
            print(f"Error drawing polygon {polygon.id}: {e}")
            
    def draw_linestring(self, linestring):
        """Draw a linestring (e.g., stop lines, traffic light lines)"""
        try:
            # Extract coordinates from the linestring
            x_coords = [pt.x for pt in linestring]
            y_coords = [pt.y for pt in linestring]
            
            if x_coords and y_coords:
                # Draw linestring with a distinct color (magenta for stop lines, etc.)
                self.ax.plot(x_coords, y_coords, 'm-', linewidth=2, alpha=0.8)          
                           
        except Exception as e:
            print(f"Error drawing linestring {linestring.id}: {e}")
            
    def draw_regulatory_element(self, reg_elem):
        """Draw regulatory element markers as a dot in the middle of the view"""
        try:
            xlim = self.ax.get_xlim()
            ylim = self.ax.get_ylim()
            
            position = None
            # If it's a traffic light, try to get its position
            if hasattr(reg_elem, 'trafficLights') and callable(getattr(reg_elem, 'trafficLights')):
                tl_refs = reg_elem.trafficLights()
                if tl_refs and len(tl_refs) > 0:
                    pt = tl_refs[0].centerline[0]
                    position = (pt.x, pt.y)
            # If it's a traffic sign
            elif hasattr(reg_elem, 'refLines') and callable(getattr(reg_elem, 'refLines')):
                ref_lines = reg_elem.refLines()
                if ref_lines and len(ref_lines) > 0:
                    pt = ref_lines[0].centerline[0]
                    position = (pt.x, pt.y)
            # For other types that have a stop line
            elif hasattr(reg_elem, 'stopLine') and callable(getattr(reg_elem, 'stopLine')):
                stop_line = reg_elem.stopLine()
                if stop_line:
                    pt = stop_line.centerline[0]
                    position = (pt.x, pt.y)
                
            if position is None:
                x = (xlim[0] + xlim[1]) / 2
                y = (ylim[0] + ylim[1]) / 2
                position = (x, y)
            
            # Draw a small dot to represent the regulatory element
            self.ax.plot(position[0], position[1], 'bo', markersize=4, alpha=0.7)
            
        except Exception as e:
            # Just log the error, no need to raise an exception
            print(f"Error drawing regulatory element {reg_elem.id}: {e}")
            
    def clear_highlights(self):
        """Clear all previous highlights using fast blitting"""
        if self._highlight_artists:
            # Remove highlight artists
            for artist in self._highlight_artists:
                if artist in self.ax.get_children():
                    artist.remove()
            
            self._highlight_artists.clear()
            
            # Use fast update if background is available
            if self._background:
                self.canvas.restore_region(self._background)
                self.canvas.blit(self.ax.bbox)
                self._update_display_fast()
    
    def highlight_element(self, element_type: str, element_id: str):
        """Highlight a specific map element"""
        if not self.lanelet_map:
            return
            
        try:
            # Clear any previous highlights first
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
        """Highlight a specific lanelet using fast blitting"""
        try:
            left_bound = lanelet.leftBound
            right_bound = lanelet.rightBound
            
            left_x = [pt.x for pt in left_bound]
            left_y = [pt.y for pt in left_bound]
            right_x = [pt.x for pt in right_bound]
            right_y = [pt.y for pt in right_bound]
            
            # Draw highlighted boundaries
            left_line = self.ax.plot(left_x, left_y, 'r-', linewidth=3, alpha=0.8, label=f'Lanelet {lanelet.id}')[0]
            right_line = self.ax.plot(right_x, right_y, 'r-', linewidth=3, alpha=0.8)[0]
            
            # Store the artists for later removal
            self._highlight_artists.extend([left_line, right_line])
            
            # Add text label at the center of the lanelet
            if left_x and left_y and right_x and right_y:
                # Calculate center point
                center_x = (sum(left_x) + sum(right_x)) / (len(left_x) + len(right_x))
                center_y = (sum(left_y) + sum(right_y)) / (len(left_y) + len(right_y))
                
                # Add ID text
                text = self.ax.text(center_x, center_y, f'Lanelet {lanelet.id}', 
                           fontsize=12, ha='center', va='center',
                           bbox=dict(boxstyle="round,pad=0.3", facecolor='red', alpha=0.8, edgecolor='darkred'),
                           color='white', weight='bold')
                
                # Store the text artist for later removal
                self._highlight_artists.append(text)
            
            # Use fast blitting update instead of full redraw
            self._fast_highlight_update()
                
        except Exception as e:
            print(f"Error highlighting lanelet: {e}")
            # Fall back to full redraw
            self.canvas.draw()
            
    def highlight_area(self, area):
        """Highlight a specific area using fast blitting"""
        try:
            outer_bound = area.outerBound
            x_coords = [pt.x for pt in outer_bound]
            y_coords = [pt.y for pt in outer_bound]
            
            if x_coords and y_coords:
                x_coords.append(x_coords[0])
                y_coords.append(y_coords[0])
                
                # Draw highlighted area
                polygon = self.ax.fill(x_coords, y_coords, alpha=0.6, color='red', edgecolor='darkred', linewidth=3, label=f'Area {area.id}')[0]
                
                # Store the artist for later removal
                self._highlight_artists.append(polygon)
                
                # Add text label at the center of the area
                centroid_x = sum(x_coords[:-1]) / len(x_coords[:-1])
                centroid_y = sum(y_coords[:-1]) / len(y_coords[:-1])
                text = self.ax.text(centroid_x, centroid_y, f'Area {area.id}', 
                           fontsize=12, ha='center', va='center',
                           bbox=dict(boxstyle="round,pad=0.3", facecolor='red', alpha=0.8, edgecolor='darkred'),
                           color='white', weight='bold')
                
                # Store the text artist for later removal
                self._highlight_artists.append(text)
                
                # Use fast blitting update instead of full redraw
                self._fast_highlight_update()
                
        except Exception as e:
            print(f"Error highlighting area: {e}")
            # Fall back to full redraw
            self.canvas.draw()
            
    def highlight_regulatory_element(self, reg_elem):
        """Highlight a regulatory element with a dot and text label using fast blitting"""
        try:
            # Get the current view bounds
            xlim = self.ax.get_xlim()
            ylim = self.ax.get_ylim()
            
            # Try to get a position associated with the regulatory element
            position = None
            
            try:
                # If it's a traffic light, try to get its position
                if hasattr(reg_elem, 'trafficLights') and callable(getattr(reg_elem, 'trafficLights')):
                    tl_refs = reg_elem.trafficLights()
                    if tl_refs and len(tl_refs) > 0:
                        pt = tl_refs[0].centerline[0]
                        position = (pt.x, pt.y)
                # If it's a traffic sign
                elif hasattr(reg_elem, 'refLines') and callable(getattr(reg_elem, 'refLines')):
                    ref_lines = reg_elem.refLines()
                    if ref_lines and len(ref_lines) > 0:
                        pt = ref_lines[0].centerline[0]
                        position = (pt.x, pt.y)
                # For other types that have a stop line
                elif hasattr(reg_elem, 'stopLine') and callable(getattr(reg_elem, 'stopLine')):
                    stop_line = reg_elem.stopLine()
                    if stop_line:
                        pt = stop_line.centerline[0]
                        position = (pt.x, pt.y)
            except Exception:
                pass
                
            if position is None:
                x = (xlim[0] + xlim[1]) / 2
                y = (ylim[0] + ylim[1]) / 2
                position = (x, y)
            
            marker = self.ax.plot(position[0], position[1], 'ro', markersize=10, alpha=0.9)[0]
            
            # Add a text label
            text = self.ax.text(position[0], position[1] + 2, f'RegElem {reg_elem.id}', 
                       fontsize=12, ha='center', va='bottom',
                       bbox=dict(boxstyle="round,pad=0.3", facecolor='red', alpha=0.8, edgecolor='darkred'),
                       color='white', weight='bold')
            
            # Store the artists for later removal
            self._highlight_artists.extend([marker, text])
            
            # Use fast blitting update instead of full redraw
            self._fast_highlight_update()
            
        except Exception as e:
            print(f"Error highlighting regulatory element: {e}")
            # Fall back to full redraw
            self.canvas.draw()
        
    def highlight_linestring(self, linestring):
        """Highlight a specific linestring using fast blitting"""
        try:
            print(f"Highlighting linestring {linestring.id}")
            
            # Extract coordinates from the linestring
            x_coords = [pt.x for pt in linestring]
            y_coords = [pt.y for pt in linestring]
            
            if x_coords and y_coords:
                # Draw highlighted linestring with thick red line
                line = self.ax.plot(x_coords, y_coords, 'r-', linewidth=4, alpha=0.9, label=f'LineString {linestring.id}')[0]
                
                # Add markers at endpoints
                start_marker = self.ax.plot(x_coords[0], y_coords[0], 'ro', markersize=8, alpha=0.9)[0]  # Start point
                end_marker = self.ax.plot(x_coords[-1], y_coords[-1], 'rs', markersize=8, alpha=0.9)[0]  # End point
                
                # Store the artists for later removal
                self._highlight_artists.extend([line, start_marker, end_marker])
                
                # Add ID text at the middle of the linestring
                mid_idx = len(x_coords) // 2
                text = self.ax.text(x_coords[mid_idx], y_coords[mid_idx], 
                           f'LS {linestring.id}', fontsize=10, ha='center', va='center',
                           bbox=dict(boxstyle="round,pad=0.3", facecolor='red', alpha=0.8, edgecolor='darkred'),
                           color='white', weight='bold')
                
                # Store the text artist for later removal
                self._highlight_artists.append(text)
                
                # Use fast blitting update instead of full redraw
                self._fast_highlight_update()
                
        except Exception as e:
            print(f"Error highlighting linestring: {e}")
            # Fall back to full redraw
            self.canvas.draw()
            
    def highlight_point(self, point):
        """Highlight a specific point using fast blitting"""
        try:
            print(f"Highlighting point {point.id}")
            
            # Draw highlighted point with large red marker
            marker = self.ax.plot(point.x, point.y, 'ro', markersize=12, alpha=0.9, label=f'Point {point.id}')[0]
            
            # Store the artist for later removal
            self._highlight_artists.append(marker)
            
            # Add text label for point ID
            text = self.ax.text(point.x, point.y, f'Point {point.id}', 
                       fontsize=10, ha='center', va='bottom',
                       bbox=dict(boxstyle="round,pad=0.3", facecolor='red', alpha=0.8, edgecolor='darkred'),
                       color='white', weight='bold')
            
            # Store the text artist for later removal
            self._highlight_artists.append(text)
            
            # Use fast blitting update instead of full redraw
            self._fast_highlight_update()
            
        except Exception as e:
            print(f"Error highlighting point: {e}")
            # Fall back to full redraw
            self.canvas.draw()
            
    def highlight_polygon_by_id(self, polygon_id):
        """Highlight a polygon by its ID"""
        try:
            polygon = self.lanelet_map.polygonLayer.get(polygon_id)
            if polygon:
                self.highlight_polygon(polygon)
            else:
                # If not found in polygon layer, try the area layer as fallback
                area = self.lanelet_map.areaLayer.get(polygon_id)
                if area:
                    self.highlight_area(area)
                else:
                    print(f"Polygon with ID {polygon_id} not found")
        except Exception as e:
            print(f"Error highlighting polygon by ID {polygon_id}: {e}")
            
    def highlight_polygon(self, polygon):
        """Highlight a specific polygon from the polygon layer using fast blitting"""
        try:
            # Clear previous highlights
            self.clear_highlights()
                
            # Extract coordinates directly from the polygon (it's iterable over its points)
            x_coords = [pt.x for pt in polygon]
            y_coords = [pt.y for pt in polygon]
            
            if not x_coords or not y_coords:
                print(f"Polygon {polygon.id} has no points")
                return
            
            # Close the polygon if not already closed
            if x_coords[0] != x_coords[-1] or y_coords[0] != y_coords[-1]:
                x_coords.append(x_coords[0])
                y_coords.append(y_coords[0])
                
            # Draw highlighted polygon with thick red edge and semi-transparent fill
            poly = self.ax.fill(x_coords, y_coords, alpha=0.6, color='red', edgecolor='darkred', linewidth=2)[0]
            
            # Store the artist for later removal
            self._highlight_artists.append(poly)
            
            # Add text label at the center of the polygon
            center_x = sum(x_coords) / len(x_coords)
            center_y = sum(y_coords) / len(y_coords)
            
            text = self.ax.text(center_x, center_y, f'Polygon {polygon.id}', 
                       fontsize=12, ha='center', va='center',
                       bbox=dict(boxstyle="round,pad=0.3", facecolor='red', alpha=0.8, edgecolor='darkred'),
                       color='white', weight='bold')
            
            # Store the text artist for later removal
            self._highlight_artists.append(text)
            
            # Use fast blitting update instead of full redraw
            self._fast_highlight_update()
            
        except Exception as e:
            print(f"Error highlighting polygon: {e}")
            # Fall back to full redraw
            self.canvas.draw()
            
    def reset_view(self):
        """Reset the view to show the entire map"""
        if self.lanelet_map:
            if hasattr(self, 'original_xlim') and hasattr(self, 'original_ylim'):
                # Use stored original bounds
                self.ax.set_xlim(self.original_xlim)
                self.ax.set_ylim(self.original_ylim)
                self.canvas.draw()
                # Reset zoom display to 100%
                self.update_zoom_display(100)
            else:
                # Fall back to re-visualizing the entire map
                self.visualize_map()
        
    def _fast_highlight_update(self):
        """Fast update using blitting - only redraws highlight elements"""
        try:
            if self._background and hasattr(self, 'ax') and self.ax:
                # Restore the background
                self.canvas.restore_region(self._background)
                
                # Draw only the highlight artists
                for artist in self._highlight_artists:
                    self.ax.draw_artist(artist)
                
                # Blit the changes
                self.canvas.blit(self.ax.bbox)
                
                # Update the Qt display
                self._update_display_fast()
            else:
                # Fall back to full redraw if no background
                self.canvas.draw()
        except Exception as e:
            print(f"Error in fast highlight update: {e}")
            # Fall back to full redraw
            self.canvas.draw()
            
    def _update_display_fast(self):
        """Fast display update - delegates to canvas"""
        try:
            self.canvas._update_display_fast()
        except Exception as e:
            print(f"Error in fast display update: {e}")
            self.canvas.draw()

    def jump_to_element(self, element_type: str, element_id: str):
        """Jump to and highlight a specific element"""
        self.highlight_element(element_type, element_id)

