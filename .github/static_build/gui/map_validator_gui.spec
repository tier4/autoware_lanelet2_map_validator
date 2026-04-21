# -*- mode: python ; coding: utf-8 -*-
from PyInstaller.utils.hooks import collect_all
import os
import sys
import glob

# Directory where this spec file lives. SPEC is injected by PyInstaller.
spec_dir = os.path.dirname(os.path.abspath(SPEC))


def find_lanelet2_binaries():
    """Dynamically locate the lanelet2 Python module (shared libs + .py files)
    from a ROS-sourced environment.
    """
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


def find_autoware_lanelet2_extension_python_files():
    """Bundle autoware_lanelet2_extension_python (used by gui_helper.py at
    runtime via MGRSProjector). HEAD's spec does not do this, so a standalone
    binary fails with ImportError; this function is the main static-build
    adaptation carried by this overlay.
    """
    bins, dats = [], []

    # Primary: whatever the running interpreter sees on sys.path
    # (e.g. /opt/ros/<distro>/lib/python<X.Y>/site-packages/).
    ext_python_path = None
    for path in sys.path:
        candidate = os.path.join(path, 'autoware_lanelet2_extension_python')
        if os.path.exists(candidate):
            ext_python_path = candidate
            break

    if ext_python_path:
        for sofile in glob.glob(os.path.join(ext_python_path, '**/*.so'), recursive=True):
            rel_path = os.path.relpath(sofile, ext_python_path)
            bins.append((sofile, os.path.join('autoware_lanelet2_extension_python',
                                              os.path.dirname(rel_path))))
        for pyfile in glob.glob(os.path.join(ext_python_path, '**/*.py'), recursive=True):
            rel_path = os.path.relpath(pyfile, os.path.dirname(ext_python_path))
            dats.append((pyfile, os.path.dirname(rel_path)))
    else:
        print("Warning: autoware_lanelet2_extension_python not found on sys.path!")

    # Optional: source-built autoware_lanelet2_extension (e.g. from the release
    # workflow's colcon build). Bundled if AUTOWARE_LANELET2_EXTENSION_PATH is set.
    autoware_ext_path = os.environ.get('AUTOWARE_LANELET2_EXTENSION_PATH')
    if autoware_ext_path and os.path.exists(autoware_ext_path):
        for sofile in glob.glob(os.path.join(autoware_ext_path, '**/*.so'), recursive=True):
            rel_path = os.path.relpath(sofile, autoware_ext_path)
            bins.append((sofile, os.path.join('autoware_lanelet2_extension',
                                              os.path.dirname(rel_path))))
        for pyfile in glob.glob(os.path.join(autoware_ext_path, '**/*.py'), recursive=True):
            rel_path = os.path.relpath(pyfile, os.path.dirname(autoware_ext_path))
            dats.append((pyfile, os.path.dirname(rel_path)))

    return bins, dats


datas = [
    (os.path.join(spec_dir, 'gui_helper.py'), '.'),
    (os.path.join(spec_dir, 'map_visualizer.py'), '.'),
    (os.path.join(spec_dir, 'matplotlib_widget.py'), '.'),
]

binaries = find_lanelet2_binaries()

_ext_bins, _ext_datas = find_autoware_lanelet2_extension_python_files()
binaries += _ext_bins
datas += _ext_datas

hiddenimports = [
    'mpl_toolkits.mplot3d',
    'gui_helper',
    'map_visualizer',
    'matplotlib_widget',
    'lanelet2',
    'lanelet2.io',
    'lanelet2.projection',
    'autoware_lanelet2_extension_python',
    'autoware_lanelet2_extension_python.projection',
    # distutils removed in Python 3.12; matplotlib may import distutils.version
    'setuptools._distutils',
    'setuptools._distutils.version',
]
tmp_ret = collect_all('PySide6-essential')
datas += tmp_ret[0]
binaries += tmp_ret[1]
hiddenimports += tmp_ret[2]

a = Analysis(
    ['gui.py', 'gui_helper.py', 'map_visualizer.py', 'matplotlib_widget.py'],
    pathex=[],
    binaries=binaries,
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[os.path.join(spec_dir, 'pyi_rth_distutils.py')],
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
    # Name must match the filename the release workflow copies into the zip
    # (see build-and-upload-binaries.yaml "Package artifacts" step).
    name='autoware_lanelet2_map_validator_gui',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
