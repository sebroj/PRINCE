{
    "targets": [
        {
            "target_name": "main",
            "sources": [
                "src/node_main.cpp",
                "src/load_data.cpp",
                "src/extern/tinyexpr/tinyexpr.c"
            ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")"
            ],
            "conditions": [
                ["OS=='win'", {
                    'msvs_settings': {
                        'VCCLCompilerTool': {
                            'WarningLevel': 4,
                            'WarnAsError': 'true',
                        }
                    },
                    'msvs_disabled_warnings': [
                        4100, # unreferenced formal parameter
                        4201, 4152, 4204, 4055, 4090 # C casts in tinyexpr.c
                    ]
                }]
            ]
        }
    ]
}
