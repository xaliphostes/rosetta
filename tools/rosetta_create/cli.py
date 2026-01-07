#!/usr/bin/env python3
"""
Command-line interface for rosetta-create.

Usage:
    # Interactive mode (wizard)
    rosetta-create

    # Non-interactive mode with arguments
    rosetta-create --name myproject --lib-name mylib --lib-repo https://github.com/user/mylib.git

    # Show help
    rosetta-create --help
"""

import argparse
import sys
from pathlib import Path
from typing import List, Optional

from .generator import ProjectGenerator, ProjectConfig, LibrarySource


AVAILABLE_TARGETS = ["python", "wasm", "javascript", "rest"]


def print_banner():
    """Print the rosetta-create banner."""
    print()
    print("=" * 60)
    print("  Rosetta Create - Project Scaffolding Tool")
    print("=" * 60)
    print()


def prompt_string(prompt: str, default: str = "") -> str:
    """Prompt for a string input with optional default."""
    if default:
        result = input(f"{prompt} [{default}]: ").strip()
        return result if result else default
    else:
        while True:
            result = input(f"{prompt}: ").strip()
            if result:
                return result
            print("  This field is required. Please enter a value.")


def prompt_choice(prompt: str, choices: List[str], default: str = "") -> str:
    """Prompt for a single choice from a list."""
    print(f"{prompt}")
    for i, choice in enumerate(choices, 1):
        marker = " (default)" if choice == default else ""
        print(f"  {i}. {choice}{marker}")

    while True:
        result = input("Enter number or name: ").strip()
        if not result and default:
            return default

        # Check if it's a number
        try:
            idx = int(result) - 1
            if 0 <= idx < len(choices):
                return choices[idx]
        except ValueError:
            pass

        # Check if it's a name
        if result.lower() in [c.lower() for c in choices]:
            for c in choices:
                if c.lower() == result.lower():
                    return c

        print(f"  Invalid choice. Please enter 1-{len(choices)} or a valid name.")


def prompt_multi_choice(prompt: str, choices: List[str], defaults: List[str] = None) -> List[str]:
    """Prompt for multiple choices from a list."""
    defaults = defaults or []
    print(f"{prompt}")
    print("  (Enter comma-separated numbers or 'all' for all options)")
    for i, choice in enumerate(choices, 1):
        marker = " *" if choice in defaults else ""
        print(f"  {i}. {choice}{marker}")

    while True:
        result = input("Enter choices: ").strip()
        if not result and defaults:
            return defaults

        if result.lower() == "all":
            return choices.copy()

        selected = []
        try:
            for part in result.split(","):
                part = part.strip()
                if part.isdigit():
                    idx = int(part) - 1
                    if 0 <= idx < len(choices):
                        selected.append(choices[idx])
                elif part.lower() in [c.lower() for c in choices]:
                    for c in choices:
                        if c.lower() == part.lower():
                            selected.append(c)

            if selected:
                return list(dict.fromkeys(selected))  # Remove duplicates, preserve order
        except (ValueError, IndexError):
            pass

        print(f"  Invalid input. Enter comma-separated numbers (1-{len(choices)}) or 'all'.")


def prompt_yes_no(prompt: str, default: bool = True) -> bool:
    """Prompt for a yes/no answer."""
    default_str = "Y/n" if default else "y/N"
    while True:
        result = input(f"{prompt} [{default_str}]: ").strip().lower()
        if not result:
            return default
        if result in ["y", "yes"]:
            return True
        if result in ["n", "no"]:
            return False
        print("  Please enter 'y' or 'n'.")


def interactive_mode() -> ProjectConfig:
    """Run the interactive wizard to gather project configuration."""
    print_banner()
    print("This wizard will help you create a new Rosetta binding project.")
    print()

    # Project basics
    print("--- Project Information ---")
    name = prompt_string("Project name", "my-bindings")
    description = prompt_string("Description", f"Python bindings for {name}")
    author = prompt_string("Author", "")
    version = prompt_string("Version", "1.0.0")
    license_type = prompt_string("License", "MIT")
    print()

    # Library configuration
    print("--- Library to Bind ---")
    lib_name = prompt_string("Library name (the C++ library you want to bind)")

    source_type = prompt_choice(
        "How will you provide the library source?",
        ["fetch", "local"],
        "fetch"
    )

    if source_type == "fetch":
        lib_repo = prompt_string("Git repository URL")
        lib_tag = prompt_string("Git tag/branch", "main")
        lib_source = LibrarySource(
            source_type="fetch",
            git_repo=lib_repo,
            git_tag=lib_tag
        )
    else:
        lib_path = prompt_string("Local path to library")
        lib_source = LibrarySource(
            source_type="local",
            local_path=lib_path
        )

    lib_include_subdir = prompt_string("Include subdirectory (relative to lib root)", "include")
    lib_src_subdir = prompt_string("Source subdirectory (relative to lib root)", "src")
    print()

    # Binding targets
    print("--- Binding Targets ---")
    targets = prompt_multi_choice(
        "Which bindings do you want to generate?",
        AVAILABLE_TARGETS,
        ["python"]
    )
    print()

    # Namespace
    print("--- Namespace Configuration ---")
    has_namespace = prompt_yes_no("Does your library use a C++ namespace?", True)
    if has_namespace:
        namespace = prompt_string("C++ namespace for your library", lib_name)
    else:
        namespace = ""
    registration_namespace = prompt_string("Registration namespace", f"{name.replace('-', '_')}_rosetta")
    print()

    # Output
    print("--- Output ---")
    output_dir = prompt_string("Output directory", f"./{name}")
    print()

    # Confirmation
    print("=" * 60)
    print("Summary:")
    print(f"  Project:     {name} v{version}")
    print(f"  Library:     {lib_name} ({source_type})")
    print(f"  Targets:     {', '.join(targets)}")
    print(f"  Output:      {output_dir}")
    print("=" * 60)

    if not prompt_yes_no("Create project?", True):
        print("Aborted.")
        sys.exit(0)

    return ProjectConfig(
        name=name,
        description=description,
        author=author,
        version=version,
        license=license_type,
        lib_name=lib_name,
        lib_source=lib_source,
        lib_include_subdir=lib_include_subdir,
        lib_src_subdir=lib_src_subdir,
        namespace=namespace,
        registration_namespace=registration_namespace,
        targets=targets,
        output_dir=output_dir
    )


