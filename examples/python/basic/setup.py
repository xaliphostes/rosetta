#!/usr/bin/env python3
"""
Setup script for Rosetta Python bindings using pybind11
"""

import os
import sys
import subprocess
from pathlib import Path

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    """Custom extension that uses CMake to build"""

    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    """Custom build command that uses CMake"""

    def run(self):
        try:
            subprocess.check_output(["cmake", "--version"])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: "
                + ", ".join(e.name for e in self.extensions)
            )

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

        # Required for auto-detection of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
        ]

        cfg = "Debug" if self.debug else "Release"
        build_args = ["--config", cfg]

        # Platform-specific configuration
        if sys.platform.startswith("darwin"):
            # Cross-compile support for macOS
            archs = os.environ.get("ARCHFLAGS", "")
            if archs:
                cmake_args += [f"-DCMAKE_OSX_ARCHITECTURES={archs}"]

        # Add more build arguments
        cmake_args += [f"-DCMAKE_BUILD_TYPE={cfg}"]
        build_args += ["--", "-j4"]

        env = os.environ.copy()
        env["CXXFLAGS"] = f'{env.get("CXXFLAGS", "")} -DVERSION_INFO=\\"{self.distribution.get_version()}\\"'

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        # Configure
        subprocess.check_call(
            ["cmake", ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env
        )

        # Build
        subprocess.check_call(
            ["cmake", "--build", "."] + build_args, cwd=self.build_temp
        )


def read_long_description():
    """Read the long description from README"""
    readme_path = Path(__file__).parent / "README.md"
    if readme_path.exists():
        return readme_path.read_text(encoding="utf-8")
    return "Automatic Python bindings for C++ using Rosetta introspection"


setup(
    name="rosetta-python",
    version="1.0.0",
    author="Rosetta Team",
    author_email="contact@rosetta.dev",
    description="Automatic Python bindings for C++ using Rosetta introspection",
    long_description=read_long_description(),
    long_description_content_type="text/markdown",
    url="https://github.com/rosetta/rosetta-python",
    ext_modules=[
        CMakeExtension("example_manual"),
        CMakeExtension("example_macro"),
        CMakeExtension("example_with_functions"),
        CMakeExtension("example_custom_converter"),
    ],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    python_requires=">=3.7",
    install_requires=[
        # No Python dependencies for the bindings themselves
    ],
    extras_require={
        "dev": [
            "pytest>=7.0",
            "pytest-cov>=3.0",
            "black>=22.0",
            "mypy>=0.950",
            "pybind11-stubgen>=0.16",
        ],
        "docs": [
            "sphinx>=4.5",
            "sphinx-rtd-theme>=1.0",
            "breathe>=4.33",
        ],
    },
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: C++",
    ],
    project_urls={
        "Bug Reports": "https://github.com/rosetta/rosetta-python/issues",
        "Source": "https://github.com/rosetta/rosetta-python",
        "Documentation": "https://rosetta-python.readthedocs.io",
    },
)