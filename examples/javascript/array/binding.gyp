{
  "targets": [
    {
      "target_name": "rosetta",
      "sources": [
        "binding.cxx"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "<(module_root_dir)/../../../include"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions", "-fno-rtti" ],
      "cflags_cc!": [ "-fno-exceptions", "-fno-rtti" ],
      "cflags_cc": [
        "-std=c++20",
        "-fexceptions",
        "-frtti"
      ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "GCC_ENABLE_CPP_RTTI": "YES",
        "CLANG_CXX_LANGUAGE_STANDARD": "c++20",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.14",
        "OTHER_CPLUSPLUSFLAGS": [
          "-std=c++20",
          "-fexceptions",
          "-frtti"
        ]
      },
      "msvs_settings": {
        "VCCLCompilerTool": { 
          "ExceptionHandling": 1,
          "RuntimeTypeInfo": "true",
          "AdditionalOptions": [
            "/std:c++20"
          ]
        }
      },
      "conditions": [
        ["OS==\"win\"", {
          "defines": [
            "_HAS_EXCEPTIONS=1"
          ]
        }]
      ]
    }
  ]
}