file(GLOB_RECURSE TESTCORE_SRC Source/*.cpp)

add_executable(TestCore ${TESTCORE_SRC})

target_include_directories(TestCore PRIVATE Include)
target_link_libraries(TestCore PUBLIC Core)