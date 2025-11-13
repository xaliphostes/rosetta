#!/usr/bin/env python3
"""
Rosetta IDL Parser
==================

Parser for Rosetta Interface Language Description (ILD) files.
Converts YAML interface descriptions into structured Python objects
for code generation.

Author: xaliphostes
License: LGPL3
"""

import yaml
from typing import Optional, List, Dict, Any, Union
from dataclasses import dataclass, field
from pathlib import Path
from enum import Enum


# ============================================================================
# Enumerations
# ============================================================================

class AccessMode(Enum):
    """Field access modes"""
    READ_WRITE = "rw"
    READ_ONLY = "ro"
    WRITE_ONLY = "wo"


class InheritanceType(Enum):
    """Inheritance types"""
    NORMAL = "normal"
    VIRTUAL = "virtual"


class AccessSpecifier(Enum):
    """C++ access specifiers"""
    PUBLIC = "public"
    PROTECTED = "protected"
    PRIVATE = "private"


class LibraryType(Enum):
    """Library types"""
    STATIC = "static"
    SHARED = "shared"
    SYSTEM = "system"  # System library (e.g., pthread, m, dl)


# ============================================================================
# Data Classes for IDL Structure
# ============================================================================

@dataclass
class LibraryDependency:
    """Library dependency specification"""
    name: str
    type: LibraryType = LibraryType.SHARED
    optional: bool = False
    system: bool = False  # True for system libraries (pthread, etc.)
    
    @classmethod
    def from_dict(cls, data: Union[str, Dict[str, Any]]) -> 'LibraryDependency':
        if isinstance(data, str):
            return cls(name=data)
        
        lib_type = LibraryType.SYSTEM if data.get('system', False) else LibraryType(data.get('type', 'shared'))
        
        return cls(
            name=data['name'],
            type=lib_type,
            optional=data.get('optional', False),
            system=data.get('system', False)
        )


@dataclass
class LibraryConfig:
    """Library configuration for binding generation"""
    name: str
    type: LibraryType = LibraryType.STATIC
    filename: Optional[str] = None  # Exact filename if different from name
    search_paths: List[str] = field(default_factory=list)
    cmake_var: Optional[str] = None  # CMake variable name for path override
    dependencies: List[LibraryDependency] = field(default_factory=list)
    
    @property
    def lib_filename(self) -> str:
        """Get the library filename based on platform conventions"""
        if self.filename:
            return self.filename
        
        # Generate platform-appropriate name
        if self.type == LibraryType.STATIC:
            # Unix: libname.a, Windows: name.lib
            return f"lib{self.name}.a"  # Default to Unix
        else:
            # Unix: libname.so, macOS: libname.dylib, Windows: name.dll
            return f"lib{self.name}.so"  # Default to Unix
    
    @property
    def cmake_variable_name(self) -> str:
        """Get the CMake variable name for library path"""
        if self.cmake_var:
            return self.cmake_var
        return f"{self.name.upper()}_LIB_PATH"
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'LibraryConfig':
        lib_type = LibraryType(data.get('type', 'static'))
        
        deps = [
            LibraryDependency.from_dict(d) 
            for d in data.get('dependencies', [])
        ]
        
        return cls(
            name=data['name'],
            type=lib_type,
            filename=data.get('filename'),
            search_paths=data.get('search_paths', []),
            cmake_var=data.get('cmake_var'),
            dependencies=deps
        )

@dataclass
class Parameter:
    """Function/method parameter"""
    name: str
    type: str
    default: Optional[str] = None
    description: Optional[str] = None

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Parameter':
        return cls(
            name=data['name'],
            type=data['type'],
            default=data.get('default'),
            description=data.get('description')
        )


@dataclass
class Field:
    """Class field/property"""
    name: str
    type: str
    access: AccessMode = AccessMode.READ_WRITE
    description: Optional[str] = None
    default: Optional[str] = None
    cpp_name: Optional[str] = None  # If C++ name differs
    getter: Optional[str] = None  # Optional getter method name
    setter: Optional[str] = None  # Optional setter method name

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Field':
        access_str = data.get('access', 'rw')
        access = AccessMode(access_str)
        
        return cls(
            name=data['name'],
            type=data['type'],
            access=access,
            description=data.get('description'),
            default=data.get('default'),
            cpp_name=data.get('cpp_name'),
            getter=data.get('getter'),
            setter=data.get('setter')
        )


@dataclass
class Method:
    """Class method"""
    name: str
    returns: str = "void"
    parameters: List[Parameter] = field(default_factory=list)
    const: bool = False
    virtual: bool = False
    pure_virtual: bool = False
    override: bool = False
    static: bool = False
    description: Optional[str] = None
    cpp_name: Optional[str] = None  # If C++ name differs

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Method':
        params = [
            Parameter.from_dict(p) for p in data.get('parameters', [])
        ]
        
        return cls(
            name=data['name'],
            returns=data.get('returns', 'void'),
            parameters=params,
            const=data.get('const', False),
            virtual=data.get('virtual', False),
            pure_virtual=data.get('pure_virtual', False),
            override=data.get('override', False),
            static=data.get('static', False),
            description=data.get('description'),
            cpp_name=data.get('cpp_name')
        )


