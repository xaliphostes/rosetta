#!/usr/bin/env python3
"""
Rosetta ILD Parser
Parses Interface Language Description files for binding generation
"""

import re
from dataclasses import dataclass, field
from typing import List, Dict, Optional, Set
from pathlib import Path


@dataclass
class Parameter:
    """Function/method parameter"""
    name: str
    type: str


@dataclass
class Constructor:
    """Class constructor definition"""
    parameters: List[Parameter] = field(default_factory=list)


@dataclass
class Field:
    """Class field definition"""
    name: str
    type: str
    alias: Optional[str] = None


@dataclass
class Method:
    """Class method definition"""
    name: str
    return_type: str
    parameters: List[Parameter] = field(default_factory=list)
    alias: Optional[str] = None
    is_const: bool = True


@dataclass
class Function:
    """Free function definition"""
    name: str
    return_type: str
    parameters: List[Parameter] = field(default_factory=list)
    alias: Optional[str] = None


@dataclass
class ClassDef:
    """Class definition"""
    name: str
    constructors: List[Constructor] = field(default_factory=list)
    fields: List[Field] = field(default_factory=list)
    methods: List[Method] = field(default_factory=list)
    base_class: Optional[str] = None


@dataclass
class ModuleConfig:
    """Module configuration"""
    name: str
    version: str = "1.0.0"
    description: str = ""
    targets: Dict[str, Dict[str, str]] = field(default_factory=dict)


@dataclass
class ILDDocument:
    """Complete ILD document"""
    module: ModuleConfig
    includes: List[str] = field(default_factory=list)
    converters: Set[str] = field(default_factory=set)
    classes: List[ClassDef] = field(default_factory=list)
    functions: List[Function] = field(default_factory=list)
    utilities: List[str] = field(default_factory=list)


