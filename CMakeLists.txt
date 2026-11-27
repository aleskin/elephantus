cmake_minimum_required(VERSION 3.20)

# Makefile.version is the single source of truth for the version (make bump-version).
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/Makefile.version" CE_VERSION)
# Re-run configure when the bump rewrites the file, or the game keeps the old version.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS Makefile.version)
string(STRIP "${CE_VERSION}" CE_VERSION)
if(NOT CE_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "Makefile.version must be MAJOR.MINOR.PATCH, got '${CE_VERSION}'")
endif()

project(circus_elephant VERSION ${CE_VERSION} LANGUAGES C CXX)

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
    src/core/input/input_frame.cpp
    src/core/input/combo.cpp
    src/core/input/pad_layout.cpp
    src/core/input/controls.cpp
    src/core/input/controls_menu.cpp
    src/core/party/minigame.cpp
    src/core/party/flow.cpp
    src/core/settings/config.cpp
    src/core/settings/user_files.cpp
    src/core/time/fixed_step.cpp
    src/core/script/script.cpp
    # Mini-game "circus": the elephant, hoops, levels.
    src/core/minigames/circus/circus.cpp
    src/core/minigames/circus/objects/hoop.cpp
    src/core/minigames/circus/objects/level_object.cpp
    src/core/minigames/circus/level/level.cpp
    src/core/minigames/circus/settings/tunables.cpp
    src/core/minigames/circus/settings/config.cpp
    src/core/minigames/circus/world/world.cpp
    src/core/minigames/circus/game/game.cpp
)
target_include_directories(elephant_core PUBLIC src)
target_compile_definitions(elephant_core PRIVATE CE_VERSION="${CE_VERSION}")
ce_strict(elephant_core)

include(cmake/raylib.cmake)

# The game: window, drawing, keyboard on top of the core.
add_executable(circus_elephant
    src/app/main.cpp
    src/app/script_run.cpp
    src/app/screen_shots.cpp
    src/app/minigames.cpp
    src/input/keyboard.cpp
    src/input/gamepad.cpp
    src/input/menu.cpp
    src/render/camera.cpp
    src/render/text.cpp
    # Screens around the mini-games.
    src/render/screens/menu.cpp
    src/render/screens/title.cpp
    src/render/screens/main_menu.cpp
    src/render/screens/minigame_list.cpp
    src/render/screens/controls.cpp
    # Mini-game "circus".
    src/render/minigames/circus/circus_play.cpp
    src/render/minigames/circus/elephant_view.cpp
    src/render/minigames/circus/scene.cpp
    src/render/minigames/circus/hud.cpp
    src/render/minigames/circus/object_views.cpp
    src/render/minigames/circus/locations/registry.cpp
    src/render/minigames/circus/locations/arena.cpp
    # Sounds made in code.
    src/sound/sounds.cpp
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
        tests/core/test_combo.cpp
        tests/core/test_config.cpp
        tests/core/test_pad_layout.cpp
        tests/core/test_controls.cpp
        tests/core/test_controls_menu.cpp
        tests/core/test_minigame.cpp
        tests/core/test_flow.cpp
        tests/core/test_script.cpp
        tests/core/minigames/circus/test_circus.cpp
        tests/core/minigames/circus/test_level.cpp
        tests/core/minigames/circus/test_world.cpp
    )
    target_link_libraries(elephant_tests PRIVATE elephant_core GTest::gtest_main)
    target_compile_definitions(elephant_tests PRIVATE CE_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
        CE_SCRIPTS_DIR="${CMAKE_SOURCE_DIR}/tests/scripts" CE_VERSION="${CE_VERSION}")
    ce_strict(elephant_tests)
    include(GoogleTest)
    gtest_discover_tests(elephant_tests)
endif()