@dataclass
class Constructor:
    """Class constructor"""
    parameters: List[Parameter] = field(default_factory=list)
    description: Optional[str] = None

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Constructor':
        params = [
            Parameter.from_dict(p) for p in data.get('parameters', [])
        ]
        
        return cls(
            parameters=params,
            description=data.get('description')
        )


@dataclass
class BaseClass:
    """Base class information"""
    name: str
    inheritance_type: InheritanceType = InheritanceType.NORMAL
    access: AccessSpecifier = AccessSpecifier.PUBLIC

    @classmethod
    def from_dict(cls, data: Union[str, Dict[str, Any]]) -> 'BaseClass':
        if isinstance(data, str):
            return cls(name=data)
        
        inheritance_str = data.get('inheritance', 'normal')
        inheritance = InheritanceType(inheritance_str)
        
        access_str = data.get('access', 'public')
        access = AccessSpecifier(access_str)
        
        return cls(
            name=data['name'],
            inheritance_type=inheritance,
            access=access
        )


@dataclass
class Class:
    """Class definition"""
    name: str
    cpp_class: Optional[str] = None  # If C++ class name differs
    description: Optional[str] = None
    
    # Class members
    fields: List[Field] = field(default_factory=list)
    methods: List[Method] = field(default_factory=list)
    constructors: List[Constructor] = field(default_factory=list)
    
    # Inheritance
    base_classes: List[BaseClass] = field(default_factory=list)
    
    # Properties
    is_abstract: bool = False
    is_polymorphic: bool = False
    
    # Auto-detection options
    auto_detect_properties: bool = False

    @property
    def cpp_name(self) -> str:
        """Get the C++ class name"""
        return self.cpp_class or self.name

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Class':
        fields = [Field.from_dict(f) for f in data.get('fields', [])]
        methods = [Method.from_dict(m) for m in data.get('methods', [])]
        constructors = [Constructor.from_dict(c) for c in data.get('constructors', [])]
        
        base_classes = [
            BaseClass.from_dict(b) for b in data.get('base_classes', [])
        ]
        
        return cls(
            name=data['name'],
            cpp_class=data.get('cpp_class'),
            description=data.get('description'),
            fields=fields,
            methods=methods,
            constructors=constructors,
            base_classes=base_classes,
            is_abstract=data.get('is_abstract', False),
            is_polymorphic=data.get('is_polymorphic', False),
            auto_detect_properties=data.get('auto_detect_properties', False)
        )


@dataclass
class Function:
    """Free function"""
    name: str
    cpp_name: Optional[str] = None
    returns: str = "void"
    parameters: List[Parameter] = field(default_factory=list)
    description: Optional[str] = None

    @property
    def cpp_function_name(self) -> str:
        """Get the C++ function name"""
        return self.cpp_name or self.name

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Function':
        params = [
            Parameter.from_dict(p) for p in data.get('parameters', [])
        ]
        
        return cls(
            name=data['name'],
            cpp_name=data.get('cpp_name'),
            returns=data.get('returns', 'void'),
            parameters=params,
            description=data.get('description')
        )


@dataclass
class TypeConverter:
    """Type converter registration"""
    type: str
    custom_converter: Optional[str] = None  # Custom converter function

    @classmethod
    def from_dict(cls, data: Union[str, Dict[str, Any]]) -> 'TypeConverter':
        if isinstance(data, str):
            return cls(type=data)
        
        return cls(
            type=data['type'],
            custom_converter=data.get('custom_converter')
        )


@dataclass
class Include:
    """Header include"""
    path: str
    system: bool = False  # True for <>, False for ""

    @property
    def is_system(self) -> bool:
        """Check if this is a system include"""
        return self.system or self.path.startswith('<')

    @property
    def formatted(self) -> str:
        """Get formatted include statement"""
        if self.is_system:
            path = self.path.strip('<>')
            return f"#include <{path}>"
        else:
            return f'#include "{self.path}"'

    @classmethod
    def from_dict(cls, data: Union[str, Dict[str, Any]]) -> 'Include':
        if isinstance(data, str):
            # Auto-detect system includes
            system = data.startswith('<') and data.endswith('>')
            return cls(path=data, system=system)
        
        return cls(
            path=data['path'],
            system=data.get('system', False)
        )


