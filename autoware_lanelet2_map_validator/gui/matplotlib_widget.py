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

"""Custom matplotlib widget for PySide6 Essentials compatibility."""
from PySide6.QtCore import QTimer
from PySide6.QtCore import Qt
from PySide6.QtCore import Signal
from PySide6.QtGui import QImage
from PySide6.QtGui import QPixmap
from PySide6.QtWidgets import QLabel
from PySide6.QtWidgets import QSizePolicy
from PySide6.QtWidgets import QVBoxLayout
from PySide6.QtWidgets import QWidget
import matplotlib
from matplotlib.backends.backend_agg import FigureCanvasAgg
from matplotlib.figure import Figure
import numpy as np

matplotlib.use("Agg")


class MatplotlibWidget(QWidget):
    """Custom Qt widget to display matplotlib plots with both interactive and static support."""

    zoomChanged = Signal(int)

    def __init__(self, figure=None, parent=None):
        super().__init__(parent)
        self.figure = figure or Figure()
        self.setMinimumSize(600, 400)

        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)
        self.setLayout(layout)

        self.canvas = FigureCanvasAgg(self.figure)

        self.image_label = QLabel()
        self.image_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.image_label.setStyleSheet("border: none; background-color: white;")
        self.image_label.setScaledContents(False)
        self.image_label.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        layout.addWidget(self.image_label)

        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self.update_display)
        self.update_timer.setSingleShot(True)

        self._background = None
        self._highlight_artists = []

        self._original_xlim = None
        self._original_ylim = None
        self._current_zoom = 100

        self.setup_figure()

    def set_axes(self, ax):
        """Set the axes reference for zoom/pan functionality."""
        self.ax = ax
        self.setup_figure()

    def setup_figure(self):
        """Set up the figure to fill the entire widget space."""
        self.figure.patch.set_facecolor("white")
        self.figure.subplots_adjust(left=0, right=1, top=1, bottom=0, wspace=0, hspace=0)

        if hasattr(self, "ax") and self.ax:
            self.ax.set_position([0, 0, 1, 1])
            self.ax.set_facecolor("white")

    def resize_event(self, event):
        """Handle widget resize to adjust figure size accordingly."""
        super().resize_event(event)
        if event.size().isValid():
            widget_size = event.size()
            fig_width = max(widget_size.width() / 100.0, 1.0)
            fig_height = max(widget_size.height() / 100.0, 1.0)

            try:
                self.figure.set_size_inches(fig_width, fig_height, forward=False)
                self.figure.set_dpi(100)
                self.setup_figure()
                if hasattr(self, "ax") and self.ax:
                    self.draw_idle()
            except Exception as e:
                print(f"Error resizing figure: {e}")

    def draw(self):
        """Draw the matplotlib figure."""
        try:
            self.canvas.draw()
            self.save_background()

            if hasattr(self, "ax") and self.ax and self._original_xlim is None:
                self._original_xlim = self.ax.get_xlim()
                self._original_ylim = self.ax.get_ylim()

            self.update_display()
        except Exception as e:
            print(f"Error drawing matplotlib figure: {e}")

    def draw_idle(self):
        """Schedule a delayed draw."""
        if not self.update_timer.isActive():
            self.update_timer.start(50)

    def save_background(self):
        """Save the current background for fast blitting."""
        try:
            if hasattr(self, "ax") and self.ax:
                for artist in self._highlight_artists:
                    artist.set_visible(False)

                self.canvas.draw()
                self._background = self.canvas.copy_from_bbox(self.ax.bbox)

                for artist in self._highlight_artists:
                    artist.set_visible(True)
        except Exception as e:
            print(f"Error saving background: {e}")

    def fast_highlight_update(self):
        """Fast update using blitting - only redraws highlight elements."""
        try:
            if self._background and hasattr(self, "ax") and self.ax:
                self.canvas.restore_region(self._background)

                for artist in self._highlight_artists:
                    self.ax.draw_artist(artist)

                self.canvas.blit(self.ax.bbox)
                self.update_display_fast()
            else:
                self.draw()
        except Exception as e:
            print(f"Error in fast highlight update: {e}")
            self.draw()

    def update_display_fast(self):
        """Fast display update - only converts the current canvas to QPixmap without full redraw."""
        buf = np.frombuffer(self.canvas.buffer_rgba(), dtype=np.uint8)
        width, height = self.canvas.get_width_height()
        buf = buf.reshape(height, width, 4)
        buf = np.ascontiguousarray(buf[:, :, :3])

        if len(buf) > 0 and buf.size > 0:
            if not buf.flags["C_CONTIGUOUS"]:
                buf = np.ascontiguousarray(buf)

            bytes_per_line = width * 3
            qimg = QImage(buf.data, width, height, bytes_per_line, QImage.Format.Format_RGB888)

            if not qimg.isNull():
                pixmap = QPixmap.fromImage(qimg)
                label_size = self.image_label.size()
                if label_size.width() > 0 and label_size.height() > 0:
                    scaled_pixmap = pixmap.scaled(
                        label_size,
                        Qt.AspectRatioMode.IgnoreAspectRatio,
                        Qt.TransformationMode.SmoothTransformation,
                    )
                    self.image_label.setPixmap(scaled_pixmap)
                else:
                    self.image_label.setPixmap(pixmap)

    def update_display(self):
        """Update the Qt widget with the current matplotlib figure."""
        self.canvas.draw()

        buf = np.frombuffer(self.canvas.buffer_rgba(), dtype=np.uint8)
        width, height = self.canvas.get_width_height()
        buf = buf.reshape(height, width, 4)
        buf = np.ascontiguousarray(buf[:, :, :3])

        if len(buf) > 0 and buf.size > 0:
            if not buf.flags["C_CONTIGUOUS"]:
                buf = np.ascontiguousarray(buf)

            bytes_per_line = width * 3
            qimg = QImage(buf.data, width, height, bytes_per_line, QImage.Format.Format_RGB888)

            if not qimg.isNull():
                pixmap = QPixmap.fromImage(qimg)
                label_size = self.image_label.size()
                if label_size.width() > 0 and label_size.height() > 0:
                    scaled_pixmap = pixmap.scaled(
                        label_size,
                        Qt.AspectRatioMode.IgnoreAspectRatio,
                        Qt.TransformationMode.SmoothTransformation,
                    )
                    self.image_label.setPixmap(scaled_pixmap)
                else:
                    self.image_label.setPixmap(pixmap)
            else:
                self.image_label.setText("No plot data available")

    def calculate_zoom_level(self):
        """Calculate current zoom level as percentage (100% = original view)."""
        if not hasattr(self, "ax") or not self.ax:
            return 100

        if self._original_xlim is None or self._original_ylim is None:
            return 100

        current_xlim = self.ax.get_xlim()
        current_ylim = self.ax.get_ylim()

        original_x_range = self._original_xlim[1] - self._original_xlim[0]
        original_y_range = self._original_ylim[1] - self._original_ylim[0]

        current_x_range = current_xlim[1] - current_xlim[0]
        current_y_range = current_ylim[1] - current_ylim[0]

        x_zoom = original_x_range / current_x_range * 100
        y_zoom = original_y_range / current_y_range * 100

        return int((x_zoom + y_zoom) / 2)

    def set_zoom_level(self, zoom_percent):
        """Set zoom level to specific percentage."""
        zoom_factor = zoom_percent / 100.0

        original_x_range = self._original_xlim[1] - self._original_xlim[0]
        original_y_range = self._original_ylim[1] - self._original_ylim[0]

        new_x_range = original_x_range / zoom_factor
        new_y_range = original_y_range / zoom_factor

        current_xlim = self.ax.get_xlim()
        current_ylim = self.ax.get_ylim()
        center_x = (current_xlim[0] + current_xlim[1]) / 2
        center_y = (current_ylim[0] + current_ylim[1]) / 2

        new_xlim = [center_x - new_x_range / 2, center_x + new_x_range / 2]
        new_ylim = [center_y - new_y_range / 2, center_y + new_y_range / 2]

        self.ax.set_xlim(new_xlim)
        self.ax.set_ylim(new_ylim)

        if hasattr(self, "_highlight_artists"):
            for artist in self._highlight_artists:
                if artist in self.ax.get_children():
                    artist.remove()
            self._highlight_artists.clear()

        self.draw()

    def wheelEvent(self, event):
        """Handle mouse wheel events for zooming."""
        if hasattr(self, "ax") and self.ax:
            xlim = self.ax.get_xlim()
            ylim = self.ax.get_ylim()

            widget_pos = event.position()
            widget_width = self.width()
            widget_height = self.height()

            norm_x = widget_pos.x() / widget_width
            norm_y = 1 - (widget_pos.y() / widget_height)

            data_x = xlim[0] + norm_x * (xlim[1] - xlim[0])
            data_y = ylim[0] + norm_y * (ylim[1] - ylim[0])

            zoom_factor = 1.1 if event.angleDelta().y() > 0 else 1.0 / 1.1

            x_range = (xlim[1] - xlim[0]) * zoom_factor
            y_range = (ylim[1] - ylim[0]) * zoom_factor

            new_xlim = [data_x - x_range * norm_x, data_x + x_range * (1 - norm_x)]
            new_ylim = [data_y - y_range * norm_y, data_y + y_range * (1 - norm_y)]

            self.ax.set_xlim(new_xlim)
            self.ax.set_ylim(new_ylim)

            if hasattr(self, "_highlight_artists"):
                for artist in self._highlight_artists:
                    if artist in self.ax.get_children():
                        artist.remove()
                self._highlight_artists.clear()

            self.draw()

            new_zoom = self.calculate_zoom_level()
            self.zoomChanged.emit(new_zoom)

    def mousePressEvent(self, event):
        """Handle mouse press events for panning."""
        if event.button() == Qt.MouseButton.LeftButton:
            self.pan_start = event.position()
            self.pan_xlim = self.ax.get_xlim() if hasattr(self, "ax") and self.ax else None
            self.pan_ylim = self.ax.get_ylim() if hasattr(self, "ax") and self.ax else None
            self.setCursor(Qt.CursorShape.ClosedHandCursor)
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event):
        """Handle mouse move events for panning."""
        if hasattr(self, "pan_start") and self.pan_start and hasattr(self, "ax") and self.ax:
            if self.pan_xlim and self.pan_ylim:
                dx = event.position().x() - self.pan_start.x()
                dy = event.position().y() - self.pan_start.y()

                widget_width = self.width()
                widget_height = self.height()

                if widget_width > 0 and widget_height > 0:
                    x_range = self.pan_xlim[1] - self.pan_xlim[0]
                    y_range = self.pan_ylim[1] - self.pan_ylim[0]

                    data_dx = -dx * x_range / widget_width
                    data_dy = dy * y_range / widget_height

                    new_xlim = [self.pan_xlim[0] + data_dx, self.pan_xlim[1] + data_dx]
                    new_ylim = [self.pan_ylim[0] + data_dy, self.pan_ylim[1] + data_dy]

                    self.ax.set_xlim(new_xlim)
                    self.ax.set_ylim(new_ylim)

                    self.canvas.draw()
                    self.update_display_fast()
        else:
            self.setCursor(Qt.CursorShape.OpenHandCursor)
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):
        """Handle mouse release events."""
        if event.button() == Qt.MouseButton.LeftButton:
            if hasattr(self, "pan_start") and self.pan_start:
                if hasattr(self, "_highlight_artists"):
                    for artist in self._highlight_artists:
                        if artist in self.ax.get_children():
                            artist.remove()
                    self._highlight_artists.clear()

                self.draw()

                new_zoom = self.calculate_zoom_level()
                self.zoomChanged.emit(new_zoom)

            self.pan_start = None
            self.pan_xlim = None
            self.pan_ylim = None
            self.setCursor(Qt.CursorShape.OpenHandCursor)
        super().mouseReleaseEvent(event)

    def mouseDoubleClickEvent(self, event):
        """Handle double-click to reset view."""
        if event.button() == Qt.MouseButton.LeftButton and hasattr(self, "ax") and self.ax:
            self.ax.autoscale()
            self.draw()
        super().mouseDoubleClickEvent(event)

    def enterEvent(self, event):
        """Handle mouse entering the widget."""
        self.setCursor(Qt.CursorShape.OpenHandCursor)
        super().enterEvent(event)

    def leaveEvent(self, event):
        """Handle mouse leaving the widget."""
        self.setCursor(Qt.CursorShape.ArrowCursor)
        super().leaveEvent(event)
