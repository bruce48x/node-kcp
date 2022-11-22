{
    "targets": [
        {
            "target_name": "kcp",
            "include_dirs": [
                "<!(node -e \"require('nan')\")"
            ],
            "sources": [
                "src/ikcp.c",
                "src/kcpobject.cc",
                "src/node-kcp.cc"
            ]
        }
    ]
}
