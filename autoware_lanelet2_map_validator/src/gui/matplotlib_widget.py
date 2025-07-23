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
Custom matplotlib widget for PySide6 Essentials compatibility.
"""
import numpy as np
import matplotlib
matplotlib.use('Agg')
from matplotlib.figure import Figure
from matplotlib.backends.backend_agg import FigureCanvasAgg

from PySide6.QtWidgets import QWidget, QVBoxLayout, QLabel, QSizePolicy
from PySide6.QtGui import QPixmap, QImage
from PySide6.QtCore import Qt, Signal, QTimer


class MatplotlibWidget(QWidget):
    """Custom Qt widget to display matplotlib plots with both interactive and static support"""
    
    # Signal emitted when zoom level changes
    zoomChanged = Signal(int)  # Emits zoom percentage
    
    def __init__(self, figure=None, parent=None):
        super().__init__(parent)
        self.figure = figure or Figure()
        self.setMinimumSize(600, 400)
        
        # Set up layout with no margins or spacing
        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)  # Remove all margins
        layout.setSpacing(0)  # Remove spacing between widgets
        self.setLayout(layout)
        
        # Use static Agg backend with PySide6 Essentials
        self.canvas = FigureCanvasAgg(self.figure)
        
        # Image label to display the plot - fill entire widget
        self.image_label = QLabel()
        self.image_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.image_label.setStyleSheet("border: none; background-color: white;")
        self.image_label.setScaledContents(False)  # Don't scale to avoid aspect ratio issues
        self.image_label.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        layout.addWidget(self.image_label)
        
        # Timer for delayed updates
        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self._update_display)
        self.update_timer.setSingleShot(True)
        
        # For optimization: store background for blitting
        self._background = None
        self._highlight_artists = []  # Store highlight artists separately for fast removal
        
        # Zoom tracking
        self._original_xlim = None
        self._original_ylim = None
        self._current_zoom = 100  # Start at 100% (original view)
        
        # Setup the figure to fill the entire space
        self._setup_figure()
        
    def set_axes(self, ax):
        """Set the axes reference for zoom/pan functionality"""
        self.ax = ax
        self._setup_figure()
        
    def _setup_figure(self):
        """Setup the figure to fill the entire widget space"""
        # Configure figure to have no margins and fill space
        self.figure.patch.set_facecolor('white')
        self.figure.subplots_adjust(left=0, right=1, top=1, bottom=0, wspace=0, hspace=0)
        
        # If we have an axes, configure it
        if hasattr(self, 'ax') and self.ax:
            self.ax.set_position([0, 0, 1, 1])  # Fill entire figure
            self.ax.set_facecolor('white')
            
    def resizeEvent(self, event):
        """Handle widget resize to adjust figure size accordingly"""
        super().resizeEvent(event)
        if event.size().isValid():
            # Update figure size to match widget size exactly
            widget_size = event.size()
            # Convert widget size to inches (using 100 DPI)
            fig_width = max(widget_size.width() / 100.0, 1.0)
            fig_height = max(widget_size.height() / 100.0, 1.0)
            
            try:
                # Resize figure to exact widget dimensions
                self.figure.set_size_inches(fig_width, fig_height, forward=False)
                self.figure.set_dpi(100)  # Set consistent DPI
                self._setup_figure()  # Ensure figure still fills space
                if hasattr(self, 'ax') and self.ax:
                    self.draw_idle()
            except Exception as e:
                print(f"Error resizing figure: {e}")
        
    def draw(self):
        """Draw the matplotlib figure"""
        try:
            self.canvas.draw()
            self._save_background()  # Save background for fast blitting
            
            # Store original bounds if not already stored
            if hasattr(self, 'ax') and self.ax and self._original_xlim is None:
                self._original_xlim = self.ax.get_xlim()
                self._original_ylim = self.ax.get_ylim()
            
            self._update_display()
        except Exception as e:
            print(f"Error drawing matplotlib figure: {e}")
            
    def draw_idle(self):
        """Schedule a delayed draw"""
        if not self.update_timer.isActive():
            self.update_timer.start(50)  # 50ms delay
            
    def _save_background(self):
        """Save the current background for fast blitting"""
        try:
            if hasattr(self, 'ax') and self.ax:
                # Save background without any highlight artists
                for artist in self._highlight_artists:
                    artist.set_visible(False)
                
                self.canvas.draw()
                self._background = self.canvas.copy_from_bbox(self.ax.bbox)
                
                # Restore highlight artists visibility
                for artist in self._highlight_artists:
                    artist.set_visible(True)
        except Exception as e:
            print(f"Error saving background: {e}")
            
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
                self.draw()
        except Exception as e:
            print(f"Error in fast highlight update: {e}")
            # Fall back to full redraw
            self.draw()
            
    def _update_display_fast(self):
        """Fast display update - only converts the current canvas to QPixmap without full redraw"""
        buf = np.frombuffer(self.canvas.buffer_rgba(), dtype=np.uint8)
        width, height = self.canvas.get_width_height()
        buf = buf.reshape(height, width, 4)  # RGBA format
        # Convert RGBA to RGB and ensure C-contiguous
        buf = np.ascontiguousarray(buf[:, :, :3])
    
        if len(buf) > 0 and buf.size > 0:
            # Ensure buffer is in the right format and C-contiguous
            if not buf.flags['C_CONTIGUOUS']:
                buf = np.ascontiguousarray(buf)
            
            # Calculate the correct bytes per line
            bytes_per_line = width * 3  # 3 bytes per pixel for RGB
            
            # Convert to QImage with proper stride
            qimg = QImage(buf.data, width, height, bytes_per_line, QImage.Format.Format_RGB888)
            
            if not qimg.isNull():
                # Convert to pixmap and display at exact widget size
                pixmap = QPixmap.fromImage(qimg)
                
                # Scale pixmap to exact label size to fill completely
                label_size = self.image_label.size()
                if label_size.width() > 0 and label_size.height() > 0:
                    # Scale to exact size, ignoring aspect ratio to fill completely
                    scaled_pixmap = pixmap.scaled(
                        label_size, 
                        Qt.AspectRatioMode.IgnoreAspectRatio,  # This fills the entire space
                        Qt.TransformationMode.SmoothTransformation
                    )
                    self.image_label.setPixmap(scaled_pixmap)
                else:
                    self.image_label.setPixmap(pixmap)
                        
    def _update_display(self):
        """Update the Qt widget with the current matplotlib figure"""
        # Get the figure as a numpy array
        self.canvas.draw()
        
        buf = np.frombuffer(self.canvas.buffer_rgba(), dtype=np.uint8)
        width, height = self.canvas.get_width_height()
        buf = buf.reshape(height, width, 4)  # RGBA format
        # Convert RGBA to RGB and ensure C-contiguous
        buf = np.ascontiguousarray(buf[:, :, :3])
        
        if len(buf) > 0 and buf.size > 0:
            # Ensure buffer is in the right format and C-contiguous
            if not buf.flags['C_CONTIGUOUS']:
                buf = np.ascontiguousarray(buf)
            
            # Calculate the correct bytes per line
            bytes_per_line = width * 3  # 3 bytes per pixel for RGB
            
            # Convert to QImage with proper stride
            qimg = QImage(buf.data, width, height, bytes_per_line, QImage.Format.Format_RGB888)
            
            if not qimg.isNull():
                # Convert to pixmap and display at exact widget size
                pixmap = QPixmap.fromImage(qimg)
                
                # Scale pixmap to exact label size to fill completely
                label_size = self.image_label.size()
                if label_size.width() > 0 and label_size.height() > 0:
                    # Scale to exact size, ignoring aspect ratio to fill completely
                    scaled_pixmap = pixmap.scaled(
                        label_size, 
                        Qt.AspectRatioMode.IgnoreAspectRatio,  # This fills the entire space
                        Qt.TransformationMode.SmoothTransformation
                    )
                    self.image_label.setPixmap(scaled_pixmap)
                else:
                    self.image_label.setPixmap(pixmap)
            else:
                self.image_label.setText("No plot data available")

    def _calculate_zoom_level(self):
        """Calculate current zoom level as percentage (100% = original view)"""
        if not hasattr(self, 'ax') or not self.ax:
            return 100
            
        if self._original_xlim is None or self._original_ylim is None:
            return 100
            
        current_xlim = self.ax.get_xlim()
        current_ylim = self.ax.get_ylim()
        
        # Calculate zoom based on the ratio of original to current view range
        original_x_range = self._original_xlim[1] - self._original_xlim[0]
        original_y_range = self._original_ylim[1] - self._original_ylim[0]
        
        current_x_range = current_xlim[1] - current_xlim[0]
        current_y_range = current_ylim[1] - current_ylim[0]
        
        # Use the average of x and y zoom ratios
        x_zoom = original_x_range / current_x_range * 100
        y_zoom = original_y_range / current_y_range * 100
        
        return int((x_zoom + y_zoom) / 2)
    
    def _set_zoom_level(self, zoom_percent):
        """Set zoom level to specific percentage"""
        if not hasattr(self, 'ax') or not self.ax:
            return
            
        if self._original_xlim is None or self._original_ylim is None:
            return
            
        # Calculate the zoom factor (zoom_percent/100)
        zoom_factor = zoom_percent / 100.0
        
        # Calculate the new ranges
        original_x_range = self._original_xlim[1] - self._original_xlim[0]
        original_y_range = self._original_ylim[1] - self._original_ylim[0]
        
        new_x_range = original_x_range / zoom_factor
        new_y_range = original_y_range / zoom_factor
        
        # Get current center point
        current_xlim = self.ax.get_xlim()
        current_ylim = self.ax.get_ylim()
        center_x = (current_xlim[0] + current_xlim[1]) / 2
        center_y = (current_ylim[0] + current_ylim[1]) / 2
        
        # Set new limits centered on current center
        new_xlim = [center_x - new_x_range / 2, center_x + new_x_range / 2]
        new_ylim = [center_y - new_y_range / 2, center_y + new_y_range / 2]
        
        self.ax.set_xlim(new_xlim)
        self.ax.set_ylim(new_ylim)
        
        # Clear highlights and redraw
        if hasattr(self, '_highlight_artists'):
            for artist in self._highlight_artists:
                if artist in self.ax.get_children():
                    artist.remove()
            self._highlight_artists.clear()
        
        self.draw()
        
    def wheelEvent(self, event):
        """Handle mouse wheel events for zooming"""
        if hasattr(self, 'ax') and self.ax:
            # Get current axis limits
            xlim = self.ax.get_xlim()
            ylim = self.ax.get_ylim()
            
            # Get mouse position in data coordinates
            widget_pos = event.position()
            widget_width = self.width()
            widget_height = self.height()
            
            # Convert widget coordinates to normalized coordinates (0-1)
            norm_x = widget_pos.x() / widget_width
            norm_y = 1 - (widget_pos.y() / widget_height)  # Flip Y axis
            
            # Convert normalized coordinates to data coordinates
            data_x = xlim[0] + norm_x * (xlim[1] - xlim[0])
            data_y = ylim[0] + norm_y * (ylim[1] - ylim[0])
            
            # Zoom factor
            zoom_factor = 1.1 if event.angleDelta().y() > 0 else 1.0 / 1.1
            
            # Calculate new limits centered on mouse position
            x_range = (xlim[1] - xlim[0]) * zoom_factor
            y_range = (ylim[1] - ylim[0]) * zoom_factor
            
            new_xlim = [data_x - x_range * norm_x, data_x + x_range * (1 - norm_x)]
            new_ylim = [data_y - y_range * norm_y, data_y + y_range * (1 - norm_y)]
            
            # Apply new limits
            self.ax.set_xlim(new_xlim)
            self.ax.set_ylim(new_ylim)
            
            # Clear any highlights since they may be out of view
            if hasattr(self, '_highlight_artists'):
                for artist in self._highlight_artists:
                    if artist in self.ax.get_children():
                        artist.remove()
                self._highlight_artists.clear()
            
            # Redraw and save new background
            self.draw()
            
            # Emit zoom change signal
            new_zoom = self._calculate_zoom_level()
            self.zoomChanged.emit(new_zoom)
    
    def mousePressEvent(self, event):
        """Handle mouse press events for panning"""
        if event.button() == Qt.MouseButton.LeftButton:
            self.pan_start = event.position()
            self.pan_xlim = self.ax.get_xlim() if hasattr(self, 'ax') and self.ax else None
            self.pan_ylim = self.ax.get_ylim() if hasattr(self, 'ax') and self.ax else None
            self.setCursor(Qt.CursorShape.ClosedHandCursor)
        super().mousePressEvent(event)
    
    def mouseMoveEvent(self, event):
        """Handle mouse move events for panning"""
        if hasattr(self, 'pan_start') and self.pan_start and hasattr(self, 'ax') and self.ax:
            if self.pan_xlim and self.pan_ylim:
                # Calculate the difference in widget coordinates
                dx = event.position().x() - self.pan_start.x()
                dy = event.position().y() - self.pan_start.y()
                
                # Convert to data coordinate differences
                widget_width = self.width()
                widget_height = self.height()
                
                if widget_width > 0 and widget_height > 0:
                    x_range = self.pan_xlim[1] - self.pan_xlim[0]
                    y_range = self.pan_ylim[1] - self.pan_ylim[0]
                    
                    data_dx = -dx * x_range / widget_width
                    data_dy = dy * y_range / widget_height  # Flip Y axis
                    
                    # Apply pan
                    new_xlim = [self.pan_xlim[0] + data_dx, self.pan_xlim[1] + data_dx]
                    new_ylim = [self.pan_ylim[0] + data_dy, self.pan_ylim[1] + data_dy]
                    
                    self.ax.set_xlim(new_xlim)
                    self.ax.set_ylim(new_ylim)
                    
                    # Use fast update during panning (without saving background)
                    self.canvas.draw()
                    self._update_display_fast()
        else:
            # Show pan cursor when hovering over the plot area
            self.setCursor(Qt.CursorShape.OpenHandCursor)
        super().mouseMoveEvent(event)
    
    def mouseReleaseEvent(self, event):
        """Handle mouse release events"""
        if event.button() == Qt.MouseButton.LeftButton:
            if hasattr(self, 'pan_start') and self.pan_start:
                # Clear any highlights and save new background after panning
                if hasattr(self, '_highlight_artists'):
                    for artist in self._highlight_artists:
                        if artist in self.ax.get_children():
                            artist.remove()
                    self._highlight_artists.clear()
                
                # Save new background after panning
                self.draw()
                
                # Emit zoom change signal (panning might affect perceived zoom)
                new_zoom = self._calculate_zoom_level()
                self.zoomChanged.emit(new_zoom)
                
            self.pan_start = None
            self.pan_xlim = None
            self.pan_ylim = None
            self.setCursor(Qt.CursorShape.OpenHandCursor)
        super().mouseReleaseEvent(event)
    
    def mouseDoubleClickEvent(self, event):
        """Handle double-click to reset view"""
        if event.button() == Qt.MouseButton.LeftButton and hasattr(self, 'ax') and self.ax:
            # Reset to auto-scale
            self.ax.autoscale()
            self.draw()
        super().mouseDoubleClickEvent(event)
    
    def enterEvent(self, event):
        """Handle mouse entering the widget"""
        self.setCursor(Qt.CursorShape.OpenHandCursor)
        super().enterEvent(event)
        
    def leaveEvent(self, event):
        """Handle mouse leaving the widget"""
        self.setCursor(Qt.CursorShape.ArrowCursor)
        super().leaveEvent(event)
        
    def resizeEvent(self, event):
        """Handle widget resize"""
        super().resizeEvent(event)
        # Redraw with new size for both backends
        self.draw_idle()


