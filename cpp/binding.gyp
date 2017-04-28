{
    "targets": [
        {
            "target_name": "main",
            "sources": [
                "src/node_main.cpp",
                "src/parameters.cpp",
                "extern/exprtk/src/exprtk.cpp"
            ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")",
                "src",
                "extern/gsl/include",
                "extern/exprtk/include"
            ],
            "conditions": [
                ["OS=='win'", {
                    "msvs_settings": {
                        "VCCLCompilerTool": {
                            "ExceptionHandling": 1,
                            "WarningLevel": 4,
                            "WarnAsError": "true",
                            "AdditionalOptions": [
                                "/bigobj"
                            ]
                        },
                        "VCLinkerTool": {
                            "AdditionalDependencies": [
                                "gsl.lib"
                            ],
                            "AdditionalLibraryDirectories": [
                                "<(module_root_dir)/extern/gsl/lib"
                            ]
                        }
                    },
                    "msvs_disabled_warnings": [
                        4100, # unreferenced formal parameter
                        4541, 4267, 4996 # things in exprtk (ugh)
                    ]
                }],
                ["OS=='mac'", {
                    "xcode_settings": {
                        # TODO add warning handling?
                    }
                }]
            ]
        }
    ]
}
