file(GLOB_RECURSE CORE_SRC Source/*.cpp)

add_library(Core SHARED ${CORE_SRC})

target_include_directories(Core PUBLIC Include)
target_compile_definitions(Core PRIVATE EXPORT_CORE)