def parse_args() -> argparse.Namespace:
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        prog="rosetta-create",
        description="Create a new Rosetta binding project",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Interactive mode (wizard)
  rosetta-create

  # Create project with GitHub library
  rosetta-create --name pmp-bindings --lib-name pmp \\
    --lib-repo https://github.com/pmp-library/pmp-library.git \\
    --targets python,wasm

  # Create project with local library
  rosetta-create --name mylib-bindings --lib-name mylib \\
    --lib-path ../mylib --targets python
"""
    )

    # Project info
    parser.add_argument("--name", "-n", help="Project name")
    parser.add_argument("--description", "-d", help="Project description")
    parser.add_argument("--author", "-a", help="Author name")
    parser.add_argument("--version", "-v", default="1.0.0", help="Version (default: 1.0.0)")
    parser.add_argument("--license", "-l", default="MIT", help="License (default: MIT)")

    # Library source (mutually exclusive)
    lib_group = parser.add_mutually_exclusive_group()
    lib_group.add_argument("--lib-repo", help="Git repository URL for library")
    lib_group.add_argument("--lib-path", help="Local path to library")

    parser.add_argument("--lib-name", help="Name of the library to bind")
    parser.add_argument("--lib-tag", default="main", help="Git tag/branch (default: main)")
    parser.add_argument("--lib-include-dir", default="include", help="Include subdirectory (default: include)")
    parser.add_argument("--lib-src-dir", default="src", help="Source subdirectory (default: src)")

    # Namespace
    parser.add_argument("--namespace", default=None, help="C++ namespace (use 'none' for no namespace, default: lib-name)")
    parser.add_argument("--registration-namespace", help="Registration namespace")

    # Targets
    parser.add_argument(
        "--targets", "-t",
        default="python",
        help=f"Comma-separated binding targets: {','.join(AVAILABLE_TARGETS)} (default: python)"
    )

    # Output
    parser.add_argument("--output", "-o", help="Output directory (default: ./<name>)")

    # Flags
    parser.add_argument("--interactive", "-i", action="store_true", help="Force interactive mode")
    parser.add_argument("--force", "-f", action="store_true", help="Overwrite existing directory")

    return parser.parse_args()


def config_from_args(args: argparse.Namespace) -> Optional[ProjectConfig]:
    """Create ProjectConfig from command-line arguments."""
    # Check if we have enough info for non-interactive mode
    if not args.name or not args.lib_name or (not args.lib_repo and not args.lib_path):
        return None

    # Parse targets
    targets = [t.strip() for t in args.targets.split(",")]
    for t in targets:
        if t not in AVAILABLE_TARGETS:
            print(f"Error: Unknown target '{t}'. Available: {', '.join(AVAILABLE_TARGETS)}")
            sys.exit(1)

    # Create library source
    if args.lib_repo:
        lib_source = LibrarySource(
            source_type="fetch",
            git_repo=args.lib_repo,
            git_tag=args.lib_tag
        )
    else:
        lib_source = LibrarySource(
            source_type="local",
            local_path=args.lib_path
        )

    # Handle namespace: None means use lib_name as default, "none" means no namespace
    if args.namespace is None:
        namespace = args.lib_name
    elif args.namespace.lower() == "none":
        namespace = ""  # No namespace
    else:
        namespace = args.namespace

    return ProjectConfig(
        name=args.name,
        description=args.description or f"Bindings for {args.lib_name}",
        author=args.author or "",
        version=args.version,
        license=args.license,
        lib_name=args.lib_name,
        lib_source=lib_source,
        lib_include_subdir=args.lib_include_dir,
        lib_src_subdir=args.lib_src_dir,
        namespace=namespace,
        registration_namespace=args.registration_namespace or f"{args.name.replace('-', '_')}_rosetta",
        targets=targets,
        output_dir=args.output or f"./{args.name}"
    )


def main():
    """Main entry point."""
    args = parse_args()

    # Determine mode
    if args.interactive:
        config = interactive_mode()
    else:
        config = config_from_args(args)
        if config is None:
            # Not enough arguments, fall back to interactive
            config = interactive_mode()

    # Create project
    generator = ProjectGenerator(config)

    try:
        generator.generate(force=args.force if hasattr(args, 'force') else False)
        print()
        print("Project created successfully!")
        print()
        print("Next steps:")
        print(f"  1. cd {config.output_dir}")
        print(f"  2. Edit bindings/{config.name.replace('-', '_')}_registration.h to register your classes")
        print("  3. mkdir build && cd build && cmake .. && make")
        print(f"  4. ./{config.name.replace('-', '_')}_generator project.json")
        print()
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
