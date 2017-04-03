{
    "targets": [
        {
            "target_name": "main",
            "sources": [
                "node_main.cpp",
                "load_data.cpp"
            ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")"
            ]
        }
    ]
}
