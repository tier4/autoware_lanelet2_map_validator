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
GUI package for Autoware Lanelet2 Map Validator.

This package contains the GUI application and supporting modules for
validating Lanelet2 maps with integrated visualization capabilities.
"""

try:
    from .gui import ValidatorUI
    from .gui_helper import DropLineEdit
    from .gui_helper import create_appropriate_projector
    from .gui_helper import run_lanelet2_validator
    from .map_visualizer import MapVisualizerWidget
    from .matplotlib_widget import MatplotlibWidget
except ImportError:
    pass

__version__ = "1.0.0"
__author__ = "Autoware Foundation"
__all__ = [
    "ValidatorUI",
    "MapVisualizerWidget",
    "MatplotlibWidget",
    "run_lanelet2_validator",
    "DropLineEdit",
    "create_appropriate_projector",
]
