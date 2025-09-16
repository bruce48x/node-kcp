{
    "targets": [
        {
            "target_name": "kcp",
            'cflags!': [ '-fno-exceptions' ],
            'cflags_cc!': [ '-fno-exceptions' ],
            'conditions': [
              ["OS=='win'", {
                "defines": [
                  "_HAS_EXCEPTIONS=1"
                ],
                "msvs_settings": {
                  "VCCLCompilerTool": {
                    "ExceptionHandling": 1,
                    "AdditionalOptions": [ "/utf-8" ]
                  },
                },
              }],
              ["OS=='mac'", {
                'xcode_settings': {
                  'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                  'CLANG_CXX_LIBRARY': 'libc++',
                  'MACOSX_DEPLOYMENT_TARGET': '10.7',
                },
              }],
            ],
            'include_dirs': [
              "<!(node -p \"require('node-addon-api').include_dir\")"
            ],
            "sources": [
              "src/ikcp.c",
              "src/kcpobject.cc",
              "src/node-kcp.cc",
              "src/reedsolomon.cpp",
              "src/matrix.cpp",
              "src/galois.cpp",
              "src/galois_table.cpp",
              "src/galois_noasm.cpp",
              "src/fec.cpp",
              "src/inversion_tree.cpp",
            ]
        }
    ]
}