@dataclass
class Module:
    """Module configuration"""
    name: str
    version: str = "1.0.0"
    namespace: Optional[str] = None
    description: Optional[str] = None

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Module':
        return cls(
            name=data['name'],
            version=data.get('version', '1.0.0'),
            namespace=data.get('namespace'),
            description=data.get('description')
        )


@dataclass
class Utilities:
    """Utility functions configuration"""
    version_info: bool = True
    list_classes: bool = True
    type_inspection: bool = True
    get_class_info: bool = True

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Utilities':
        return cls(
            version_info=data.get('version_info', True),
            list_classes=data.get('list_classes', True),
            type_inspection=data.get('type_inspection', True),
            get_class_info=data.get('get_class_info', True)
        )


@dataclass
class InterfaceDefinition:
    """Complete interface definition"""
    module: Module
    includes: List[Include] = field(default_factory=list)
    converters: List[TypeConverter] = field(default_factory=list)
    classes: List[Class] = field(default_factory=list)
    functions: List[Function] = field(default_factory=list)
    utilities: Optional[Utilities] = None
    library: Optional[LibraryConfig] = None  # Library configuration for binding generation

    def get_class(self, name: str) -> Optional[Class]:
        """Get a class by name"""
        for cls in self.classes:
            if cls.name == name or cls.cpp_name == name:
                return cls
        return None

    def get_function(self, name: str) -> Optional[Function]:
        """Get a function by name"""
        for func in self.functions:
            if func.name == name or func.cpp_function_name == name:
                return func
        return None


# ============================================================================
# IDL Parser
# ============================================================================

class IDLParser:
    """Parser for Rosetta Interface Language Description files"""

    def __init__(self):
        self.errors: List[str] = []
        self.warnings: List[str] = []

    def parse_file(self, filepath: Union[str, Path]) -> Optional[InterfaceDefinition]:
        """
        Parse an IDL file
        
        Args:
            filepath: Path to the YAML IDL file
            
        Returns:
            InterfaceDefinition object or None if parsing failed
        """
        filepath = Path(filepath)
        
        if not filepath.exists():
            self.errors.append(f"File not found: {filepath}")
            return None
        
        try:
            with open(filepath, 'r') as f:
                data = yaml.safe_load(f)
            
            return self.parse_dict(data)
            
        except yaml.YAMLError as e:
            self.errors.append(f"YAML parsing error: {e}")
            return None
        except Exception as e:
            self.errors.append(f"Unexpected error: {e}")
            return None

    def parse_string(self, yaml_content: str) -> Optional[InterfaceDefinition]:
        """
        Parse IDL from a YAML string
        
        Args:
            yaml_content: YAML content as string
            
        Returns:
            InterfaceDefinition object or None if parsing failed
        """
        try:
            data = yaml.safe_load(yaml_content)
            return self.parse_dict(data)
        except yaml.YAMLError as e:
            self.errors.append(f"YAML parsing error: {e}")
            return None
        except Exception as e:
            self.errors.append(f"Unexpected error: {e}")
            return None

    def parse_dict(self, data: Dict[str, Any]) -> Optional[InterfaceDefinition]:
        """
        Parse IDL from a dictionary
        
        Args:
            data: Parsed YAML data
            
        Returns:
            InterfaceDefinition object or None if parsing failed
        """
        self.errors.clear()
        self.warnings.clear()
        
        try:
            # Parse module (required)
            if 'module' not in data:
                self.errors.append("Missing required 'module' section")
                return None
            
            module = Module.from_dict(data['module'])
            
            # Parse includes
            includes = []
            for inc in data.get('includes', []):
                includes.append(Include.from_dict(inc))
            
            # Parse converters
            converters = []
            for conv in data.get('converters', []):
                converters.append(TypeConverter.from_dict(conv))
            
            # Parse classes
            classes = []
            for cls_data in data.get('classes', []):
                try:
                    cls = Class.from_dict(cls_data)
                    classes.append(cls)
                except Exception as e:
                    self.errors.append(f"Error parsing class '{cls_data.get('name', '?')}': {e}")
            
            # Parse functions
            functions = []
            for func_data in data.get('functions', []):
                try:
                    func = Function.from_dict(func_data)
                    functions.append(func)
                except Exception as e:
                    self.errors.append(f"Error parsing function '{func_data.get('name', '?')}': {e}")
            
            # Parse utilities
            utilities = None
            if 'utilities' in data:
                utilities = Utilities.from_dict(data['utilities'])
            
            # Parse library configuration
            library = None
            if 'library' in data:
                try:
                    library = LibraryConfig.from_dict(data['library'])
                except Exception as e:
                    self.errors.append(f"Error parsing library configuration: {e}")
            
            if self.errors:
                return None
            
            return InterfaceDefinition(
                module=module,
                includes=includes,
                converters=converters,
                classes=classes,
                functions=functions,
                utilities=utilities,
                library=library
            )
            
        except Exception as e:
            self.errors.append(f"Error creating interface definition: {e}")
            return None

    def validate(self, idl: InterfaceDefinition) -> bool:
        """
        Validate an interface definition
        
        Args:
            idl: Interface definition to validate
            
        Returns:
            True if valid, False otherwise
        """
        # Check for duplicate class names
        class_names = [cls.name for cls in idl.classes]
        if len(class_names) != len(set(class_names)):
            self.errors.append("Duplicate class names found")
            return False
        
        # Check for duplicate function names
        func_names = [func.name for func in idl.functions]
        if len(func_names) != len(set(func_names)):
            self.errors.append("Duplicate function names found")
            return False
        
        # Validate inheritance
        for cls in idl.classes:
            for base in cls.base_classes:
                if not idl.get_class(base.name):
                    self.warnings.append(
                        f"Class '{cls.name}' inherits from '{base.name}' "
                        f"which is not defined in this IDL"
                    )
        
        return len(self.errors) == 0

    def has_errors(self) -> bool:
        """Check if there are any errors"""
        return len(self.errors) > 0

    def has_warnings(self) -> bool:
        """Check if there are any warnings"""
        return len(self.warnings) > 0

    def print_diagnostics(self):
        """Print all errors and warnings"""
        if self.errors:
            print("\nErrors:")
            for error in self.errors:
                print(f"{error}")
        
        if self.warnings:
            print("\nWarnings:")
            for warning in self.warnings:
                print(f"{warning}")


