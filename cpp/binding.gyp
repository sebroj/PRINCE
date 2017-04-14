{
    "targets": [
        {
            "target_name": "main",
            "sources": [
                "src/node_main.cpp",
                "src/load_data.cpp"
            ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")"
            ],
            "conditions": [
                ["OS=='win'", {
                }]
            ]
        }
    ]
}
