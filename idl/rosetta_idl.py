#!/usr/bin/env python3
"""
Rosetta Binding Generator - Command Line Tool
Generates language bindings from ILD files
"""

import argparse
import sys
from pathlib import Path
from idl_parser import IDLParser
from js_generator import JSBindingGenerator


def main():
    parser = argparse.ArgumentParser(
        description='Generate language bindings from Rosetta ILD files',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Generate JavaScript bindings
  rosetta-gen --input interface.yaml --lang js --output ./bindings
  
  # Generate Python bindings (future)
  rosetta-gen --input interface.yaml --lang python --output ./bindings
  
  # Generate for multiple languages
  rosetta-gen --input interface.yaml --lang js,python --output ./bindings
        """
    )
    
    parser.add_argument(
        '-i', '--input',
        required=True,
        help='Input ILD file (YAML or JSON)'
    )
    
    parser.add_argument(
        '-l', '--lang',
        required=True,
        choices=['js', 'javascript', 'python', 'py', 'all'],
        help='Target language(s) for binding generation'
    )
    
    parser.add_argument(
        '-o', '--output',
        default='./bindings',
        help='Output directory (default: ./bindings)'
    )
    
    parser.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='Verbose output'
    )
    
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Parse ILD but don\'t generate files'
    )
    
    args = parser.parse_args()
    
    # Validate input file
    input_path = Path(args.input)
    if not input_path.exists():
        print(f"❌ Error: Input file not found: {args.input}", file=sys.stderr)
        sys.exit(1)
    
    # Parse ILD file
    if args.verbose:
        print(f"📖 Parsing ILD file: {args.input}")
    
    try:
        ild_parser = IDLParser()
        interface = ild_parser.parse_file(args.input)
    except Exception as e:
        print(f"❌ Error parsing ILD file: {e}", file=sys.stderr)
        if args.verbose:
            import traceback
            traceback.print_exc()
        sys.exit(1)
    
    # Display parsed information
    if args.verbose:
        print(f"\n✓ Successfully parsed interface description:")
        print(f"  Module: {interface.module.name} v{interface.module.version}")
        print(f"  Classes: {len(interface.classes)}")
        for cls in interface.classes:
            print(f"    - {cls.name} ({len(cls.fields)} fields, {len(cls.methods)} methods)")
        print(f"  Functions: {len(interface.functions)}")
        print(f"  Converters: {len(interface.converters)}")
        print()
    
    if args.dry_run:
        print("✓ Dry run complete - no files generated")
        sys.exit(0)
    
    # Normalize language selection
    languages = []
    lang = args.lang.lower()
    if lang in ['js', 'javascript']:
        languages = ['javascript']
    elif lang in ['py', 'python']:
        languages = ['python']
    elif lang == 'all':
        languages = ['javascript', 'python']
    
    # Generate bindings
    output_base = Path(args.output)
    
    for language in languages:
        if language == 'javascript':
            output_dir = output_base / 'javascript'
            if args.verbose:
                print(f"🔨 Generating JavaScript bindings to: {output_dir}")
            
            try:
                generator = JSBindingGenerator(interface)
                generator.generate_all(str(output_dir))
                print(f"\n✅ JavaScript bindings generated successfully!")
                print(f"\nNext steps:")
                print(f"  cd {output_dir}")
                print(f"  npm install")
                print(f"  node test.js")
            except Exception as e:
                print(f"❌ Error generating JavaScript bindings: {e}", file=sys.stderr)
                if args.verbose:
                    import traceback
                    traceback.print_exc()
                sys.exit(1)
        
        elif language == 'python':
            output_dir = output_base / 'python'
            print(f"⚠️  Python bindings not yet implemented")
            # TODO: Implement Python binding generator
            # try:
            #     generator = PythonBindingGenerator(interface)
            #     generator.generate_all(str(output_dir))
            # except Exception as e:
            #     print(f"❌ Error generating Python bindings: {e}", file=sys.stderr)
            #     sys.exit(1)


if __name__ == '__main__':
    main()