# ============================================================================
# Code Generation Helpers
# ============================================================================

def parse_idl(filepath: Union[str, Path]) -> Optional[InterfaceDefinition]:
    """
    Convenience function to parse and validate an IDL file
    
    Args:
        filepath: Path to the IDL file
        
    Returns:
        InterfaceDefinition or None if parsing/validation failed
    """
    parser = IDLParser()
    idl = parser.parse_file(filepath)
    
    if not idl:
        parser.print_diagnostics()
        return None
    
    if not parser.validate(idl):
        parser.print_diagnostics()
        return None
    
    if parser.has_errors():
        parser.print_diagnostics()
        return None
    
    if parser.has_warnings():
        parser.print_diagnostics()
    
    return idl


def write_rosetta_bindings(idl: InterfaceDefinition, output_dir: Path):
    """
    Write Rosetta binding registration code
    
    Args:
        idl: Interface definition
        output_dir: Output directory for generated files
    """
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Generate the registration file (*.cxx)
    registration_file = output_dir / f"{idl.module.name}_registration.cxx"
    
    with open(registration_file, 'w') as f:
        # Header
        f.write(f"// Auto-generated Rosetta registration for {idl.module.name}\n")
        f.write(f"// Generated by Rosetta IDL Parser\n")
        f.write(f"// DO NOT EDIT THIS FILE MANUALLY\n\n")
        
        # Includes
        f.write("#include <rosetta/rosetta.h>\n")
        for include in idl.includes:
            f.write(f"{include.formatted}\n")
        f.write("\n")
        
        # Namespace
        if idl.module.namespace:
            f.write(f"using namespace {idl.module.namespace};\n\n")
        
        # Registration function
        f.write(f"void register_{idl.module.name}_classes() {{\n")
        
        # Register each class
        for cls in idl.classes:
            _write_class_registration(f, cls, idl)
        
        f.write("}\n")
    
    print(f"Generated Rosetta registration: {registration_file}")
    return registration_file


