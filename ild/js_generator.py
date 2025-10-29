#!/usr/bin/env python3
"""
Rosetta JavaScript Binding Generator
Generates N-API binding code from ILD files
"""

from pathlib import Path
from typing import List
from ild_parser import ILDDocument, ClassDef, Method, Field, Function


class JSBindingGenerator:
    """Generates N-API binding code"""
    
    def __init__(self, doc: ILDDocument):
        self.doc = doc
    
    def generate_binding_cpp(self) -> str:
        """Generate the main binding.cxx file"""
        
        code = f"""// ============================================================================
// Auto-generated binding code from {self.doc.module.name}.ild
// DO NOT EDIT - Changes will be overwritten
// ============================================================================

#include <iostream>
#include <rosetta/generators/js/js_generator.h>
#include <rosetta/generators/js/type_converters.h>
#include <rosetta/rosetta.h>

using namespace rosetta;
using namespace rosetta::generators::js;

// Include headers
"""
        
        # Add includes
        for include in self.doc.includes:
            code += f'#include "{include}"\n'
        
        code += """
// ============================================================================
// ROSETTA REGISTRATION
// ============================================================================

void register_classes() {
"""
        
        # Generate class registrations
        for cls in self.doc.classes:
            code += self._generate_class_registration(cls)
        
        code += """}

// ============================================================================
// N-API BINDING
// ============================================================================

BEGIN_JS_MODULE(gen) {
    // Register classes
    register_classes();

    // Register type converters
"""
        
        # Generate converter registrations
        for converter in sorted(self.doc.converters):
            if converter.startswith('vector<'):
                element_type = converter[7:-1]  # Extract T from vector<T>
                code += f"    register_vector_converter<{element_type}>(gen);\n"
            elif converter.startswith('optional<'):
                element_type = converter[9:-1]  # Extract T from optional<T>
                code += f"    register_optional_converter<{element_type}>(gen);\n"
            elif converter.startswith('map<'):
                # Parse map<K,V>
                inner = converter[4:-1]
                parts = inner.split(',')
                if len(parts) == 2:
                    key_type = parts[0].strip()
                    val_type = parts[1].strip()
                    code += f"    register_map_converter<{key_type}, {val_type}>(gen);\n"
        
        code += "\n    // Bind classes\n"
        code += "    gen.bind_classes<"
        code += ", ".join(cls.name for cls in self.doc.classes)
        code += ">();\n\n"
        
        # Generate free functions
        for func in self.doc.functions:
            code += self._generate_function_binding(func)
        
        # Add utilities
        if 'inspect_type' in self.doc.utilities:
            code += self._generate_inspect_type()
        
        code += """
    gen.add_utilities();
}
END_JS_MODULE();
"""
        
        return code
    
    def _generate_class_registration(self, cls: ClassDef) -> str:
        """Generate Rosetta registration for a class"""
        code = f"\n    ROSETTA_REGISTER_CLASS({cls.name})\n"
        
        # Register fields
        for field in cls.fields:
            code += f"        .field(\"{field.name}\", &{cls.name}::{field.name})\n"
        
        # Register methods
        for method in cls.methods:
            code += f"        .method(\"{method.name}\", &{cls.name}::{method.name})\n"
        
        code += ";\n"
        return code
    
    def _generate_function_binding(self, func: Function) -> str:
        """Generate binding for a free function"""
        # For now, generate a placeholder
        # Full implementation would need proper parameter handling
        return f"    // TODO: Bind function {func.name}\n"
    
    def _generate_inspect_type(self) -> str:
        """Generate inspectType utility function"""
        return """
    // Add utility to inspect types
    auto inspect_type = [](const Napi::CallbackInfo &info) -> Napi::Value {
        Napi::Env env = info.Env();

        if (info.Length() < 1 || !info[0].IsString()) {
            Napi::TypeError::New(env, "Expected type name as string").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        std::string     type_name = info[0].As<Napi::String>().Utf8Value();
        const TypeInfo *type_info = TypeRegistry::instance().get_by_name(type_name);

        if (!type_info) {
            return env.Null();
        }

        Napi::Object result = Napi::Object::New(env);
        result.Set("name", Napi::String::New(env, type_info->name));
        result.Set("fullName", Napi::String::New(env, type_info->full_name()));
        result.Set("size", Napi::Number::New(env, type_info->size));
        result.Set("alignment", Napi::Number::New(env, type_info->alignment));
        result.Set("isTemplate", Napi::Boolean::New(env, type_info->is_template));
        result.Set("isConst", Napi::Boolean::New(env, type_info->is_const));
        result.Set("isPointer", Napi::Boolean::New(env, type_info->is_pointer));
        result.Set("isNumeric", Napi::Boolean::New(env, type_info->is_numeric()));

        return result;
    };

    gen.exports.Set("inspectType", Napi::Function::New(gen.env, inspect_type, "inspectType"));
"""
    
    def generate_binding_gyp(self, library_name: str = None) -> str:
        """Generate binding.gyp file"""
        if library_name is None:
            library_name = self.doc.module.targets.get('javascript', {}).get('output_name', 'module.node')
        
        # Remove .node extension if present
        if library_name.endswith('.node'):
            library_name = library_name[:-5]
        
        gyp = f"""{{
  "targets": [
    {{
      "target_name": "{library_name}",
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
          "-std=c++20"
        ]
      }},
      "msvs_settings": {{
        "VCCLCompilerTool": {{
          "ExceptionHandling": 1,
          "AdditionalOptions": [ "/std:c++20" ]
        }}
      }},
      "conditions": [
        ["OS=='linux'", {{
          "cflags_cc": [ "-std=c++20", "-fexceptions" ]
        }}]
      ]
    }}
  ]
}}
"""
        return gyp
    
    def generate_package_json(self) -> str:
        """Generate package.json file"""
        module_name = self.doc.module.name
        version = self.doc.module.version
        description = self.doc.module.description or f"{module_name} native module"
        
        package = f"""{{
  "name": "{module_name}",
  "version": "{version}",
  "description": "{description}",
  "main": "index.js",
  "scripts": {{
    "install": "node-gyp rebuild",
    "test": "node test.js"
  }},
  "dependencies": {{
    "node-addon-api": "^8.0.0"
  }},
  "devDependencies": {{
    "node-gyp": "^10.0.0"
  }},
  "gypfile": true,
  "keywords": [
    "native",
    "addon",
    "rosetta",
    "{module_name}"
  ],
  "license": "MIT"
}}
"""
        return package
    
    def generate_index_js(self) -> str:
        """Generate index.js wrapper"""
        output_name = self.doc.module.targets.get('javascript', {}).get('output_name', 'module.node')
        
        index = f"""// Auto-generated index.js
const addon = require('./build/Release/{output_name}');

module.exports = addon;
"""
        return index
    
    def generate_test_js(self) -> str:
        """Generate basic test file"""
        module_name = self.doc.module.name
        
        test = f"""// Auto-generated test file
const addon = require('./index.js');

console.log('{'='*70}');
console.log('{module_name} Test Suite');
console.log('{'='*70}');

"""
        
        # Generate tests for each class
        for i, cls in enumerate(self.doc.classes, 1):
            test += f"\n// Test {i}: {cls.name}\n"
            test += f"console.log('\\n[Test {i}] {cls.name}');\n"
            test += f"const obj{i} = new addon.{cls.name}();\n"
            
            # Test field access
            for field in cls.fields[:3]:  # First 3 fields
                test += f"console.log('  {field.name}:', obj{i}.{field.name});\n"
            
            # Test method calls
            for method in cls.methods[:2]:  # First 2 methods
                if not method.parameters:
                    test += f"console.log('  {method.name}():', obj{i}.{method.name}());\n"
        
        test += """
// List all classes
console.log('\\n[Classes]');
console.log('Available classes:', addon.listClasses());

console.log('\\nTest suite complete!');
"""
        
        return test
    
    def generate_all(self, output_dir: str = "."):
        """Generate all files"""
        output_path = Path(output_dir)
        output_path.mkdir(parents=True, exist_ok=True)
        
        files = {
            'binding.cxx': self.generate_binding_cpp(),
            'binding.gyp': self.generate_binding_gyp(),
            'package.json': self.generate_package_json(),
            'index.js': self.generate_index_js(),
            'test.js': self.generate_test_js()
        }
        
        for filename, content in files.items():
            filepath = output_path / filename
            with open(filepath, 'w') as f:
                f.write(content)
            print(f"Generated: {filepath}")


def main():
    """Test the generator"""
    from ild_parser import ILDParser
    
    parser = ILDParser()
    doc = parser.parse_file('/home/claude/geometry.ild')
    
    generator = JSBindingGenerator(doc)
    generator.generate_all('/home/claude/generated')


if __name__ == '__main__':
    main()