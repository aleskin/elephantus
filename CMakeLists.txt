cmake_minimum_required(VERSION 3.20)

project(circus_elephant LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Strict warnings for our own targets only: raylib and GoogleTest are built as is.
function(ce_strict target)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endfunction()

# Game core: rules, physics, levels. Pure C++, no raylib, no output.
add_library(elephant_core STATIC
    src/core/game_info.cpp
    src/core/text/kv_file.cpp
    src/core/objects/hoop.cpp
    src/core/objects/level_object.cpp
    src/core/level/level.cpp
)
target_include_directories(elephant_core PUBLIC src)
ce_strict(elephant_core)

include(cmake/raylib.cmake)

# The game: window, drawing, keyboard on top of the core.
add_executable(circus_elephant
    src/app/main.cpp
    src/render/camera.cpp
    src/render/object_views.cpp
    src/render/locations/registry.cpp
    src/render/locations/arena.cpp
)
target_link_libraries(circus_elephant PRIVATE elephant_core raylib)
ce_strict(circus_elephant)

option(CE_BUILD_TESTS "Build core unit tests" ON)
if(CE_BUILD_TESTS)
    enable_testing()
    include(cmake/googletest.cmake)

    add_executable(elephant_tests
        tests/core/test_game_info.cpp
        tests/core/test_kv_file.cpp
        tests/core/test_level.cpp
    )
    target_link_libraries(elephant_tests PRIVATE elephant_core GTest::gtest_main)
    target_compile_definitions(elephant_tests PRIVATE CE_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")
    ce_strict(elephant_tests)
    include(GoogleTest)
    gtest_discover_tests(elephant_tests)
endif()