def write_cmake_for_python(idl: InterfaceDefinition, output_dir: Path, binding_file: Path):
    """
    Write CMakeLists.txt for Python binding compilation
    
    Args:
        idl: Interface definition
        output_dir: Output directory for CMakeLists.txt
        binding_file: Path to the binding .cpp file
    """
    cmake_file = output_dir / "CMakeLists.txt"
    module_name = f"{idl.module.name}"
    binding_filename = binding_file.name
    registration_filename = f"{idl.module.name}_registration.cxx"
    lib_name = f"lib{idl.module.name}.a"
    
    with open(cmake_file, 'w') as f:
        f.write(f"# Auto-generated CMakeLists.txt for {idl.module.name} Python bindings\n")
        f.write(f"# Generated by Rosetta IDL Parser\n\n")
        
        f.write(f"cmake_minimum_required(VERSION 3.15)\n")
        f.write(f"project(rosetta_{module_name})\n\n")
        
        # C++ standard
        f.write(f"# C++ standard\n")
        f.write(f"set(CMAKE_CXX_STANDARD 20)\n")
        f.write(f"set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n")
        
        # FetchContent for pybind11
        f.write(f"# Fetch pybind11\n")
        f.write(f"include(FetchContent)\n")
        f.write(f"if(WIN32)\n")
        f.write(f"    # Option 1: Set Python path explicitly\n")
        f.write(f"    # set(Python_EXECUTABLE \"C:/Python313/\")\n")
        f.write(f"    \n")
        f.write(f"    # Option 2: Use py launcher to find Python\n")
        f.write(f"    execute_process(\n")
        f.write(f"        COMMAND py -3 -c \"import sys; print(sys.executable)\"\n")
        f.write(f"        OUTPUT_VARIABLE PYTHON_FROM_PY\n")
        f.write(f"        OUTPUT_STRIP_TRAILING_WHITESPACE\n")
        f.write(f"        ERROR_QUIET\n")
        f.write(f"    )\n")
        f.write(f"    if(PYTHON_FROM_PY)\n")
        f.write(f"        set(Python_EXECUTABLE \"${{PYTHON_FROM_PY}}\")\n")
        f.write(f"        message(STATUS \"Detected Python executable: ${{PYTHON_FROM_PY}}\")\n")
        f.write(f"    endif()\n")
        f.write(f"endif()\n\n")
        
        f.write(f"FetchContent_Declare(\n")
        f.write(f"    pybind11\n")
        f.write(f"    GIT_REPOSITORY https://github.com/pybind/pybind11.git\n")
        f.write(f"    GIT_TAG v3.0.1\n")
        f.write(f")\n")
        f.write(f"FetchContent_MakeAvailable(pybind11)\n\n")
        
        # Include directories
        f.write(f"# =============================================================================\n")
        f.write(f"# Include directories for Rosetta headers\n")
        f.write(f"# =============================================================================\n")
        f.write(f"include_directories(\n")
        f.write(f"    ${{CMAKE_CURRENT_SOURCE_DIR}}\n")
        f.write(f"    ${{CMAKE_CURRENT_SOURCE_DIR}}/..\n")
        f.write(f"    ${{CMAKE_CURRENT_SOURCE_DIR}}/../../../include\n")
        f.write(f"    ${{Python_INCLUDE_DIRS}}\n")
        
        # Add include paths from IDL
        user_includes = [inc for inc in idl.includes if not inc.is_system]
        if user_includes:
            f.write(f"    # User includes from IDL\n")
            for inc in user_includes:
                # Extract directory from include path
                inc_dir = Path(inc.path).parent
                if inc_dir != Path('.'):
                    f.write(f"    ${{CMAKE_CURRENT_SOURCE_DIR}}/{inc_dir}\n")
        
        f.write(f")\n\n")
        
        # Create Python module
        f.write(f"# =============================================================================\n")
        f.write(f"# Create the Python module\n")
        f.write(f"# =============================================================================\n")
        f.write(f"pybind11_add_module({module_name}\n")
        f.write(f"    {binding_filename}\n")
        f.write(f"    {registration_filename}\n")
        f.write(f")\n\n")
        
        # Target include directories
        f.write(f"# Set include directories for the target\n")
        f.write(f"target_include_directories({module_name} PRIVATE\n")
        f.write(f"    ${{CMAKE_CURRENT_SOURCE_DIR}}\n")
        f.write(f"    ${{CMAKE_CURRENT_SOURCE_DIR}}/..\n")
        f.write(f")\n\n")
        
        # Link with Python libraries on Windows
        f.write(f"# Link with Python libraries if needed (Windows)\n")
        f.write(f"if(WIN32 AND Python_LIBRARIES)\n")
        f.write(f"    target_link_libraries({module_name} PRIVATE ${{Python_LIBRARIES}})\n")
        f.write(f"endif()\n\n")
        
        # Link with third-party library
        f.write(f"# =============================================================================\n")
        f.write(f"# Link with third-party libraries\n")
        f.write(f"# =============================================================================\n")
        
        if idl.library:
            lib_config = idl.library
            lib_name = lib_config.lib_filename
            cmake_var = lib_config.cmake_variable_name
            
            # Add search paths as hints
            if lib_config.search_paths:
                f.write(f"# Search paths for {lib_config.name}\n")
                f.write(f"set({lib_config.name.upper()}_SEARCH_PATHS\n")
                for search_path in lib_config.search_paths:
                    f.write(f"    {search_path}\n")
                f.write(f")\n\n")
            
            # Try to find the library
            f.write(f"# Find {lib_config.name} library\n")
            if lib_config.search_paths:
                f.write(f"find_library({lib_config.name.upper()}_LIB\n")
                f.write(f"    NAMES {lib_config.name}\n")
                f.write(f"    PATHS ${{{lib_config.name.upper()}_SEARCH_PATHS}}\n")
                f.write(f"    NO_DEFAULT_PATH\n")
                f.write(f")\n\n")
            
            # Allow manual override via CMake variable
            f.write(f"set({cmake_var} \"${{{lib_config.name.upper()}_LIB}}\" CACHE FILEPATH \"Path to {lib_name}\")\n\n")
            
            # Link if found
            f.write(f"if(EXISTS ${{{cmake_var}}})\n")
            f.write(f"    message(STATUS \"Found {lib_config.name} library at: ${{{cmake_var}}}\")\n")
            f.write(f"    target_link_libraries({module_name} PUBLIC ${{{cmake_var}}})\n")
            f.write(f"else()\n")
            f.write(f"    message(WARNING \"{lib_name} not found\")\n")
            if lib_config.search_paths:
                f.write(f"    message(WARNING \"Searched in: ${{{lib_config.name.upper()}_SEARCH_PATHS}}\")\n")
            f.write(f"    message(WARNING \"Specify manually with: cmake -D{cmake_var}=/path/to/{lib_name}\")\n")
            f.write(f"endif()\n\n")
            
            # Link dependencies
            if lib_config.dependencies:
                f.write(f"# Link dependencies\n")
                for dep in lib_config.dependencies:
                    if dep.system:
                        # System library - use find_library or direct linking
                        f.write(f"target_link_libraries({module_name} PUBLIC {dep.name})\n")
                    else:
                        # Another library - similar process
                        dep_cmake_var = f"{dep.name.upper()}_LIB_PATH"
                        f.write(f"if(EXISTS ${{{dep_cmake_var}}})\n")
                        f.write(f"    target_link_libraries({module_name} PUBLIC ${{{dep_cmake_var}}})\n")
                        if not dep.optional:
                            f.write(f"else()\n")
                            f.write(f"    message(WARNING \"Required dependency {dep.name} not found\")\n")
                        f.write(f"endif()\n")
                f.write("\n")
        else:
            # Fallback to old behavior if no library config
            lib_name = f"lib{idl.module.name}.a"
            f.write(f"# No library configuration in IDL - using default\n")
            f.write(f"set({idl.module.name.upper()}_LIB_PATH \"\" CACHE FILEPATH \"Path to {lib_name}\")\n\n")
            f.write(f"if(EXISTS ${{{idl.module.name.upper()}_LIB_PATH}})\n")
            f.write(f"    message(STATUS \"Found {idl.module.name} library at: ${{{idl.module.name.upper()}_LIB_PATH}}\")\n")
            f.write(f"    target_link_libraries({module_name} PUBLIC ${{{idl.module.name.upper()}_LIB_PATH}})\n")
            f.write(f"else()\n")
            f.write(f"    message(WARNING \"{lib_name} not found at: ${{{idl.module.name.upper()}_LIB_PATH}}\")\n")
            f.write(f"    message(WARNING \"Specify with: cmake -D{idl.module.name.upper()}_LIB_PATH=/path/to/{lib_name}\")\n")
            f.write(f"endif()\n\n")
        
        # Set output directory
        f.write(f"# Set output directory\n")
        f.write(f"set_target_properties({module_name} PROPERTIES\n")
        f.write(f"    LIBRARY_OUTPUT_DIRECTORY \"${{CMAKE_BINARY_DIR}}\"\n")
        f.write(f"    RUNTIME_OUTPUT_DIRECTORY \"${{CMAKE_BINARY_DIR}}\"\n")
        f.write(f")\n\n")
        
        # Windows .pyd extension
        f.write(f"# On Windows, ensure .pyd extension\n")
        f.write(f"if(WIN32)\n")
        f.write(f"    set_target_properties({module_name} PROPERTIES SUFFIX \".pyd\")\n")
        f.write(f"endif()\n\n")
        
        # Installation
        f.write(f"# =============================================================================\n")
        f.write(f"# Installation\n")
        f.write(f"# =============================================================================\n")
        f.write(f"install(TARGETS {module_name}\n")
        f.write(f"    LIBRARY DESTINATION .\n")
        f.write(f"    RUNTIME DESTINATION .\n")
        f.write(f")\n\n")
        
        # Copy test script
        f.write(f"# =============================================================================\n")
        f.write(f"# Copy test script to build directory\n")
        f.write(f"# =============================================================================\n")
        f.write(f"if(EXISTS ${{CMAKE_CURRENT_SOURCE_DIR}}/test.py)\n")
        f.write(f"    configure_file(\n")
        f.write(f"        ${{CMAKE_CURRENT_SOURCE_DIR}}/test.py\n")
        f.write(f"        ${{CMAKE_BINARY_DIR}}/test.py\n")
        f.write(f"        COPYONLY\n")
        f.write(f"    )\n")
        f.write(f"    message(STATUS \"Copied test.py to build directory\")\n")
        f.write(f"endif()\n")
    
    print(f"Generated CMakeLists.txt: {cmake_file}")
    return cmake_file


