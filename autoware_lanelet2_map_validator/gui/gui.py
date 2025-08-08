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

"""GUI application for Autoware Lanelet2 map validation with integrated map visualization."""
from __future__ import annotations

from datetime import datetime
import os
from pathlib import Path
import re
import subprocess
import sys

import PySide6

if getattr(sys, "frozen", False):
    SCRIPT_DIR = os.path.dirname(sys.executable)
else:
    SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

from gui_helper import DropLineEdit
from gui_helper import run_lanelet2_validator
from map_visualizer import MapVisualizerWidget

if not hasattr(PySide6, "__version__"):
    PySide6.__version__ = "6.0.0"

from PySide6.QtCore import QTimer
from PySide6.QtCore import Qt
from PySide6.QtWidgets import QApplication
from PySide6.QtWidgets import QCheckBox
from PySide6.QtWidgets import QComboBox
from PySide6.QtWidgets import QFileDialog
from PySide6.QtWidgets import QGridLayout
from PySide6.QtWidgets import QGroupBox
from PySide6.QtWidgets import QHBoxLayout
from PySide6.QtWidgets import QLabel
from PySide6.QtWidgets import QLineEdit
from PySide6.QtWidgets import QListWidget
from PySide6.QtWidgets import QListWidgetItem
from PySide6.QtWidgets import QMainWindow
from PySide6.QtWidgets import QPushButton
from PySide6.QtWidgets import QTabWidget
from PySide6.QtWidgets import QTextBrowser
from PySide6.QtWidgets import QVBoxLayout
from PySide6.QtWidgets import QWidget


