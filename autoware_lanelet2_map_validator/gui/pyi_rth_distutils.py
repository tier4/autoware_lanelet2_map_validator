# Copyright 2026 TIER IV, Inc.
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

"""PyInstaller runtime hook: make ``distutils.version`` importable for matplotlib.

CPython 3.12+ removed ``distutils`` from the standard library. Setuptools still
provides it via ``setuptools._distutils``, but that shim is not always active
before bundled code imports ``distutils.version`` (e.g. some matplotlib
versions). Register the modules in ``sys.modules`` before the main script runs.
"""

from __future__ import annotations

import sys


def _install_distutils_shim() -> None:
    if "distutils.version" in sys.modules:
        return
    try:
        import setuptools  # noqa: F401 — must load before distutils (setuptools hack)
        import setuptools._distutils as _distutils
    except ImportError:
        return
    sys.modules.setdefault("distutils", _distutils)
    try:
        from setuptools._distutils import version as _version

        sys.modules["distutils.version"] = _version
    except ImportError:
        pass


_install_distutils_shim()
