#!/usr/bin/env python3
"""
Rosetta JavaScript Binding Generator
Generates N-API binding code from ILD files
"""

from idl_parser import InterfaceDescription, Class, Method, Field, Converter
from typing import List, Set
from pathlib import Path


class JSBindingGenerator:
    """Generates JavaScript/N-API binding code"""
    
    def __init__(self, interface: InterfaceDescription):
        self.interface = interface
        self.includes: Set[str] = set()
    
    def _format_include(self, include: str) -> str:
        """
        Format include statement intelligently:
        - If it contains '/' or '.h' -> user include with quotes
        - Otherwise -> system include with angle brackets
        
        Examples:
            'vector' -> '#include <vector>'
            'string' -> '#include <string>'
            'mylib/Vector3D.h' -> '#include "mylib/Vector3D.h"'
            'Vector3D.h' -> '#include "Vector3D.h"'
        """
        include = include.strip()
        
        # Check if it's a user include (has path separator or .h extension)
        if '/' in include or include.endswith('.h') or include.endswith('.hpp'):
            return f'#include "{include}"'
        else:
            return f'#include <{include}>'
    
    def generate_binding_cpp(self) -> str:
        """Generate the binding.cxx file"""
        
        lines = []
        
        # Header
        lines.append("// " + "=" * 76)
        lines.append(f"// {self.interface.module.name} - JavaScript Bindings")
        lines.append("// Auto-generated from ILD file - DO NOT EDIT MANUALLY")
        lines.append("// " + "=" * 76)
        lines.append("")
        
        # Includes
        lines.append("#include <iostream>")
        lines.append("#include <rosetta/generators/js/js_generator.h>")
        lines.append("#include <rosetta/generators/js/type_converters.h>")
        lines.append("#include <rosetta/rosetta.h>")
        lines.append("")
        
        # Add user-specified includes
        if self.interface.includes:
            lines.append("// User-specified includes")
            for include in self.interface.includes:
                lines.append(self._format_include(include))
            lines.append("")
        
        # Add any additional custom includes
        for include in sorted(self.includes):
            lines.append(self._format_include(include))
        if self.includes:
            lines.append("")
        
        lines.append("using namespace rosetta;")
        lines.append("using namespace rosetta::generators::js;")
        lines.append("")
        
        # Generate class registration function
        lines.append("// " + "=" * 76)
        lines.append("// CLASS REGISTRATION")
        lines.append("// " + "=" * 76)
        lines.append("")
        lines.append("void register_classes() {")
        
        for cls in self.interface.classes:
            cpp_class = cls.cpp_class or cls.name
            lines.append(f"    ROSETTA_REGISTER_CLASS({cpp_class})")
            
            # Register fields
            for field in cls.fields:
                field_name = field.cpp_name or field.name
                lines.append(f'        .field("{field.name}", &{cpp_class}::{field_name})')
            
            # Register methods
            for method in cls.methods:
                method_name = method.cpp_name or method.name
                lines.append(f'        .method("{method.name}", &{cpp_class}::{method_name})')
            
            lines.append("        ;")
            lines.append("")
        
        lines.append("}")
        lines.append("")
        
        # Generate converter registration
        lines.append("// " + "=" * 76)
        lines.append("// TYPE CONVERTERS")
        lines.append("// " + "=" * 76)
        lines.append("")
        lines.append("void register_converters(JsGenerator &gen) {")
        
        for converter in self.interface.converters:
            if converter.kind == "vector":
                # Extract element type from std::vector<T>
                elem_type = self._extract_template_arg(converter.type)
                lines.append(f"    register_vector_converter<{elem_type}>(gen);")
            elif converter.kind == "optional":
                elem_type = self._extract_template_arg(converter.type)
                lines.append(f"    register_optional_converter<{elem_type}>(gen);")
            elif converter.kind == "array":
                # For std::array<T, N>
                lines.append(f"    // TODO: register array converter for {converter.type}")
            elif converter.kind == "map":
                lines.append(f"    // TODO: register map converter for {converter.type}")
        
        lines.append("}")
        lines.append("")
        
        # Generate N-API module initialization
        lines.append("// " + "=" * 76)
        lines.append("// N-API MODULE INITIALIZATION")
        lines.append("// " + "=" * 76)
        lines.append("")
        lines.append("BEGIN_JS_MODULE(gen) {")
        lines.append("    // Register classes")
        lines.append("    register_classes();")
        lines.append("")
        lines.append("    // Register type converters")
        lines.append("    register_converters(gen);")
        lines.append("")
        lines.append("    // Bind classes")
        lines.append("    gen.bind_classes<")
        
        class_list = [cls.cpp_class or cls.name for cls in self.interface.classes]
        for i, cls_name in enumerate(class_list):
            if i < len(class_list) - 1:
                lines.append(f"        {cls_name},")
            else:
                lines.append(f"        {cls_name}")
        
        lines.append("    >();")
        lines.append("")
        
        # Add custom converters if needed
        lines.append("    // Add custom converters here if needed")
        lines.append("")
        
        # Add utilities
        if self.interface.utilities.type_inspection:
            lines.append("    // Add type inspection utility")
            lines.append("    auto inspect_type = [](const Napi::CallbackInfo &info) -> Napi::Value {")
            lines.append("        Napi::Env env = info.Env();")
            lines.append("        if (info.Length() < 1 || !info[0].IsString()) {")
            lines.append('            Napi::TypeError::New(env, "Expected type name as string").ThrowAsJavaScriptException();')
            lines.append("            return env.Undefined();")
            lines.append("        }")
            lines.append("        std::string type_name = info[0].As<Napi::String>().Utf8Value();")
            lines.append("        const TypeInfo *type_info = TypeRegistry::instance().get_by_name(type_name);")
            lines.append("        if (!type_info) {")
            lines.append("            return env.Null();")
            lines.append("        }")
            lines.append("        Napi::Object result = Napi::Object::New(env);")
            lines.append('        result.Set("name", Napi::String::New(env, type_info->name));')
            lines.append('        result.Set("fullName", Napi::String::New(env, type_info->full_name()));')
            lines.append('        result.Set("size", Napi::Number::New(env, type_info->size));')
            lines.append('        result.Set("alignment", Napi::Number::New(env, type_info->alignment));')
            lines.append('        result.Set("isTemplate", Napi::Boolean::New(env, type_info->is_template));')
            lines.append('        result.Set("isConst", Napi::Boolean::New(env, type_info->is_const));')
            lines.append('        result.Set("isPointer", Napi::Boolean::New(env, type_info->is_pointer));')
            lines.append('        result.Set("isNumeric", Napi::Boolean::New(env, type_info->is_numeric()));')
            lines.append("        return result;")
            lines.append("    };")
            lines.append('    gen.exports.Set("inspectType", Napi::Function::New(gen.env, inspect_type, "inspectType"));')
            lines.append("")
        
        if self.interface.utilities.list_classes or self.interface.utilities.version_info:
            lines.append("    // Add utility functions")
            lines.append("    gen.add_utilities();")
        
        lines.append("}")
        lines.append("END_JS_MODULE()")
        
        return "\n".join(lines)
    
    def generate_binding_gyp(self) -> str:
        """Generate the binding.gyp file"""
        
        module_name = self.interface.module.name
        
        gyp = f"""{{
  "targets": [
    {{
      "target_name": "{module_name}",
      "sources": [
        "binding.cxx"
      ],
      "include_dirs": [
        "<!@(node -p \\"require('node-addon-api').include\\")",
        "."
      ],
      "dependencies": [
        "<!(node -p \\"require('node-addon-api').gyp\\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "xcode_settings": {{
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.15",
        "OTHER_CFLAGS": [
          "-std=c++17"
        ]
      }},
      "msvs_settings": {{
        "VCCLCompilerTool": {{
          "ExceptionHandling": 1,
          "AdditionalOptions": [ "/std:c++17" ]
        }}
      }},
      "conditions": [
        ["OS=='linux'", {{
          "cflags_cc": [
            "-std=c++17",
            "-fexceptions"
          ]
        }}]
      ]
    }}
  ]
}}
"""
        return gyp
    
    def generate_package_json(self) -> str:
        """Generate package.json file"""
        
        module = self.interface.module
        
        package = f"""{{
  "name": "{module.name}",
  "version": "{module.version}",
  "description": "{module.description or f'{module.name} - N-API bindings'}",
  "main": "index.js",
  "scripts": {{
    "install": "node-gyp rebuild",
    "test": "node test.js"
  }},
  "keywords": [
    "napi",
    "rosetta",
    "bindings"
  ],
  "author": "",
  "license": "MIT",
  "gypfile": true,
  "dependencies": {{
    "node-addon-api": "^7.0.0"
  }},
  "devDependencies": {{
    "node-gyp": "^10.0.0"
  }}
}}
"""
        return package
    
    def generate_test_js(self) -> str:
        """Generate a basic test file"""
        
        module_name = self.interface.module.name
        
        lines = []
        lines.append(f"const addon = require('./build/Release/{module_name}.node');")
        lines.append("")
        lines.append("console.log('='.repeat(70));")
        lines.append(f"console.log('{module_name.upper()} - JavaScript Binding Test');")
        lines.append("console.log('='.repeat(70));")
        lines.append("")
        
        for i, cls in enumerate(self.interface.classes, 1):
            lines.append(f"// Test {i}: {cls.name}")
            lines.append("console.log('\\n[Test " + str(i) + f"] {cls.name}');")
            lines.append("console.log('-'.repeat(70));")
            lines.append(f"const obj{i} = new addon.{cls.name}();")
            
            # Test fields
            if cls.fields:
                lines.append(f"console.log('Fields:');")
                for field in cls.fields[:3]:  # Limit to first 3 fields
                    lines.append(f"console.log('  {field.name}:', obj{i}.{field.name});")
            
            # Test methods
            if cls.methods:
                lines.append(f"console.log('Methods:');")
                for method in cls.methods[:2]:  # Limit to first 2 methods
                    if not method.parameters:
                        lines.append(f"console.log('  {method.name}():', obj{i}.{method.name}());")
            
            lines.append("")
        
        lines.append("// List all classes")
        lines.append("if (addon.listClasses) {")
        lines.append("    console.log('\\nAvailable classes:', addon.listClasses());")
        lines.append("}")
        lines.append("")
        lines.append("console.log('\\nAll tests completed!');")
        
        return "\n".join(lines)
    
    def _extract_template_arg(self, type_str: str) -> str:
        """Extract template argument from type string like 'std::vector<int>'"""
        start = type_str.find('<')
        end = type_str.rfind('>')
        if start != -1 and end != -1:
            return type_str[start + 1:end].strip()
        return type_str
    
    def generate_all(self, output_dir: str = "."):
        """Generate all files"""
        output_path = Path(output_dir)
        output_path.mkdir(parents=True, exist_ok=True)
        
        # Generate binding.cxx
        binding_cpp = self.generate_binding_cpp()
        (output_path / "binding.cxx").write_text(binding_cpp)
        print(f"✓ Generated {output_path}/binding.cxx")
        
        # Generate binding.gyp
        binding_gyp = self.generate_binding_gyp()
        (output_path / "binding.gyp").write_text(binding_gyp)
        print(f"✓ Generated {output_path}/binding.gyp")
        
        # Generate package.json
        package_json = self.generate_package_json()
        (output_path / "package.json").write_text(package_json)
        print(f"✓ Generated {output_path}/package.json")
        
        # Generate test.js
        test_js = self.generate_test_js()
        (output_path / "test.js").write_text(test_js)
        print(f"✓ Generated {output_path}/test.js")


if __name__ == '__main__':
    import sys
    from idl.idl_parser import IDLParser
    
    if len(sys.argv) < 2:
        print("Usage: python js_generator.py <ild_file> [output_dir]")
        sys.exit(1)
    
    ild_file = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else "."
    
    # Parse ILD file
    parser = IDLParser()
    interface = parser.parse_file(ild_file)
    
    # Generate bindings
    generator = JSBindingGenerator(interface)
    generator.generate_all(output_dir)
    
    print("\n✅ JavaScript bindings generated successfully!")
    print(f"\nNext steps:")
    print(f"  cd {output_dir}")
    print(f"  npm install")
    print(f"  node test.js")