target("FukoCore")
    set_kind("shared")

    add_includedirs("./Include/")

    setup_module("Core")

    add_files("./Source/*/*.cpp")
    