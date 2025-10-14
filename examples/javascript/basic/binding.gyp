{
  "targets": [
    {
      "target_name": "basic",
      "sources": [
        "binding.cxx"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "<(module_root_dir)/../..",
        "<(module_root_dir)/../../../include"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions", "-fno-rtti" ],
      "cflags_cc!": [ "-fno-exceptions", "-fno-rtti" ],
      "cflags": [ "-std=c++20" ],
      "cflags_cc": [ "-std=c++20" ],
      "defines": [ "NAPI_CPP_EXCEPTIONS" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "GCC_ENABLE_CPP_RTTI": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.14",
        "OTHER_CFLAGS": [ "-std=c++20" ]
      },
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1,
          "RuntimeTypeInfo": "true",
          "AdditionalOptions": [ "/std:c++20", "/GR" ]
        }
      },
      "conditions": [
        ["OS=='win'", {
          "defines": [
            "_HAS_EXCEPTIONS=1"
          ]
        }]
      ]
    }
  ]
}