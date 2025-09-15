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
                    "AdditionalOptions": ["/utf-8"]
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
              "<!(node -p \"require('node-addon-api').include_dir\")",
              "deps/libcorrect/include"
            ],
            "sources": [
              "src/ikcp.c",
              "src/kcpobject.cc",
              "src/node-kcp.cc",
              "src/rs_wrapper.cc",
              "deps/libcorrect/src/reed-solomon/encode.c",
              "deps/libcorrect/src/reed-solomon/decode.c",
              "deps/libcorrect/src/reed-solomon/polynomial.c",
              "deps/libcorrect/src/reed-solomon/reed-solomon.c",
              "deps/libcorrect/src/fec_shim.c",
              "deps/libcorrect/src/convolutional/bit.c",
              "deps/libcorrect/src/convolutional/convolutional.c",
              "deps/libcorrect/src/convolutional/decode.c",
              "deps/libcorrect/src/convolutional/encode.c",
              "deps/libcorrect/src/convolutional/error_buffer.c",
              "deps/libcorrect/src/convolutional/history_buffer.c",
              "deps/libcorrect/src/convolutional/lookup.c",
              "deps/libcorrect/src/convolutional/metric.c"
            ]
        }
    ]
}