class ValidatorUI(QMainWindow):
    """Main GUI window for the Lanelet2 map validator."""

    def __init__(self):
        super().__init__()
        self.setWindowTitle("OSM Validator")
        self.resize(1280, 720)

        self.available_validators = self.fetch_available_validators()
        self.map_visualizer = MapVisualizerWidget()

        self.error_content = ""
        self.warning_content = ""

        central = QWidget()
        main_layout = QHBoxLayout(central)

        validator_panel = QVBoxLayout()
        validator_panel.setContentsMargins(20, 20, 20, 20)
        validator_panel.setSpacing(15)

        validator_group = QGroupBox("Validator Selection")
        validator_group_layout = QVBoxLayout(validator_group)

        self.validator_list = QListWidget()
        self.validator_list.setSelectionMode(QListWidget.MultiSelection)
        self.validator_list.setMinimumHeight(650)

        for validator in self.available_validators:
            item = QListWidgetItem(validator)
            self.validator_list.addItem(item)

        validator_group_layout.addWidget(self.validator_list)

        # Manual input checkbox and text field
        manual_layout = QHBoxLayout()
        manual_layout.setContentsMargins(0, 0, 0, 0)
        self.manual_input_checkbox = QCheckBox("Manual regex input")
        self.manual_input_checkbox.stateChanged.connect(self.toggle_manual_input)
        manual_layout.addWidget(self.manual_input_checkbox)
        validator_group_layout.addLayout(manual_layout)

        self.validator_edit = QLineEdit()
        self.validator_edit.setPlaceholderText("Optional: Regex filter, e.g. mapping.*")
        self.validator_edit.setEnabled(False)
        validator_group_layout.addWidget(self.validator_edit)

        validator_panel.addWidget(validator_group)
        validator_panel.addStretch(1)
        main_layout.addLayout(validator_panel, stretch=2)

        report_panel = QVBoxLayout()
        report_group = QGroupBox("Report")
        report_layout = QVBoxLayout(report_group)
        self.tab_widget = QTabWidget()

        # Add map visualizer as the first tab
        self.tab_widget.addTab(self.map_visualizer, "Map Visualizer")

        self.error_tab = QTextBrowser()
        self.error_tab.setReadOnly(True)
        self.error_tab.setOpenExternalLinks(False)
        self.setup_text_edit_links(self.error_tab)
        self.tab_widget.addTab(self.error_tab, "Errors")

        self.warn_tab = QTextBrowser()
        self.warn_tab.setReadOnly(True)
        self.warn_tab.setOpenExternalLinks(False)
        self.setup_text_edit_links(self.warn_tab)
        self.tab_widget.addTab(self.warn_tab, "Warnings")

        self.info_tab = QTextBrowser()
        self.info_tab.setReadOnly(True)
        report_layout.addWidget(self.tab_widget)
        report_panel.addWidget(report_group, stretch=1)
        main_layout.addLayout(report_panel, stretch=5)

        input_panel = QVBoxLayout()
        input_panel.setContentsMargins(20, 20, 20, 20)
        input_panel.setSpacing(15)

        # OSM File group
        osm_group = QGroupBox("OSM File")
        osm_layout = QHBoxLayout(osm_group)
        self.osm_edit = DropLineEdit()
        self.osm_edit.setPlaceholderText("Drag & drop .osm here or click Browse…")
        browse_osm = QPushButton("Browse")
        browse_osm.clicked.connect(self.browse_osm)
        osm_layout.addWidget(self.osm_edit)
        osm_layout.addWidget(browse_osm)
        input_panel.addWidget(osm_group)

        req_group = QGroupBox("Input Requirement JSON")
        req_layout = QHBoxLayout(req_group)
        self.req_edit = DropLineEdit()
        self.req_edit.setPlaceholderText(
            "Optional: Drag & drop input_requirement.json here or " "click Browse…"
        )
        browse_req = QPushButton("Browse")
        browse_req.clicked.connect(self.browse_req)
        req_layout.addWidget(self.req_edit)
        req_layout.addWidget(browse_req)
        input_panel.addWidget(req_group)

        # Output Location group
        output_group = QGroupBox("Output Directory")
        output_layout = QHBoxLayout(output_group)
        self.output_edit = QLineEdit()
        self.output_edit.setPlaceholderText("Optional: Select output directory…")
        browse_output = QPushButton("Browse")
        browse_output.clicked.connect(self.browse_output)
        output_layout.addWidget(self.output_edit)
        output_layout.addWidget(browse_output)
        input_panel.addWidget(output_group)

        param_group = QGroupBox("Parameters YAML")
        param_layout = QHBoxLayout(param_group)
        self.param_edit = DropLineEdit()
        self.param_edit.setPlaceholderText(
            "Optional: Drag & drop params.yaml here or click Browse…"
        )
        browse_param = QPushButton("Browse")
        browse_param.clicked.connect(self.browse_param)
        param_layout.addWidget(self.param_edit)
        param_layout.addWidget(browse_param)
        input_panel.addWidget(param_group)

        excl_group = QGroupBox("Exclusion List JSON")
        excl_layout = QHBoxLayout(excl_group)
        self.excl_edit = DropLineEdit()
        self.excl_edit.setPlaceholderText(
            "Optional: Drag & drop exclusion_list.json here or " "click Browse…"
        )
        browse_excl = QPushButton("Browse")
        browse_excl.clicked.connect(self.browse_excl)
        excl_layout.addWidget(self.excl_edit)
        excl_layout.addWidget(browse_excl)
        input_panel.addWidget(excl_group)

        opts_group = QGroupBox("Options")
        opts_grid = QGridLayout(opts_group)
        opts_grid.setHorizontalSpacing(8)
        opts_grid.setVerticalSpacing(8)
        opts_grid.setContentsMargins(12, 12, 12, 12)

        opts_grid.addWidget(QLabel("Projector:"), 0, 0)
        self.projector_combo = QComboBox()
        self.projector_combo.addItems(["mgrs", "utm", "transverse_mercator"])
        opts_grid.addWidget(self.projector_combo, 0, 1)
        opts_grid.addWidget(QLabel("Language:"), 0, 2)
        self.language_combo = QComboBox()
        self.language_combo.addItems(["en", "ja"])
        opts_grid.addWidget(self.language_combo, 0, 3)

        # Visualize map checkbox (row 1)
        self.visualize_map_checkbox = QCheckBox("Visualize map")
        self.visualize_map_checkbox.setObjectName("visualize_map_checkbox")
        self.visualize_map_checkbox.setChecked(True)
        self.visualize_map_checkbox.setToolTip(
            "Enable/disable map visualization for better performance"
        )
        self.visualize_map_checkbox.setStyleSheet(
            """
            QCheckBox {
                font-size: 14px;
                font-weight: bold;
                padding: 4px;
            }
            QCheckBox::indicator {
                width: 18px;
                height: 18px;
            }
        """
        )
        self.visualize_map_checkbox.stateChanged.connect(self.toggle_map_visualization)
        opts_grid.addWidget(self.visualize_map_checkbox, 1, 0, 1, 2)

        input_panel.addWidget(opts_group)

        # menu Bar with Theme Switcher
        menubar = self.menuBar()
        settings_menu = menubar.addMenu("Settings")
        theme_menu = settings_menu.addMenu("Theme")

        action_light = theme_menu.addAction("Light")
        action_dark = theme_menu.addAction("Dark")

        def show_cli_help():
            help_text = """
<pre>
Help for autoware_lanelet2_map_validator GUI:
----------------------------------------------------------------------------------
OSM File (-m arg)           Path to the map to be validated (required)
----------------------------------------------------------------------------------
Input Requrement JSON       Path to the JSON file where the list of
(-i arg)                    requirements and validators is written (optional)
----------------------------------------------------------------------------------
Output Directory (-o arg)   Directory to save the list of validation
                            results in a JSON format (optional)
----------------------------------------------------------------------------------
Exclusion List JSON         List of primitives that should be excluded
(-x arg)                    from the validation in a JSON format (optional)
----------------------------------------------------------------------------------
Validator (-v arg)          Comma separated list of regexes to filter the
                            applicable validators. Will run all validators
                            by default.
                            Example: routing_graph.* to run all checks for the
                            routing graph
----------------------------------------------------------------------------------
Parameter YAML              Path to the YAML file where a list of
(--parameteres arg)         parameters is written (optional)
----------------------------------------------------------------------------------
Projector (-p arg)          Projector used for loading lanelet map.
                            Available projectors are: mgrs, utm,
                            transverse_mercator. (default: mgrs)
----------------------------------------------------------------------------------
language (-l arg)           Language to display the issue messages. Available
                            languare are: jp, en. (default: en)
----------------------------------------------------------------------------------
</pre>
                        """
            self.info_tab.setHtml(help_text)
            self.tab_widget.setCurrentWidget(self.info_tab)

        # Add Help menu
        help_menu = menubar.addMenu("Help")
        action_cli_help = help_menu.addAction("Show CLI Help")
        action_cli_help.triggered.connect(show_cli_help)

        def set_theme_light():
            self.setStyleSheet(
                """
                QWidget { background-color: #f6f6f7; color: #232629; font-family: 'Segoe UI', 'Noto Sans', 'Arial', sans-serif; font-size: 13px; }

                QLineEdit, QPushButton {
                    background-color: #fff; color: #232629;
                    border-radius: 6px; border: 1px solid #bbb;
                    padding: 6px;
                }
                QTextBrowser {
                    background-color: #fff; color: #232629;
                    border: 1px solid #bbb;
                    padding: 6px;
                }
                QListWidget {
                    background-color: #fff; color: #232629;
                    border: 1px solid #bbb; border-radius: 6px;
                    padding: 4px;
                }
                QListWidget::item {
                    padding: 4px 8px;
                    border-radius: 4px;
                    margin: 1px;
                }
                QListWidget::item:selected {
                    background-color: #0078d7;
                    color: white;
                }
                QListWidget::item:hover {
                    background-color: #e9e9ea;
                }
                QCheckBox {
                    color: #232629;
                    font-weight: 500;
                }
                QCheckBox::indicator {
                    width: 16px; height: 16px;
                    border: 1px solid #bbb;
                    border-radius: 3px;
                    background-color: #fff;
                }
                QCheckBox::indicator:checked {
                    background-color: #0078d7;
                    border-color: #0078d7;
                }
                /* Override for the visualize map checkbox */
                QCheckBox[objectName="visualize_map_checkbox"] {
                    font-size: 14px;
                    font-weight: bold;
                    color: #232629;
                    padding: 4px;
                }
                QCheckBox[objectName="visualize_map_checkbox"]::indicator {
                    width: 18px;
                    height: 18px;
                    border: 2px solid #0078d7;
                    border-radius: 4px;
                    background-color: #fff;
                }
                QCheckBox[objectName="visualize_map_checkbox"]::indicator:checked {
                    background-color: #0078d7;
                    border-color: #0078d7;
                }
                QPushButton {
                    background-color: #e9e9ea; color: #232629;
                    border-radius: 6px; border: 1px solid #bbb;
                    padding: 6px 16px;
                }
                QPushButton:hover {
                    background-color: #d5d5d7;
                }
                QTabWidget::pane { border: 1px solid #bbb;}
                QTabBar::tab {
                    background: #e9e9ea; color: #232629; border-radius: 0px; padding: 6px 12px;
                }
                QTabBar::tab:selected {
                    background: #fff; color: #232629; border-radius: 0px;
                }
                QGroupBox {
                    border: 1px solid #bbb; border-radius: 8px; margin-top: 10px;
                    font-weight: bold;
                }
                QGroupBox:title {
                    subcontrol-origin: margin; left: 10px; padding: 0 3px 0 3px;
                }
                QLabel { font-weight: 500; }
                QMenu::item:selected {
                    background-color: #e9e9ea;
                    color: #232629;
                }
            """
            )

        def set_theme_dark():
            self.setStyleSheet(
                """
                QWidget { background-color: #232629; color: #f0f0f0; font-family: 'Segoe UI', 'Noto Sans', 'Arial', sans-serif; font-size: 13px; }
                QLineEdit, QPushButton {
                    background-color: #31363b; color: #f0f0f0;
                    border-radius: 6px; border: 1px solid #444;
                    padding: 6px;
                }
                QTextBrowser {
                    background-color: #31363b; color: #f0f0f0;
                    border: 1px solid #444;
                    padding: 6px;
                }
                QListWidget {
                    background-color: #31363b; color: #f0f0f0;
                    border: 1px solid #444; border-radius: 6px;
                    padding: 4px;
                }
                QListWidget::item {
                    padding: 4px 8px;
                    border-radius: 4px;
                    margin: 1px;
                }
                QListWidget::item:selected {
                    background-color: #0078d7;
                    color: white;
                }
                QListWidget::item:hover {
                    background-color: #50555b;
                }
                QCheckBox {
                    color: #f0f0f0;
                    font-weight: 500;
                }
                QCheckBox::indicator {
                    width: 16px; height: 16px;
                    border: 1px solid #555;
                    border-radius: 3px;
                    background-color: #31363b;
                }
                QCheckBox::indicator:checked {
                    background-color: #0078d7;
                    border-color: #0078d7;
                }
                /* Override for the visualize map checkbox */
                QCheckBox[objectName="visualize_map_checkbox"] {
                    font-size: 14px;
                    font-weight: bold;
                    color: #f0f0f0;
                    padding: 4px;
                }
                QCheckBox[objectName="visualize_map_checkbox"]::indicator {
                    width: 18px;
                    height: 18px;
                    border: 2px solid #0078d7;
                    border-radius: 4px;
                    background-color: #31363b;
                }
                QCheckBox[objectName="visualize_map_checkbox"]::indicator:checked {
                    background-color: #0078d7;
                    border-color: #0078d7;
                }
                QPushButton {
                    background-color: #3b4045; color: #f0f0f0;
                    border-radius: 6px; border: 1px solid #555;
                    padding: 6px 16px;
                }
                QPushButton:hover {
                    background-color: #50555b;
                }
                QTabWidget::pane { border: 1px solid #444; }
                QTabBar::tab {
                    background: #31363b; color: #f0f0f0; border-radius: 0px; padding: 6px 12px;
                }
                QTabBar::tab:selected {
                    background: #50555b; color: #fff; border-radius: 0px;
                }
                QGroupBox {
                    border: 1px solid #444; border-radius: 8px; margin-top: 10px;
                    font-weight: bold;
                }
                QGroupBox:title {
                    subcontrol-origin: margin; left: 10px; padding: 0 3px 0 3px;
                }
                QLabel { font-weight: 500; }
                QMenu::item:selected {
                    background-color: #50555b;
                    color: #fff;
                }
            """
            )

        action_light.triggered.connect(set_theme_light)
        action_dark.triggered.connect(set_theme_dark)
        set_theme_dark()
        input_panel.addSpacing(18)

        # Emphasized Run Validator button
        btn_layout = QHBoxLayout()
        self.run_btn = QPushButton("Run Validator")
        self.run_btn.clicked.connect(self.run_validator)
        self.run_btn.setStyleSheet(
            """
            QPushButton {
                font-size: 16px;
                font-weight: bold;
                padding: 10px 0;
                background-color: #0078d7;
                color: white;
                border-radius: 6px;
            }
            QPushButton:hover {
                background-color: #005fa3;
            }
        """
        )
        btn_layout.addWidget(self.run_btn)
        input_panel.addLayout(btn_layout)
        input_panel.addStretch(1)
        main_layout.addLayout(input_panel, stretch=3)

        # customize margins and spacing
        input_panel.setContentsMargins(24, 24, 24, 24)
        input_panel.setSpacing(18)
        report_panel.setContentsMargins(18, 18, 18, 18)
        report_panel.setSpacing(12)

        self.tab_widget.setTabPosition(QTabWidget.North)
        self.setCentralWidget(central)

    def toggle_map_visualization(self, state):
        """Toggle map visualization without removing the tab."""
        if state == Qt.Checked:
            self.map_visualizer.map_info_label.setText("Map visualization enabled")
            osm_path = self.osm_edit.text().strip()
            if osm_path and os.path.exists(osm_path):
                projector = self.projector_combo.currentText()
                try:
                    self.map_visualizer.load_map_file(osm_path, projector)
                except Exception as e:
                    print(f"Error reloading map for visualization: {e}")
                    self.map_visualizer.map_info_label.setText(f"Error loading map: {str(e)}")
            else:
                if hasattr(self.map_visualizer, "ax") and self.map_visualizer.ax:
                    self.map_visualizer.ax.clear()
                    self.map_visualizer.ax.text(
                        0.5,
                        0.5,
                        "Map visualization enabled\nLoad an OSM file to view the map",
                        transform=self.map_visualizer.ax.transAxes,
                        ha="center",
                        va="center",
                        fontsize=14,
                        color="green",
                    )
                    if hasattr(self.map_visualizer, "canvas"):
                        self.map_visualizer.canvas.draw()
        else:
            self.map_visualizer.map_info_label.setText("Map visualization disabled")

            if hasattr(self.map_visualizer, "ax") and self.map_visualizer.ax:
                self.map_visualizer.ax.clear()
                self.map_visualizer.ax.text(
                    0.5,
                    0.5,
                    'Map visualization disabled\nEnable "Visualize map" in Options to view the map',
                    transform=self.map_visualizer.ax.transAxes,
                    ha="center",
                    va="center",
                    fontsize=14,
                    color="gray",
                )
                if hasattr(self.map_visualizer, "canvas"):
                    self.map_visualizer.canvas.draw()

            if hasattr(self.map_visualizer, "lanelet_map"):
                self.map_visualizer.lanelet_map = None
                self.map_visualizer.current_file = None

    def browse_osm(self):
        """Browse for OSM file."""
        fn, _ = QFileDialog.getOpenFileName(self, "Select .osm file", "", "OSM Files (*.osm)")
        if fn:
            self.osm_edit.setText(fn)

    def browse_excl(self):
        """Browse for exclusion list JSON file."""
        fn, _ = QFileDialog.getOpenFileName(
            self, "Select exclusion list JSON", "", "JSON Files (*.json)"
        )
        if fn:
            self.excl_edit.setText(fn)

    def browse_req(self):
        """Browse for input requirement JSON file."""
        fn, _ = QFileDialog.getOpenFileName(
            self, "Select input requirement JSON", "", "JSON Files (*.json)"
        )
        if fn:
            self.req_edit.setText(fn)

    def browse_param(self):
        """Browse for parameters YAML file."""
        fn, _ = QFileDialog.getOpenFileName(
            self, "Select Parameters YAML", "", "YAML Files (*.yaml)"
        )
        if fn:
            self.param_edit.setText(fn)

    def browse_output(self):
        """Browse for output directory."""
        dir_path = QFileDialog.getExistingDirectory(self, "Select Output Directory", "")
        if dir_path:
            self.output_edit.setText(dir_path)

    def fetch_available_validators(self):
        """Fetch available validators from the command line tool."""
        try:
            result = subprocess.run(
                [
                    "ros2",
                    "run",
                    "autoware_lanelet2_map_validator",
                    "autoware_lanelet2_map_validator",
                    "--print",
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
            output = result.stdout.strip()

            validators = []
            if output:
                lines = output.split("\n")
                for line in lines:
                    line = line.strip()
                    if (
                        line
                        and not line.startswith("=")
                        and not line.startswith("-")
                        and not line.startswith("Available")
                        and not line.startswith("The")
                        and not line.lower().startswith("list")
                        and not line.lower().startswith("following")
                        and len(line.split()) > 0
                    ):

                        validator_name = line.split()[0] if line.split() else line

                        if (
                            validator_name
                            and validator_name not in validators
                            and validator_name
                            not in ["The", "Available", "List", "Following", "validators:", "are:"]
                            and validator_name.lower()
                            not in ["the", "available", "list", "following", "validators", "are"]
                            and (
                                "." in validator_name
                                or "_" in validator_name
                                or validator_name.islower()
                            )
                        ):
                            validators.append(validator_name)

            return sorted(validators) if validators else []
        except Exception as e:
            print(f"Error fetching validators: {e}")
            return []

    def toggle_manual_input(self, state):
        """Toggle between multi-select picker and manual regex input."""
        is_manual = state == Qt.Checked
        self.validator_edit.setEnabled(is_manual)
        self.validator_list.setEnabled(not is_manual)

        if is_manual:
            self.validator_list.clearSelection()
        else:
            self.validator_edit.clear()

    def run_validator(self):
        osm_path = self.osm_edit.text().strip()
        excl_path = self.excl_edit.text().strip()
        req_path = self.req_edit.text().strip()
        projector = self.projector_combo.currentText()
        param_path = self.param_edit.text().strip()
        output_dir = self.output_edit.text().strip()
        language = self.language_combo.currentText()

        # Get validator filter based on input method
        if self.manual_input_checkbox.isChecked():
            # Use manual regex input
            validator_filter = self.validator_edit.text().strip()
        else:
            # Use selected validators from multi-select picker
            selected_items = self.validator_list.selectedItems()
            if selected_items:
                # Join selected validator names with pipe (|) for regex OR operation
                selected_validators = [item.text() for item in selected_items]
                validator_filter = "|".join(selected_validators)
            else:
                validator_filter = ""

        # Validate paths
        if not os.path.exists(osm_path):
            self.error_content = (
                '<span style="color:red">Please specify valid .osm file first.</span>'
            )
            self.error_tab.setHtml(self.error_content)
            self.warn_tab.clear()
            self.tab_widget.setCurrentWidget(self.error_tab)
            return
        if excl_path and not os.path.exists(excl_path):
            self.error_content = '<span style="color:red">Exclusion file path is invalid.</span>'
            self.error_tab.setHtml(self.error_content)
            self.warn_tab.clear()
            self.tab_widget.setCurrentWidget(self.error_tab)
            return
        if req_path and not os.path.exists(req_path):
            self.error_content = (
                '<span style="color:red">Input requirement file path is invalid.</span>'
            )
            self.error_tab.setHtml(self.error_content)
            self.warn_tab.clear()
            self.tab_widget.setCurrentWidget(self.error_tab)
            return

        if param_path and not os.path.exists(param_path):
            self.error_content = '<span style="color:red">Parameter file path is invalid.</span>'
            self.error_tab.setHtml(self.error_content)
            self.warn_tab.clear()
            self.tab_widget.setCurrentWidget(self.error_tab)
            return

        if output_dir and not os.path.exists(output_dir):
            self.error_content = '<span style="color:red">Output file path is invalid.</span>'
            self.error_tab.setHtml(self.error_content)
            self.warn_tab.clear()
            self.tab_widget.setCurrentWidget(self.error_tab)
            return

        if output_dir:
            output_file = os.path.join(output_dir, "lanelet2_validation_results.json")
            existed_before = os.path.exists(output_file)

        try:
            if self.visualize_map_checkbox.isChecked():
                try:
                    self.map_visualizer.load_map_file(osm_path, projector)
                except Exception as e:
                    print(f"Error loading map for visualization: {e}")
                    self.map_visualizer.map_info_label.setText(f"Error loading map: {str(e)}")
            else:
                self.map_visualizer.map_info_label.setText("Map visualization disabled")

            rc, out_lines, err_lines = run_lanelet2_validator(
                projector=projector,
                map_file=Path(osm_path),
                input_requirements=Path(req_path) if req_path else None,
                exclusion_file=Path(excl_path) if excl_path else None,
                parameters_path=Path(param_path) if param_path else None,
                output_dir=Path(output_dir) if output_dir else None,
                language=language,
                validator_filter=validator_filter if validator_filter else None,
            )

            # Populate errors
            error_html = []
            if rc != 0 or err_lines:
                if rc != 0:
                    error_html.append(
                        '<span style="color:red">Validator failed or errors present:</span><br/>'
                    )
                for line in err_lines:
                    if ":" in line:
                        _, rest = line.split(":", 1)
                        clickable_rest = self.make_clickable_message(rest)
                        error_html.append(
                            f'<span style="color:red">Error:</span><span>{clickable_rest}</span><br/>'
                        )
                    else:
                        clickable_line = self.make_clickable_message(line)
                        error_html.append(
                            f'<span style="color:red">Error:</span><span>{clickable_line}</span><br/>'
                        )

            # Set error content and store it
            self.error_content = "".join(error_html) if error_html else "<span>No errors.</span>"
            self.error_tab.setHtml(self.error_content)

            # Populate warnings
            warn_html = []
            for line in out_lines:
                if line.lower().startswith("warning"):
                    if ":" in line:
                        _, rest = line.split(":", 1)
                        clickable_rest = self.make_clickable_message(rest)
                        warn_html.append(
                            f'<span style="color:orange">Warning:</span>'
                            f"<span>{clickable_rest}</span><br/>"
                        )
                    else:
                        clickable_line = self.make_clickable_message(line)
                        warn_html.append(
                            f'<span style="color:orange">Warning:</span>'
                            f"<span>{clickable_line}</span><br/>"
                        )

            self.warning_content = "".join(warn_html) if warn_html else "<span>No warnings.</span>"
            self.warn_tab.setHtml(self.warning_content)

            if error_html:
                self.tab_widget.setCurrentWidget(self.error_tab)
            else:
                self.tab_widget.setCurrentWidget(self.warn_tab)

            if output_dir:
                now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                if existed_before:
                    msg = (
                        f"<span>[{now}] lanelet2_validation_results.json "
                        f'<span style="color:yellow"> overwritten </span>'
                        f"<span> in {output_dir}</span>"
                    )
                else:
                    msg = (
                        f"<span>[{now}] lanelet2_validation_results.json "
                        f'<span style="color:green"> saved </span>'
                        f"<span> to {output_dir}</span>"
                    )
                self.info_tab.append(msg)

        except Exception as e:
            self.error_content = (
                f'<span style="color:red">Error running ' f"validator:<br/>{e}</span>"
            )
            self.error_tab.setHtml(self.error_content)
            self.warn_tab.clear()
            self.tab_widget.setCurrentWidget(self.error_tab)

    def make_clickable_message(self, message: str) -> str:
        """Convert element IDs in validation messages to clickable links."""
        patterns = [
            (r"Lanelet\s+(\d+)", "Lanelet"),
            (r"Area\s+(\d+)", "Area"),
            (r"Polygon\s+(\d+)", "Polygon"),
            (r"RegulatoryElement\s+(\d+)", "RegulatoryElement"),
            (r"LineString\s+(\d+)", "LineString"),
            (r"Point\s+(\d+)", "Point"),
            (r"lanelet\s+(\d+)", "Lanelet"),
            (r"area\s+(\d+)", "Area"),
            (r"polygon\s+(\d+)", "Polygon"),
            (r"regulatory\s*element\s+(\d+)", "RegulatoryElement"),
            (r"linestring\s+(\d+)", "LineString"),
            (r"point\s+(\d+)", "Point"),
            (r"way\s+(\d+)", "LineString"),  # OSM way -> LineString
            (r"node\s+(\d+)", "Point"),  # OSM node -> Point
            (r"relation\s+(\d+)", "Area"),  # OSM relation -> Area
        ]

        result = message
        for pattern, element_type in patterns:

            def replace_func(match):
                element_id = match.group(1)
                return (
                    f'<a href="element://{element_type}/{element_id}" '
                    f'style="color: #0066cc; text-decoration: underline;">'
                    f"{match.group(0)}</a>"
                )

            result = re.sub(pattern, replace_func, result, flags=re.IGNORECASE)

        return result

    def setup_text_edit_links(self, text_edit: QTextBrowser):
        """Set up clickable links in a text edit widget."""

        def on_anchor_clicked(url):
            if text_edit == self.error_tab:
                self.error_content = text_edit.toHtml()
            elif text_edit == self.warn_tab:
                self.warning_content = text_edit.toHtml()

            # Call the original handler
            self.on_element_link_clicked(url)

            # Use a timer to restore content after a short delay
            # This ensures the content is restored even if Qt clears it
            def restore_content():
                if text_edit == self.error_tab and self.error_content:
                    text_edit.setHtml(self.error_content)
                elif text_edit == self.warn_tab and self.warning_content:
                    text_edit.setHtml(self.warning_content)

            # Restore content immediately
            restore_content()

            # Also restore after a short delay in case Qt processes the link asynchronously
            QTimer.singleShot(50, restore_content)

        text_edit.anchorClicked.connect(on_anchor_clicked)

    def on_element_link_clicked(self, url):
        """Handle clicks on element links."""
        url_str = url.toString()
        if url_str.startswith("element://"):
            # Parse the element type and ID from the URL
            parts = url_str.replace("element://", "").split("/")
            if len(parts) == 2:
                element_type, element_id = parts
                print(f"Highlighting {element_type} {element_id}")

                # Check if map visualization is enabled
                if not self.visualize_map_checkbox.isChecked():
                    # Show a message in the map visualizer (tab is always present)
                    self.map_visualizer.map_info_label.setText(
                        f"Cannot highlight {element_type} {element_id}: "
                        "Map visualization is disabled. Enable it in Options."
                    )

                    # Clear the visualizer and show a message
                    if hasattr(self.map_visualizer, "ax") and self.map_visualizer.ax:
                        self.map_visualizer.ax.clear()
                        self.map_visualizer.ax.text(
                            0.5,
                            0.5,
                            f"Cannot highlight {element_type} {element_id}\n\n"
                            "Map visualization is disabled.\n"
                            'Enable "Visualize map" in Options to view and highlight elements.',
                            transform=self.map_visualizer.ax.transAxes,
                            ha="center",
                            va="center",
                            fontsize=12,
                            color="orange",
                            bbox={
                                "boxstyle": "round,pad=0.5",
                                "facecolor": "lightyellow",
                                "alpha": 0.8,
                            },
                        )
                        if hasattr(self.map_visualizer, "canvas"):
                            self.map_visualizer.canvas.draw()

                    self.tab_widget.setCurrentIndex(0)
                    return

                # Map visualization is enabled, proceed with highlighting

                # Load the current map file into the visualizer if not already loaded
                osm_path = self.osm_edit.text().strip()
                projector_type = self.projector_combo.currentText()
                if osm_path and os.path.exists(osm_path):
                    if self.map_visualizer.current_file != osm_path:
                        # Load the map file with the selected projector
                        try:
                            self.map_visualizer.load_map_file(osm_path, projector_type)
                        except Exception as e:
                            print(f"Error loading map for visualization: {e}")

                # Highlight the element on the map
                self.map_visualizer.jump_to_element(element_type, element_id)

                # Switch to the Map Visualizer tab (index 0)
                self.tab_widget.setCurrentIndex(0)

                # Update the map visualizer info to show which element was highlighted
                self.map_visualizer.map_info_label.setText(
                    f"Highlighted: {element_type} {element_id} | "
                    + self.map_visualizer.map_info_label.text()
                )


if __name__ == "__main__":
    app = QApplication(sys.argv)
    validator_ui = ValidatorUI()
    validator_ui.show()
    sys.exit(app.exec())