def _write_class_registration(f, cls: Class, idl: InterfaceDefinition):
    """Write registration code for a single class"""
    f.write(f"    // Register {cls.name}\n")
    f.write(f"    ROSETTA_REGISTER_CLASS({cls.cpp_name})\n")
    
    # Constructors
    for ctor in cls.constructors:
        if ctor.parameters:
            param_types = ", ".join([p.type for p in ctor.parameters])
            f.write(f"        .constructor<{param_types}>()\n")
        else:
            f.write(f"        .constructor<>()\n")
    
    # Base classes
    for base in cls.base_classes:
        if base.inheritance_type == InheritanceType.VIRTUAL:
            f.write(f"        .virtually_inherits_from<{base.name}>()\n")
        else:
            f.write(f"        .inherits_from<{base.name}>()\n")
    
    # Fields
    for field in cls.fields:
        cpp_field_name = field.cpp_name or field.name
        
        if field.getter and field.setter:
            # Property with getter/setter - NO type parameter
            f.write(f"        .property(\"{field.name}\", "
                   f"&{cls.cpp_name}::{field.getter}, "
                   f"&{cls.cpp_name}::{field.setter})\n")
        elif field.getter and field.access == AccessMode.READ_ONLY:
            # Read-only property - NO type parameter
            f.write(f"        .readonly_property(\"{field.name}\", "
                   f"&{cls.cpp_name}::{field.getter})\n")
        elif field.setter and field.access == AccessMode.WRITE_ONLY:
            # Write-only property - NO type parameter
            f.write(f"        .writeonly_property(\"{field.name}\", "
                   f"&{cls.cpp_name}::{field.setter})\n")
        else:
            # Direct field access
            f.write(f"        .field(\"{field.name}\", &{cls.cpp_name}::{cpp_field_name})\n")
    
    # Methods - with overload detection and static method support
    # First, group methods by name to detect overloads
    method_groups = {}
    for method in cls.methods:
        method_name = method.name
        if method_name not in method_groups:
            method_groups[method_name] = []
        method_groups[method_name].append(method)
    
    # Now write each method with appropriate handling
    # Track which methods we've already registered to avoid duplicates
    registered_methods = set()
    
    for method in cls.methods:
        cpp_method_name = method.cpp_name or method.name
        is_overloaded = len(method_groups[method.name]) > 1
        
        # Create unique key for this method signature
        param_sig = ",".join([p.type for p in method.parameters])
        method_key = f"{method.name}::{param_sig}::{method.const}"
        
        # Skip if already registered
        if method_key in registered_methods:
            continue
        registered_methods.add(method_key)
        
        # For overloaded methods, create unique Python name by appending parameter info
        py_method_name = method.name
        if is_overloaded:
            # Create a unique suffix based on parameter types
            if len(method.parameters) == 0:
                suffix = "_void"
            else:
                # Simplify type names for suffix
                suffix_parts = []
                for param in method.parameters:
                    ptype = param.type
                    # Extract base type name (remove const, &, *, std::, etc.)
                    ptype = ptype.replace("const ", "").replace("&", "").replace("*", "").strip()
                    ptype = ptype.replace("std::", "")
                    # Take last part after ::
                    if "::" in ptype:
                        ptype = ptype.split("::")[-1]
                    # Remove template arguments for simplicity
                    if "<" in ptype:
                        ptype = ptype[:ptype.index("<")]
                    suffix_parts.append(ptype.lower())
                suffix = "_" + "_".join(suffix_parts)
            py_method_name = method.name + suffix
        
        if method.pure_virtual:
            # Pure virtual method
            param_types_str = ", ".join([method.returns] + [p.type for p in method.parameters])
            f.write(f"        .pure_virtual_method<{param_types_str}>(\"{py_method_name}\")\n")
        elif method.static:
            # Static method - use .static_method() instead of .method()
            method_ptr = _generate_method_pointer(cls.cpp_name, method, is_overloaded, is_static=True)
            f.write(f"        .static_method(\"{py_method_name}\", {method_ptr})\n")
        elif method.override:
            # Override method
            method_ptr = _generate_method_pointer(cls.cpp_name, method, is_overloaded)
            f.write(f"        .override_method(\"{py_method_name}\", {method_ptr})\n")
        elif method.virtual:
            # Virtual method
            method_ptr = _generate_method_pointer(cls.cpp_name, method, is_overloaded)
            f.write(f"        .virtual_method(\"{py_method_name}\", {method_ptr})\n")
        else:
            # Regular method - ALWAYS use explicit cast for overloaded methods
            method_ptr = _generate_method_pointer(cls.cpp_name, method, is_overloaded)
            f.write(f"        .method(\"{py_method_name}\", {method_ptr})\n")
    
    # Auto-detect properties
    if cls.auto_detect_properties:
        f.write(f"        .auto_detect_properties()\n")
    
    f.write("        ;\n\n")


