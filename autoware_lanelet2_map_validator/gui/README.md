# Autoware Lanelet2 Map Validator GUI

A graphical user interface for validating Lanelet2 maps with integrated map visualization capabilities.

## Features

- **Interactive Map Validation**: Select and run specific validators on OSM/Lanelet2 map files
- **Integrated Map Visualization**: View maps with matplotlib-based rendering
- **Clickable Error/Warning Links**: Click on validation messages to highlight problematic elements on the map
- **Multi-tab Interface**: Separate tabs for map visualization, errors, warnings, and info
- **Drag & Drop Support**: Drag and drop files directly into input fields
- **Theme Support**: Light and dark themes available
- **Export Results**: Save validation results to JSON format

## Prerequisites

### System Requirements

- Python 3.10 or higher
- ROS 2 Humble (for the underlying validation tools)
- Pyside6
- Matplotlib

### ROS 2 Dependencies

Make sure the `autoware_lanelet2_map_validator` package is built and sourced:

## Running the GUI

### Method 1: Direct Python Execution (Recommended for Development)

1. **Install all dependencies**:

   ```bash
   pip install -r requirements.txt
   ```

2. **Run the GUI**:

   ```bash
   ros2 run autoware_lanelet2_map_validator gui.py
   ```

### Method 2: PyInstaller Executable (Standalone Distribution)

#### Building the Executable

1. **Install all dependencies**:

   ```bash
   pip install -r requirements.txt
   ```

2. **Install PyInstaller**:

   ```bash
   pip install pyinstaller
   ```

3. **Navigate to the GUI directory**:

   ```bash
   cd ~/$AUTOWARE_WORKSPACE/src/autoware_lanelet2_map_validator/autoware_lanelet2_map_validator/gui
   ```

4. **Source the Autoware workspace**:

   ```bash
   source $AUTOWARE_WORKSPACE/install/setup.bash
   ```

5. **Build the executable**:

   ```bash
   pyinstaller map_validator_gui.spec
   ```

6. **Run the executable**:

   ```bash
   ./dist/map_validator_gui
   ```

#### Distributing the Executable

The built executable in `./dist/map_validator_gui` can be distributed to other systems with similar architecture. The executable includes all Python dependencies but still requires:

- ROS 2 Humble environment
- The `autoware_lanelet2_map_validator` package for the subprocess CLI

## Usage Guide

### Basic Workflow

1. **Load a Map**:

   - Click "Browse" next to "OSM File" or drag & drop an `.osm` file
   - The map will automatically load in the visualization tab

2. **Select Validators**:
   - Choose from the list of available validators on the left panel
   - Or use "Manual regex input" for custom validator filtering

3. **Configure Options** (Optional):
   - Set projector type (MGRS, UTM, Transverse Mercator)
   - Choose language (English or Japanese)
   - Add requirement files, exclusion lists, or parameter files

4. **Run Validation**:
   - Click "Run Validator" button
   - Results will appear in the Errors/Warnings tabs

5. **Interactive Debugging**:
   - Click on element IDs in error/warning messages
   - The map will automatically highlight the problematic elements

### Input Files

- **OSM File** (Required): The map file to validate (`.osm` format)
- **Input Requirement JSON** (Optional): Validation requirements specification
- **Exclusion List JSON** (Optional): Elements to exclude from validation
- **Parameters YAML** (Optional): Custom validation parameters
- **Output Directory** (Optional): Where to save validation results

## Troubleshooting

### Common Issues

## Development

### File Structure

```bash
gui/
├── gui.py                 # Main application file
├── gui_helper.py          # Helper functions for validation
├── map_visualizer.py      # Map visualization widget
├── matplotlib_widget.py   # Matplotlib integration widget
├── map_validator_gui.spec # PyInstaller specification
├── __init__.py           # Package initialization
└── README.md             # This file
```

## License

This software is part of the Autoware project and is licensed under the Apache License 2.0.
