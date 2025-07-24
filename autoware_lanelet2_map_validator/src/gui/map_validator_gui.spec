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

datas = [
    (os.path.join(spec_dir, 'gui_helper.py'), '.'),
    (os.path.join(spec_dir, 'map_visualizer.py'), '.'),
    (os.path.join(spec_dir, 'matplotlib_widget.py'), '.'),
]

# Use dynamic search instead of hardcoded path
binaries = find_lanelet2_binaries()

hiddenimports = [
    'mpl_toolkits.mplot3d',
    'gui_helper',
    'map_visualizer',
    'matplotlib_widget',
    'lanelet2',
    'autoware_lanelet2_extension_python.projection',
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