def _generate_method_pointer(class_name: str, method: Method, is_overloaded: bool, is_static: bool = False) -> str:
    """
    Generate method pointer with explicit cast if needed for overloaded methods.
    
    Args:
        class_name: Name of the class
        method: Method object
        is_overloaded: Whether this method name is overloaded
        is_static: Whether this is a static method
    
    Returns:
        String representing the method pointer (with cast if needed)
    """
    cpp_method_name = method.cpp_name or method.name
    
    # If not overloaded, simple pointer is enough
    if not is_overloaded:
        return f"&{class_name}::{cpp_method_name}"
    
    # For overloaded methods, generate explicit cast
    return_type = method.returns
    
    # Build parameter type list
    if method.parameters:
        param_types = ", ".join([p.type for p in method.parameters])
    else:
        param_types = ""
    
    # Generate the appropriate signature based on method type
    if is_static:
        # Static method: ReturnType (*)(ParamTypes...)
        if param_types:
            signature = f"{return_type} (*)({param_types})"
        else:
            signature = f"{return_type} (*)()"
    else:
        # Member method: ReturnType (ClassName::*)(ParamTypes...)
        if param_types:
            signature = f"{return_type} ({class_name}::*)({param_types})"
        else:
            signature = f"{return_type} ({class_name}::*)()"
        
        # Add const qualifier if needed
        if method.const:
            signature += " const"
    
    # Return the cast
    return f"static_cast<{signature}>(&{class_name}::{cpp_method_name})"


