#!/usr/bin/env python3
"""
Rosetta ILD Parser
Parses Interface Language Description files and generates binding code
"""

import json
from dataclasses import dataclass, field
from typing import List, Dict, Optional, Any
from pathlib import Path

# Try to import yaml, but make it optional
try:
    import yaml
    HAS_YAML = True
except ImportError:
    HAS_YAML = False
    print("Warning: PyYAML not installed. Only JSON format will be supported.")
    print("Install with: pip install pyyaml --break-system-packages")


@dataclass
class Parameter:
    """Function/method parameter"""
    name: str
    type: str
    default: Optional[str] = None


@dataclass
class Method:
    """Class method definition"""
    name: str
    returns: str = "void"
    const: bool = False
    parameters: List[Parameter] = field(default_factory=list)
    description: str = ""
    cpp_name: Optional[str] = None  # If different from name


@dataclass
class Field:
    """Class field definition"""
    name: str
    type: str
    access: str = "rw"  # rw, ro, wo
    description: str = ""
    cpp_name: Optional[str] = None


@dataclass
class Class:
    """Class definition"""
    name: str
    cpp_class: Optional[str] = None
    description: str = ""
    fields: List[Field] = field(default_factory=list)
    methods: List[Method] = field(default_factory=list)
    base_classes: List[str] = field(default_factory=list)


@dataclass
class Function:
    """Free function definition"""
    name: str
    cpp_name: Optional[str] = None
    returns: str = "void"
    parameters: List[Parameter] = field(default_factory=list)
    description: str = ""


@dataclass
class Converter:
    """Type converter definition"""
    type: str
    kind: str  # vector, optional, map, array, custom


@dataclass
class ModuleConfig:
    """Module configuration"""
    name: str
    version: str = "1.0.0"
    namespace: Optional[str] = None
    description: str = ""


@dataclass
class Utilities:
    """Utility functions to include"""
    version_info: bool = True
    list_classes: bool = True
    type_inspection: bool = True


@dataclass
class InterfaceDescription:
    """Complete interface description"""
    module: ModuleConfig
    includes: List[str] = field(default_factory=list)
    classes: List[Class] = field(default_factory=list)
    functions: List[Function] = field(default_factory=list)
    converters: List[Converter] = field(default_factory=list)
    utilities: Utilities = field(default_factory=Utilities)


class IDLParser:
    """Parser for Rosetta ILD files"""
    
    @staticmethod
    def parse_file(filepath: str) -> InterfaceDescription:
        """Parse an ILD file"""
        path = Path(filepath)
        
        with open(path, 'r') as f:
            if path.suffix in ['.yaml', '.yml']:
                if not HAS_YAML:
                    raise ImportError(
                        "PyYAML is required to parse YAML files.\n"
                        "Install with: pip install pyyaml --break-system-packages\n"
                        "Or use JSON format instead (.json extension)"
                    )
                data = yaml.safe_load(f)
            elif path.suffix == '.json':
                data = json.load(f)
            else:
                raise ValueError(f"Unsupported file format: {path.suffix}. Use .yaml, .yml, or .json")
        
        return IDLParser.parse_dict(data)
    
    @staticmethod
    def parse_dict(data: Dict[str, Any]) -> InterfaceDescription:
        """Parse dictionary data into InterfaceDescription"""
        
        # Parse module config
        module_data = data.get('module', {})
        module = ModuleConfig(
            name=module_data.get('name', 'module'),
            version=module_data.get('version', '1.0.0'),
            namespace=module_data.get('namespace'),
            description=module_data.get('description', '')
        )
        
        # Parse converters
        converters = []
        for conv_data in data.get('converters', []):
            converters.append(Converter(
                type=conv_data['type'],
                kind=conv_data['kind']
            ))
        
        # Parse classes
        classes = []
        for class_data in data.get('classes', []):
            fields = []
            for field_data in class_data.get('fields', []):
                fields.append(Field(
                    name=field_data['name'],
                    type=field_data['type'],
                    access=field_data.get('access', 'rw'),
                    description=field_data.get('description', ''),
                    cpp_name=field_data.get('cpp_name')
                ))
            
            methods = []
            for method_data in class_data.get('methods', []):
                params = []
                for param_data in method_data.get('parameters', []):
                    params.append(Parameter(
                        name=param_data['name'],
                        type=param_data['type'],
                        default=param_data.get('default')
                    ))
                
                methods.append(Method(
                    name=method_data['name'],
                    returns=method_data.get('returns', 'void'),
                    const=method_data.get('const', False),
                    parameters=params,
                    description=method_data.get('description', ''),
                    cpp_name=method_data.get('cpp_name')
                ))
            
            classes.append(Class(
                name=class_data['name'],
                cpp_class=class_data.get('cpp_class'),
                description=class_data.get('description', ''),
                fields=fields,
                methods=methods,
                base_classes=class_data.get('base_classes', [])
            ))
        
        # Parse functions
        functions = []
        for func_data in data.get('functions', []):
            params = []
            for param_data in func_data.get('parameters', []):
                params.append(Parameter(
                    name=param_data['name'],
                    type=param_data['type'],
                    default=param_data.get('default')
                ))
            
            functions.append(Function(
                name=func_data['name'],
                cpp_name=func_data.get('cpp_name'),
                returns=func_data.get('returns', 'void'),
                parameters=params,
                description=func_data.get('description', '')
            ))
        
        # Parse utilities
        util_data = data.get('utilities', {})
        utilities = Utilities(
            version_info=util_data.get('version_info', True),
            list_classes=util_data.get('list_classes', True),
            type_inspection=util_data.get('type_inspection', True)
        )
        
        return InterfaceDescription(
            module=module,
            includes=data.get('includes', []),
            classes=classes,
            functions=functions,
            converters=converters,
            utilities=utilities
        )


if __name__ == '__main__':
    # Test parsing
    parser = IDLParser()
    interface = parser.parse_file('rosetta.ild.yaml')
    
    print(f"Module: {interface.module.name} v{interface.module.version}")
    print(f"Classes: {len(interface.classes)}")
    print(f"Functions: {len(interface.functions)}")
    print(f"Converters: {len(interface.converters)}")