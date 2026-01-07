"""
Rosetta Create - Project scaffolding tool for Rosetta binding projects.

This tool generates a complete project skeleton for creating C++ bindings
using the Rosetta introspection library.
"""

__version__ = "1.0.0"
__author__ = "xaliphostes"

from .generator import ProjectGenerator

__all__ = ["ProjectGenerator"]