class ILDParser:
    """Parser for Rosetta ILD files"""
    
    def __init__(self):
        self.current_line = 0
        self.lines = []
    
    def parse_file(self, filepath: str) -> ILDDocument:
        """Parse an ILD file"""
        with open(filepath, 'r') as f:
            content = f.read()
        return self.parse(content)
    
    def parse(self, content: str) -> ILDDocument:
        """Parse ILD content"""
        # Remove comments and empty lines
        self.lines = []
        for line in content.split('\n'):
            # Remove comments
            if '#' in line:
                line = line[:line.index('#')]
            line = line.strip()
            if line:
                self.lines.append(line)
        
        self.current_line = 0
        doc = ILDDocument(module=ModuleConfig(name="unnamed"))
        
        while self.current_line < len(self.lines):
            line = self.lines[self.current_line]
            
            if line.startswith('module '):
                doc.module = self._parse_module()
            elif line.startswith('include '):
                doc.includes.append(self._parse_include())
            elif line.startswith('converters {'):
                doc.converters = self._parse_converters()
            elif line.startswith('class '):
                doc.classes.append(self._parse_class())
            elif line.startswith('function '):
                doc.functions.append(self._parse_function())
            elif line.startswith('utilities {'):
                doc.utilities = self._parse_utilities()
            else:
                self.current_line += 1
        
        return doc
    
    def _parse_module(self) -> ModuleConfig:
        """Parse module block"""
        line = self.lines[self.current_line]
        match = re.match(r'module\s+(\w+)\s*{', line)
        if not match:
            raise ValueError(f"Invalid module syntax at line {self.current_line}")
        
        module = ModuleConfig(name=match.group(1))
        self.current_line += 1
        
        while self.current_line < len(self.lines):
            line = self.lines[self.current_line]
            
            if line == '}':
                self.current_line += 1
                break
            elif line.startswith('version:'):
                module.version = self._parse_value(line)
            elif line.startswith('description:'):
                module.description = self._parse_value(line)
            elif line.startswith('targets {'):
                module.targets = self._parse_targets()
            else:
                self.current_line += 1
        
        return module
    
    def _parse_targets(self) -> Dict[str, Dict[str, str]]:
        """Parse targets block"""
        targets = {}
        self.current_line += 1
        
        while self.current_line < len(self.lines):
            line = self.lines[self.current_line]
            
            if line == '}':
                self.current_line += 1
                break
            elif line.endswith('{'):
                target_name = line[:-1].strip()
                targets[target_name] = self._parse_target_config()
            else:
                self.current_line += 1
        
        return targets
    
    def _parse_target_config(self) -> Dict[str, str]:
        """Parse single target configuration"""
        config = {}
        self.current_line += 1
        
        while self.current_line < len(self.lines):
            line = self.lines[self.current_line]
            
            if line == '}':
                self.current_line += 1
                break
            elif ':' in line:
                key, value = line.split(':', 1)
                config[key.strip()] = value.strip().strip('"')
                self.current_line += 1
            else:
                self.current_line += 1
        
        return config
    
    def _parse_include(self) -> str:
        """Parse include statement"""
        line = self.lines[self.current_line]
        match = re.match(r'include\s+"([^"]+)"', line)
        if not match:
            raise ValueError(f"Invalid include syntax at line {self.current_line}")
        
        self.current_line += 1
        return match.group(1)
    
    def _parse_converters(self) -> Set[str]:
        """Parse converters block"""
        converters = set()
        self.current_line += 1
        
        while self.current_line < len(self.lines):
            line = self.lines[self.current_line]
            
            if line == '}':
                self.current_line += 1
                break
            else:
                converters.add(line)
                self.current_line += 1
        
        return converters
    
    def _parse_class(self) -> ClassDef:
        """Parse class block"""
        line = self.lines[self.current_line]
        match = re.match(r'class\s+(\w+)(?:\s*:\s*(\w+))?\s*{', line)
        if not match:
            raise ValueError(f"Invalid class syntax at line {self.current_line}")
        
        class_def = ClassDef(
            name=match.group(1),
            base_class=match.group(2)
        )
        self.current_line += 1
        
        while self.current_line < len(self.lines):
            line = self.lines[self.current_line]
            
            if line == '}':
                self.current_line += 1
                break
            elif line.startswith('constructor'):
                class_def.constructors.append(self._parse_constructor())
            elif line.startswith('field '):
                class_def.fields.append(self._parse_field())
            elif line.startswith('method '):
                class_def.methods.append(self._parse_method())
            else:
                self.current_line += 1
        
        # Add default constructor if none specified
        if not class_def.constructors:
            class_def.constructors.append(Constructor())
        
        return class_def
    
    def _parse_constructor(self) -> Constructor:
        """Parse constructor"""
        line = self.lines[self.current_line]
        params = self._parse_parameter_list(line[len('constructor'):])
        self.current_line += 1
        return Constructor(parameters=params)
    
    def _parse_field(self) -> Field:
        """Parse field definition"""
        line = self.lines[self.current_line]
        # field name: type [as alias]
        match = re.match(r'field\s+(\w+)\s*:\s*([^\s]+)(?:\s+as\s+(\w+))?', line)
        if not match:
            raise ValueError(f"Invalid field syntax at line {self.current_line}")
        
        self.current_line += 1
        return Field(
            name=match.group(1),
            type=match.group(2),
            alias=match.group(3)
        )
    
    def _parse_method(self) -> Method:
        """Parse method definition"""
        line = self.lines[self.current_line]
        # method name(params): return_type [as alias]
        match = re.match(r'method\s+(\w+)\s*\(([^)]*)\)\s*:\s*([^\s]+)(?:\s+as\s+(\w+))?', line)
        if not match:
            raise ValueError(f"Invalid method syntax at line {self.current_line}")
        
        name = match.group(1)
        params_str = match.group(2)
        return_type = match.group(3)
        alias = match.group(4)
        
        params = self._parse_parameter_list(params_str)
        
        self.current_line += 1
        return Method(
            name=name,
            return_type=return_type,
            parameters=params,
            alias=alias
        )
    
    def _parse_function(self) -> Function:
        """Parse function definition"""
        line = self.lines[self.current_line]
        # function name(params): return_type [as alias]
        match = re.match(r'function\s+(\w+)\s*\(([^)]*)\)\s*:\s*([^\s]+)(?:\s+as\s+(\w+))?', line)
        if not match:
            raise ValueError(f"Invalid function syntax at line {self.current_line}")
        
        name = match.group(1)
        params_str = match.group(2)
        return_type = match.group(3)
        alias = match.group(4)
        
        params = self._parse_parameter_list(params_str)
        
        self.current_line += 1
        return Function(
            name=name,
            return_type=return_type,
            parameters=params,
            alias=alias
        )
    
    def _parse_utilities(self) -> List[str]:
        """Parse utilities block"""
        utilities = []
        self.current_line += 1
        
        while self.current_line < len(self.lines):
            line = self.lines[self.current_line]
            
            if line == '}':
                self.current_line += 1
                break
            else:
                utilities.append(line)
                self.current_line += 1
        
        return utilities
    
    def _parse_parameter_list(self, params_str: str) -> List[Parameter]:
        """Parse parameter list"""
        params = []
        if not params_str.strip():
            return params
        
        for param in params_str.split(','):
            param = param.strip()
            if not param:
                continue
            
            # Handle "type name" or just "type"
            parts = param.rsplit(' ', 1)
            if len(parts) == 2:
                param_type, param_name = parts
            else:
                param_type = parts[0]
                param_name = f"arg{len(params)}"
            
            params.append(Parameter(name=param_name, type=param_type))
        
        return params
    
    def _parse_value(self, line: str) -> str:
        """Parse a key: value line"""
        value = line.split(':', 1)[1].strip()
        return value.strip('"')


def main():
    """Test the parser"""
    parser = ILDParser()
    
    # Test with the example file
    doc = parser.parse_file('/home/claude/geometry.ild')
    
    print(f"Module: {doc.module.name} v{doc.module.version}")
    print(f"Description: {doc.module.description}")
    print(f"\nIncludes: {doc.includes}")
    print(f"\nConverters: {doc.converters}")
    print(f"\nClasses:")
    for cls in doc.classes:
        print(f"  - {cls.name}")
        print(f"    Constructors: {len(cls.constructors)}")
        print(f"    Fields: {[f.name for f in cls.fields]}")
        print(f"    Methods: {[m.name for m in cls.methods]}")
    print(f"\nFunctions: {[f.name for f in doc.functions]}")
    print(f"\nUtilities: {doc.utilities}")


if __name__ == '__main__':
    main()