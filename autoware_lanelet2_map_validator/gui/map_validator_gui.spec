# -*- mode: python ; coding: utf-8 -*-
from PyInstaller.utils.hooks import collect_all
import os
import sys
import glob

# Get the current directory where the spec file is located
spec_dir = os.path.dirname(os.path.abspath(SPEC))

# Dynamic path detection
def find_lanelet2_binaries():
    """Dynamically find lanelet2 shared libraries"""
    python_version = f"{sys.version_info.major}.{sys.version_info.minor}"
    ros_distro = os.environ.get('ROS_DISTRO', 'humble')

    search_paths = [
        f"/opt/ros/{ros_distro}/lib/python{python_version}/site-packages/lanelet2",
        f"/opt/ros/*/lib/python{python_version}/site-packages/lanelet2",
        f"/usr/lib/python{python_version}/site-packages/lanelet2",
    ]

    for path_pattern in search_paths:
        for path in glob.glob(path_pattern):
            if os.path.exists(path) and glob.glob(f"{path}/*.so"):
                print(f"Found lanelet2 at: {path}")
                return [(f"{path}/*.so", 'lanelet2')]

    print("Warning: lanelet2 binaries not found!")
    return []

def find_autoware_lanelet2_extension_python():
    """Dynamically find autoware_lanelet2_extension_python module"""
    search_paths = []
    
    # Check PYTHONPATH first
    pythonpath = os.environ.get('PYTHONPATH', '')
    if pythonpath:
        for path in pythonpath.split(':'):
            if 'autoware_lanelet2_extension_python' in path or path.endswith('site-packages'):
                module_path = os.path.join(path, 'autoware_lanelet2_extension_python')
                if os.path.exists(module_path):
                    search_paths.append(module_path)
    
    # Standard search paths
    search_paths.extend([
        "/opt/ros/humble/lib/python3.10/site-packages/autoware_lanelet2_extension_python",
        "/usr/lib/python3.10/site-packages/autoware_lanelet2_extension_python",
    ])
    
    # Also check environment variable
    custom_path = os.environ.get('AUTOWARE_LANELET2_EXTENSION_PYTHON_PATH')
    if custom_path:
        search_paths.insert(0, custom_path)

    for path in search_paths:
        if os.path.exists(path):
            print(f"Found autoware_lanelet2_extension_python at: {path}")
            binaries = []
            datas_list = []
            
            # Add .so files as binaries
            for so_file in glob.glob(f"{path}/*.so"):
                binaries.append((so_file, 'autoware_lanelet2_extension_python'))
            
            # Add .py files and __pycache__ as datas
            for py_file in glob.glob(f"{path}/*.py"):
                datas_list.append((py_file, 'autoware_lanelet2_extension_python'))
            
            if os.path.exists(f"{path}/__pycache__"):
                datas_list.append((f"{path}/__pycache__", 'autoware_lanelet2_extension_python/__pycache__'))
            
            return binaries, datas_list

    print("Warning: autoware_lanelet2_extension_python not found!")
    return [], []

datas = [
    (os.path.join(spec_dir, 'gui_helper.py'), '.'),
    (os.path.join(spec_dir, 'map_visualizer.py'), '.'),
    (os.path.join(spec_dir, 'matplotlib_widget.py'), '.'),
]

# Find autoware_lanelet2_extension_python
ext_binaries, ext_datas = find_autoware_lanelet2_extension_python()
binaries = find_lanelet2_binaries() + ext_binaries
datas += ext_datas

hiddenimports = [
    'mpl_toolkits.mplot3d',
    'gui_helper',
    'map_visualizer',
    'matplotlib_widget',
    'lanelet2',
    'autoware_lanelet2_extension_python',
    'autoware_lanelet2_extension_python.projection',
    'autoware_lanelet2_extension_python.utility',
    'autoware_lanelet2_extension_python.regulatory_elements',
]
tmp_ret = collect_all('PySide6-essential')
datas += tmp_ret[0]; binaries += tmp_ret[1]; hiddenimports += tmp_ret[2]

a = Analysis(
    ['gui.py', 'gui_helper.py', 'map_visualizer.py', 'matplotlib_widget.py'],
    pathex=[],
    binaries=binaries,
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='map_validator_gui',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
