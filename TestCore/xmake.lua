target("TestCore")
    set_kind("binary")

    add_includedirs("./Include/")
    depend_module("FukoCore")

    add_files("./Source/TestMain.cpp")