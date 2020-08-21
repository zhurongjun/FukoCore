-- use c++ 17
set_languages("c++17")

-- modes
add_rules("mode.debug", "mode.release")

-- global define
if is_mode("debug") then
	add_defines("FUKO_DEBUG = 1")
else
	add_defines("FUKO_DEBUG = 0")
end

-- win include 
if is_plat("windows") then
    add_includedirs("D:/Windows Kits/10/Include/10.0.17763.0/um")
end

-- modules function 
codePath = path.absolute("./Code/") .. "/"
function static_module(Name)
    target(Name)
        set_kind("static")
        add_includedirs("./Include/")
        add_files("./Source/**.cpp")
end
function dynamic_module(Name)
    target(Name)
        set_kind("shared")
        add_defines(string.upper(Name) .. "_API=__declspec(dllexport)")
        add_includedirs("./Include/")
        add_files("./Source/**.cpp")
end
function execute_module(Name)
    target(Name)
        set_kind("binary")
        add_includedirs("./Include/")
        add_files("./Source/**.cpp")
end
function depend_module(Name)
    add_includedirs(codePath .. Name .. "/Include/")
    add_deps(Name)
	add_defines(string.upper(Name) .. "_API=__declspec(dllimport)")
end

-- modules
includes(codePath .. "Core")
includes(codePath .. "JobSystem")
includes(codePath .. "Gfx")
includes(codePath .. "TestCore")
includes(codePath .. "TestGfx")