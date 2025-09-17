{
  "targets": [
    {
      "target_name": "sqlanywhere",
      "defines": [ '_SACAPI_VERSION=5', 'DRIVER_NAME=sqlanywhere' ],
      "sources": [
        "src/sqlanywhere.cpp",
        "src/utils.cpp",
        "src/connection.cpp",
        "src/stmt.cpp",
        "src/sacapidll.cpp",
        "src/async_workers.cpp"
      ],
      "include_dirs": [
          "src/h",
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],

      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1
        }
      },
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
      }
    }
  ]
}