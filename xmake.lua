-- use c++ 17
set_languages("c++17")

-- rules
add_rules("mode.debug","mode.release")

-- function
function setup_module(Name)
    add_defines(Name .. "_EXPORT")
end

function depend_module(Name)
    add_includedirs("../" .. Name .. "/Include/")
    add_deps("FukoCore")
end

-- modules
includes("FukoCore")
includes("TestCore")