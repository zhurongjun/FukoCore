cmake_minimum_required(VERSION 3.16.0)

project(FukoCore VERSION 0.0.0 LANGUAGES CXX)

# cxx settings 
set(CMAKE_CXX_STANDARD 17)

# debug define
if (NOT CMAKE_BUILD_TYPE)
    add_compile_definitions(FUKO_DEBUG=1)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
elseif(CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
    add_compile_definitions(FUKO_DEBUG=1)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
    add_compile_definitions(FUKO_DEBUG=0)
endif()

# setup output dir
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# module 
add_subdirectory(Code/Core)
add_subdirectory(Code/TestCore)