#!/usr/bin/env python3
"""
Rosetta ILD Tool
Command-line interface for generating bindings from ILD files
"""

import argparse
import sys
from pathlib import Path
from ild_parser import ILDParser
from js_generator import JSBindingGenerator


class RosettaILD:
    """Main CLI application"""
    
    def __init__(self):
        self.parser = argparse.ArgumentParser(
            description='Rosetta ILD - Interface Language Description tool for binding generation',
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog="""
Examples:
  # Generate JavaScript bindings
  rosetta-ild generate geometry.ild --target javascript --output ./bindings

  # Generate Python bindings (future)
  rosetta-ild generate geometry.ild --target python --output ./bindings

  # Validate an ILD file
  rosetta-ild validate geometry.ild

  # Show info about an ILD file
  rosetta-ild info geometry.ild
            """
        )
        
        subparsers = self.parser.add_subparsers(dest='command', help='Commands')
        
        # Generate command
        gen_parser = subparsers.add_parser('generate', help='Generate bindings from ILD file')
        gen_parser.add_argument('ild_file', help='Path to ILD file')
        gen_parser.add_argument('--target', '-t', 
                               choices=['javascript', 'python', 'csharp'],
                               default='javascript',
                               help='Target language (default: javascript)')
        gen_parser.add_argument('--output', '-o', 
                               default='.',
                               help='Output directory (default: current directory)')
        gen_parser.add_argument('--library-name', '-l',
                               help='Override library name')
        
        # Validate command
        val_parser = subparsers.add_parser('validate', help='Validate an ILD file')
        val_parser.add_argument('ild_file', help='Path to ILD file')
        
        # Info command
        info_parser = subparsers.add_parser('info', help='Show information about an ILD file')
        info_parser.add_argument('ild_file', help='Path to ILD file')
        
        # Init command
        init_parser = subparsers.add_parser('init', help='Initialize a new ILD file')
        init_parser.add_argument('module_name', help='Module name')
        init_parser.add_argument('--output', '-o',
                                default='module.ild',
                                help='Output file (default: module.ild)')
    
    def run(self, args=None):
        """Run the CLI"""
        args = self.parser.parse_args(args)
        
        if not args.command:
            self.parser.print_help()
            return 1
        
        try:
            if args.command == 'generate':
                return self.cmd_generate(args)
            elif args.command == 'validate':
                return self.cmd_validate(args)
            elif args.command == 'info':
                return self.cmd_info(args)
            elif args.command == 'init':
                return self.cmd_init(args)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            return 1
        
        return 0
    
    def cmd_generate(self, args):
        """Generate bindings"""
        print(f"Parsing ILD file: {args.ild_file}")
        
        parser = ILDParser()
        doc = parser.parse_file(args.ild_file)
        
        print(f"Module: {doc.module.name} v{doc.module.version}")
        print(f"Target: {args.target}")
        print(f"Output: {args.output}")
        
        if args.target == 'javascript':
            generator = JSBindingGenerator(doc)
            generator.generate_all(args.output)
            print(f"\n✓ JavaScript bindings generated successfully!")
            print(f"\nNext steps:")
            print(f"  cd {args.output}")
            print(f"  npm install")
            print(f"  npm test")
        elif args.target == 'python':
            print("Python binding generation not yet implemented")
            return 1
        elif args.target == 'csharp':
            print("C# binding generation not yet implemented")
            return 1
        
        return 0
    
    def cmd_validate(self, args):
        """Validate ILD file"""
        print(f"Validating: {args.ild_file}")
        
        try:
            parser = ILDParser()
            doc = parser.parse_file(args.ild_file)
            
            print("✓ Syntax is valid")
            print(f"\nModule: {doc.module.name}")
            print(f"Classes: {len(doc.classes)}")
            print(f"Functions: {len(doc.functions)}")
            
            # Basic semantic checks
            warnings = []
            
            # Check for missing includes
            if not doc.includes:
                warnings.append("No includes specified - ensure classes are defined")
            
            # Check for classes without methods or fields
            for cls in doc.classes:
                if not cls.fields and not cls.methods:
                    warnings.append(f"Class {cls.name} has no fields or methods")
            
            if warnings:
                print("\nWarnings:")
                for warning in warnings:
                    print(f"  ⚠ {warning}")
            else:
                print("\n✓ No warnings")
            
            return 0
            
        except Exception as e:
            print(f"✗ Validation failed: {e}")
            return 1
    
    def cmd_info(self, args):
        """Show ILD file information"""
        parser = ILDParser()
        doc = parser.parse_file(args.ild_file)
        
        print("="*70)
        print(f"Module: {doc.module.name}")
        print("="*70)
        print(f"Version: {doc.module.version}")
        print(f"Description: {doc.module.description}")
        
        if doc.module.targets:
            print("\nTargets:")
            for target, config in doc.module.targets.items():
                print(f"  {target}:")
                for key, value in config.items():
                    print(f"    {key}: {value}")
        
        print(f"\nIncludes ({len(doc.includes)}):")
        for include in doc.includes:
            print(f"  - {include}")
        
        print(f"\nConverters ({len(doc.converters)}):")
        for converter in sorted(doc.converters):
            print(f"  - {converter}")
        
        print(f"\nClasses ({len(doc.classes)}):")
        for cls in doc.classes:
            print(f"\n  {cls.name}")
            if cls.base_class:
                print(f"    Base: {cls.base_class}")
            print(f"    Constructors: {len(cls.constructors)}")
            print(f"    Fields: {len(cls.fields)}")
            if cls.fields:
                for field in cls.fields:
                    alias = f" (as {field.alias})" if field.alias else ""
                    print(f"      - {field.name}: {field.type}{alias}")
            print(f"    Methods: {len(cls.methods)}")
            if cls.methods:
                for method in cls.methods:
                    params = ", ".join(f"{p.type} {p.name}" for p in method.parameters)
                    alias = f" (as {method.alias})" if method.alias else ""
                    print(f"      - {method.name}({params}): {method.return_type}{alias}")
        
        if doc.functions:
            print(f"\nFunctions ({len(doc.functions)}):")
            for func in doc.functions:
                params = ", ".join(f"{p.type} {p.name}" for p in func.parameters)
                print(f"  - {func.name}({params}): {func.return_type}")
        
        if doc.utilities:
            print(f"\nUtilities ({len(doc.utilities)}):")
            for util in doc.utilities:
                print(f"  - {util}")
        
        return 0
    
    def cmd_init(self, args):
        """Initialize a new ILD file"""
        template = f"""# Rosetta Interface Language Description
# Generated template for {args.module_name}

module {args.module_name} {{
    version: "1.0.0"
    description: "{args.module_name} module"
    
    targets {{
        javascript {{
            output_name: "{args.module_name}.node"
        }}
    }}
}}

# Include your C++ headers
# include "MyClass.h"

# Register type converters
converters {{
    vector<int>
    vector<double>
    optional<double>
}}

# Define your classes
# class MyClass {{
#     constructor()
#     
#     field myField: int
#     method myMethod(): void
# }}

# Add utilities
utilities {{
    inspect_type
    list_classes
    get_version
}}
"""
        
        output_path = Path(args.output)
        if output_path.exists():
            print(f"Error: {output_path} already exists")
            return 1
        
        with open(output_path, 'w') as f:
            f.write(template)
        
        print(f"✓ Created: {output_path}")
        print(f"\nEdit the file to define your bindings, then run:")
        print(f"  rosetta-ild generate {output_path} --target javascript")
        
        return 0


def main():
    """Main entry point"""
    app = RosettaILD()
    sys.exit(app.run())


if __name__ == '__main__':
    main()