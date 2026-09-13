cmake_minimum_required(VERSION 3.20)
project(SpaceGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(Vulkan REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickWidgets QuickControls2)

# Turns on AUTOMOC, which every Q_OBJECT class here depends on.
qt_standard_project_setup()

# ---------------------------------------------------------------------------
# Shaders: GLSL -> SPIR-V at build time. Orthogonal to Qt; unchanged.
# ---------------------------------------------------------------------------
if(NOT Vulkan_GLSLC_EXECUTABLE)
    message(FATAL_ERROR
        "glslc not found. Install it with: sudo apt install glslc")
endif()

set(SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

set(SHADER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/sprite.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/sprite.frag)

set(SPV_FILES "")
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPV ${SHADER_OUT_DIR}/${SHADER_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SPV}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${SHADER_NAME}"
        VERBATIM)
    list(APPEND SPV_FILES ${SPV})
endforeach()

add_custom_target(shaders DEPENDS ${SPV_FILES})

# ---------------------------------------------------------------------------
# Engine: scene tree, components, collision. No Vulkan dependency, so it can be
# unit tested headless.
# ---------------------------------------------------------------------------
add_library(engine STATIC
    src/engine/Area2D.h
    src/engine/Camera2D.cpp
    src/engine/Camera2D.h
    src/engine/Collision.cpp
    src/engine/Collision.h
    src/engine/Connection.h
    src/engine/Input.cpp
    src/engine/Input.h
    src/engine/Math.h
    src/engine/Node.cpp
    src/engine/Node.h
    src/engine/Node2D.cpp
    src/engine/Node2D.h
    src/engine/RenderQueue.cpp
    src/engine/RenderQueue.h
    src/engine/SceneTree.cpp
    src/engine/SceneTree.h
    src/engine/Shapes.h
    src/engine/Signal.h
    src/engine/Sprite.cpp
    src/engine/Sprite.h
    src/engine/View2D.h)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(engine PUBLIC Qt6::Core)
target_compile_options(engine PRIVATE -Wall -Wextra)

enable_testing()
add_executable(engine_tests tests/engine_tests.cpp)
target_link_libraries(engine_tests PRIVATE engine)
target_compile_options(engine_tests PRIVATE -Wall -Wextra)
add_test(NAME engine_tests COMMAND engine_tests)

# ---------------------------------------------------------------------------
# Application
# ---------------------------------------------------------------------------
qt_add_executable(space_game
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/VulkanWindow.cpp
    src/VulkanWindow.h
    src/SceneRenderer.cpp
    src/SceneRenderer.h
    src/backend/asteroid.cpp
    src/backend/asteroid.h
    src/backend/level.cpp
    src/backend/level.h
    src/backend/player_ship.cpp
    src/backend/player_ship.h
    src/Controller.cpp
    src/Controller.h
    src/Pipeline.cpp
    src/Pipeline.h
    src/Buffer.cpp
    src/Buffer.h
    src/Vertex.h
    src/VkCommon.h)

add_dependencies(space_game shaders)

# Absolute path to the compiled shaders, so the binary runs from any cwd.
target_compile_definitions(space_game PRIVATE SHADER_DIR="${SHADER_OUT_DIR}")

# Ships the QML inside the binary as qrc:/qml/Controls.qml.
qt_add_resources(space_game "qml"
    PREFIX "/"
    FILES qml/Controls.qml)

target_link_libraries(space_game PRIVATE
    engine
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickWidgets
    Qt6::QuickControls2
    Vulkan::Vulkan)

target_compile_options(space_game PRIVATE -Wall -Wextra)
