# -*- mode: python ; coding: utf-8 -*-
import os
import glob
import sys

# Add paths for lanelet2 and autoware_lanelet2_extension_python
binaries = []
datas = []
# Find and add lanelet2 .so files
lanelet2_path = os.environ.get('LANELET2_INSTALL_DIR')
if lanelet2_path:
    # Adjust for typical CMake install layout
    lanelet2_path = os.path.join(lanelet2_path, 'lib', 'python3.10', 'site-packages', 'lanelet2')

if not lanelet2_path or not os.path.exists(lanelet2_path):
    lanelet2_path = None
    for path in sys.path:
        potential_path = os.path.join(path, 'lanelet2')
        if os.path.exists(potential_path):
            lanelet2_path = potential_path
            break

if lanelet2_path:
    # Add .so files
    for sofile in glob.glob(os.path.join(lanelet2_path, '**/*.so'), recursive=True):
        rel_path = os.path.relpath(sofile, lanelet2_path)
        binaries.append((sofile, os.path.join('lanelet2', os.path.dirname(rel_path))))
    # Add Python files
    for pyfile in glob.glob(os.path.join(lanelet2_path, '**/*.py'), recursive=True):
        rel_path = os.path.relpath(pyfile, os.path.dirname(lanelet2_path))
        datas.append((pyfile, os.path.dirname(rel_path)))

# Find and add autoware_lanelet2_extension_python files
ext_python_path = None
for path in sys.path:
    potential_path = os.path.join(path, 'autoware_lanelet2_extension_python')
    if os.path.exists(potential_path):
        ext_python_path = potential_path
        break

if ext_python_path:
    # Add .so files
    for sofile in glob.glob(os.path.join(ext_python_path, '**/*.so'), recursive=True):
        rel_path = os.path.relpath(sofile, ext_python_path)
        binaries.append((sofile, os.path.join('autoware_lanelet2_extension_python', os.path.dirname(rel_path))))

    # Add Python files
    for pyfile in glob.glob(os.path.join(ext_python_path, '**/*.py'), recursive=True):
        rel_path = os.path.relpath(pyfile, os.path.dirname(ext_python_path))
        datas.append((pyfile, os.path.dirname(rel_path)))

# Also add from autoware source if available via environment variable
autoware_ext_path = os.environ.get('AUTOWARE_LANELET2_EXTENSION_PATH')
if autoware_ext_path and os.path.exists(autoware_ext_path):
    for sofile in glob.glob(os.path.join(autoware_ext_path, '**/*.so'), recursive=True):
        rel_path = os.path.relpath(sofile, autoware_ext_path)
        binaries.append((sofile, os.path.join('autoware_lanelet2_extension', os.path.dirname(rel_path))))
    # Add Python files from source
    for pyfile in glob.glob(os.path.join(autoware_ext_path, '**/*.py'), recursive=True):
        rel_path = os.path.relpath(pyfile, os.path.dirname(autoware_ext_path))
        datas.append((pyfile, os.path.dirname(rel_path)))

a = Analysis(
    ['gui.py'],
    pathex=[''],
    binaries=binaries,
    datas=datas,
    hiddenimports=['lanelet2', 'lanelet2.io', 'autoware_lanelet2_extension_python', 'autoware_lanelet2_extension_python.projection'],
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
