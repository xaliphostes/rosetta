{
  "targets": [
    {
      "target_name": "complex",
      
      # ══════════════════════════════════════════════════════════════
      # SOURCE FILES
      # ══════════════════════════════════════════════════════════════
      # Add your C++ source files here
      "sources": [
        "binding.cxx"
      ],
      
      # ══════════════════════════════════════════════════════════════
      # INCLUDE DIRECTORIES
      # ══════════════════════════════════════════════════════════════
      "include_dirs": [
        # node-addon-api headers (required)
        "<!@(node -p \"require('node-addon-api').include\")",
        
        # Rosetta library headers (adjust path as needed)
        "../../../include",
        # Or if you have Rosetta installed elsewhere:
        # "/path/to/rosetta/include",
        
        # Add your own include directories:
        # "include/",
        # "../common/include/",
      ],
      
      # ══════════════════════════════════════════════════════════════
      # DEPENDENCIES
      # ══════════════════════════════════════════════════════════════
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      
      # ══════════════════════════════════════════════════════════════
      # COMPILER FLAGS - ALL PLATFORMS
      # ══════════════════════════════════════════════════════════════
      # Remove default flags that disable exceptions
      "cflags!": [ "-fno-exceptions", "-fno-rtti" ],
      "cflags_cc!": [ "-fno-exceptions", "-fno-rtti" ],
      
      # C++ compiler flags (GCC/Clang)
      "cflags_cc": [ 
        # "-std=c++17",           # C++17 standard (minimum required)
        "-fexceptions",         # Enable exceptions
        "-std=c++20",         # Uncomment for C++20
        # "-O3",                # Uncomment for optimized build
        # "-g",                 # Uncomment for debug build
        # "-Wall",              # Uncomment for all warnings
        # "-Wextra",            # Uncomment for extra warnings
      ],
      
      # ══════════════════════════════════════════════════════════════
      # PREPROCESSOR DEFINES
      # ══════════════════════════════════════════════════════════════
      "defines": [ 
        "NAPI_CPP_EXCEPTIONS",  # Enable N-API C++ exceptions
        # Add your own defines:
        # "MY_CUSTOM_DEFINE",
        # "DEBUG_MODE",
      ],
      
      # ══════════════════════════════════════════════════════════════
      # LIBRARIES TO LINK
      # ══════════════════════════════════════════════════════════════
      # "libraries": [
      #   # Add external libraries here:
      #   # "-lboost_system",
      #   # "-pthread",
      #   # "/path/to/libmylib.a",
      # ],
      
      # ══════════════════════════════════════════════════════════════
      # MACOS SPECIFIC SETTINGS (Xcode)
      # ══════════════════════════════════════════════════════════════
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LANGUAGE_STANDARD": "c++20",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.15",
        "GCC_ENABLE_CPP_RTTI": "YES",
        
        # Additional Xcode settings:
        # "GCC_OPTIMIZATION_LEVEL": "3",          # -O3 optimization
        # "GCC_SYMBOLS_PRIVATE_EXTERN": "YES",    # Hide symbols
        # "DEAD_CODE_STRIPPING": "YES",           # Strip dead code
        
        # Architecture settings:
        # "ARCHS": "$(ARCHS_STANDARD)",           # Universal binary
        # "ONLY_ACTIVE_ARCH": "NO",               # Build for all archs
      },
      
      # ══════════════════════════════════════════════════════════════
      # WINDOWS SPECIFIC SETTINGS (Visual Studio)
      # ══════════════════════════════════════════════════════════════
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1,                 # Enable C++ exceptions
          "AdditionalOptions": [ 
            # "/std:c++17",                         # C++17 standard
            "/std:c++20",                       # Uncomment for C++20
            # "/O2",                              # Uncomment for optimization
            # "/W4",                              # Uncomment for warning level 4
          ],
          # "RuntimeLibrary": 2,                  # Multi-threaded DLL
          # "Optimization": 2,                    # Maximize speed
          # "InlineFunctionExpansion": 2,         # Any suitable
          # "EnableIntrinsicFunctions": "true",   # Enable intrinsics
        },
        # "VCLinkerTool": {
        #   "AdditionalLibraryDirectories": [],   # Add library paths
        #   "AdditionalDependencies": [],         # Add .lib files
        # }
      },
      
      # ══════════════════════════════════════════════════════════════
      # PLATFORM-SPECIFIC CONDITIONS
      # ══════════════════════════════════════════════════════════════
      "conditions": [
        # ────────────────────────────────────────────────────────────
        # Linux-specific
        # ────────────────────────────────────────────────────────────
        ["OS=='linux'", {
          "cflags_cc": [ 
            "-std=c++20", 
            "-fexceptions",
            # "-pthread",                         # Enable threading
            # "-fPIC",                            # Position independent code
          ],
          # "libraries": [
          #   "-pthread",
          # ],
          # "ldflags": [
          #   "-Wl,-rpath,'$$ORIGIN'",            # Set rpath
          # ],
        }],
        
        # ────────────────────────────────────────────────────────────
        # macOS-specific
        # ────────────────────────────────────────────────────────────
        ["OS=='mac'", {
          "cflags_cc": [ 
            "-std=c++20", 
            "-fexceptions",
            # "-stdlib=libc++",                   # Use libc++
          ],
          # "libraries": [
          #   "-framework CoreFoundation",        # Add frameworks
          # ],
          "xcode_settings": {
            # Override or add macOS-specific Xcode settings
          }
        }],
        
        # ────────────────────────────────────────────────────────────
        # Windows-specific
        # ────────────────────────────────────────────────────────────
        ["OS=='win'", {
          "defines": [
            "WIN32",
            "_WINDOWS",
            # "_WIN32_WINNT=0x0601",              # Windows 7+
            # "NOMINMAX",                         # Disable min/max macros
          ],
          "msvs_settings": {
            "VCCLCompilerTool": {
              # Windows-specific compiler options
            }
          }
        }],
        
        # ────────────────────────────────────────────────────────────
        # Debug vs Release builds
        # ────────────────────────────────────────────────────────────
        # ["node_gyp_configuration=='Debug'", {
        #   "defines": [ "DEBUG", "_DEBUG" ],
        #   "cflags_cc": [ "-g", "-O0" ],
        #   "xcode_settings": {
        #     "GCC_OPTIMIZATION_LEVEL": "0",
        #   },
        # }],
        # ["node_gyp_configuration=='Release'", {
        #   "defines": [ "NDEBUG" ],
        #   "cflags_cc": [ "-O3" ],
        #   "xcode_settings": {
        #     "GCC_OPTIMIZATION_LEVEL": "3",
        #   },
        # }],
      ]
    }
  ]
}