if __name__ == "__main__":
    import sys
    import argparse
    
    parser = argparse.ArgumentParser(
        description='Rosetta IDL Parser - Parse YAML interface descriptions and generate bindings',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Parse and validate an IDL file
  python rosetta_idl_parser.py interface.yaml
  
  # Generate Rosetta registration code
  python rosetta_idl_parser.py interface.yaml -o output/
  
  # Generate both Rosetta and Python bindings
  python rosetta_idl_parser.py interface.yaml -o output/ --python
        """
    )
    
    parser.add_argument('idl_file', help='Path to the YAML IDL file')
    parser.add_argument('-o', '--output-dir', dest='output_dir',
                        help='Output directory for generated files (default: current directory)',
                        default='.')
    parser.add_argument('--python', action='store_true',
                        help='Also generate Python bindings')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Verbose output')
    parser.add_argument('--validate-only', action='store_true',
                        help='Only validate the IDL, do not generate code')
    
    args = parser.parse_args()
    
    # Parse the IDL file
    if args.verbose:
        print(f"Parsing IDL file: {args.idl_file}")
    
    idl_parser = IDLParser()
    idl = idl_parser.parse_file(args.idl_file)
    
    if not idl:
        print("Failed to parse IDL file")
        idl_parser.print_diagnostics()
        sys.exit(1)
    
    # Display parsed info
    print(f"Successfully parsed IDL: {idl.module.name}")
    print(f"   Version: {idl.module.version}")
    if idl.module.namespace:
        print(f"   Namespace: {idl.module.namespace}")
    print(f"   Classes: {len(idl.classes)}")
    print(f"   Functions: {len(idl.functions)}")
    print(f"   Converters: {len(idl.converters)}")
    
    if args.verbose:
        print("\nClasses:")
        for cls in idl.classes:
            print(f"  - {cls.name}")
            if cls.constructors:
                print(f"    Constructors: {len(cls.constructors)}")
            if cls.fields:
                print(f"    Fields: {len(cls.fields)}")
            if cls.methods:
                print(f"    Methods: {len(cls.methods)}")
    
    # Validate
    if not idl_parser.validate(idl):
        print("\nValidation failed:")
        idl_parser.print_diagnostics()
        sys.exit(1)
    
    print("Validation passed")
    
    if args.validate_only:
        print("\nValidation only - no code generated")
        sys.exit(0)
    
    # Generate Rosetta registration code
    try:
        output_path = Path(args.output_dir)
        registration_file = write_rosetta_bindings(idl, output_path)
        
        # Generate Python bindings if requested
        if args.python:
            from py_binding_generator import generate_python_bindings
            
            binding_file = output_path / f"{idl.module.name}_binding.cpp"
            if generate_python_bindings(Path(args.idl_file), binding_file):
                print(f"Generated Python bindings: {binding_file}")
                
                # Generate CMakeLists.txt for Python binding
                cmake_file = write_cmake_for_python(idl, output_path, binding_file)
        
        print(f"\nCode generation complete!")
        # print(f"   Output directory: {output_path.resolve()}")
        
        # if args.python:
        #     print(f"\n“ To build the Python module:")
        #     print(f"   cd {output_path.resolve()}")
        #     print(f"   mkdir build && cd build")
        #     print(f"   cmake ..")
        #     print(f"   make")
        
    except Exception as e:
        print(f"\nCode generation failed: {e}")
        import traceback
        if args.verbose:
            traceback.print_exc()
        sys.exit(1)