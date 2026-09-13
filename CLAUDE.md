# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build, run, test

C++17, CMake ≥ 3.20, Qt 6 (Core Gui Widgets Qml Quick QuickWidgets QuickControls2), Vulkan, and `glslc` (`sudo apt install glslc`). Default build type is Debug.

    cmake -S . -B build
    cmake --build build -j
    ./build/space_game                 # run the game
    ./build/engine_tests               # or: ctest --test-dir build

- Shaders in `shaders/` are compiled to SPIR-V at build time into `build/shaders/`; the binary locates them via the absolute `SHADER_DIR` compile definition, so it runs from any cwd.
- `qml/Controls.qml` is embedded as `qrc:/qml/Controls.qml` — rebuild after editing it.
- New source files must be added by hand to the matching target list in `CMakeLists.txt` (`engine` library or `space_game` executable).
- Tests (`tests/engine_tests.cpp`) use GoogleTest (`libgtest-dev`, found via `find_package(GTest)`). Run a single test with `./build/engine_tests --gtest_filter=Engine.TestName`. New tests need a `TEST(Engine, Name)` block; no registration elsewhere is required.

## Architecture

Three layers:

1. **`src/engine/`** (static lib `engine`, namespace `engine`) — a Godot-style scene-graph engine. Depends only on `Qt6::Core` (for `Qt::Key`), **never on Vulkan**, so it is unit-testable headless. Keep it that way.
2. **`src/backend/`** — game content built on the engine (`Level`, `PlayerShip`, `Asteroid`). Despite the name, this is gameplay, not a render backend.
3. **`src/*.cpp` top level** — Qt/Vulkan app shell and renderer.

### Engine concepts
- **Node tree / composition**: `Node` owns children via `addChild<T>(...)`. Game objects are `Node2D`s composed of child components (`Sprite`, `Area2D`, `Camera2D`). Override `ready()`, `update(dt)`, `draw(RenderQueue&)`, `onEnterTree()`/`onExitTree()`.
- **Lifetime**: nodes are destroyed only via `queueFree()`, flushed at the end of `SceneTree::tick()`; raw pointers from `addChild()` stay valid through the frame.
- **"Call down, signal up"**: parents call methods on children; children emit `engine::Signal<...>` that parents connect to. `Signal::connect(receiver, fn)` auto-disconnects when the receiver is freed; `ConnectionType::Deferred` runs the slot in the tick's deferred flush (`SceneTree::callDeferred`).
- **Frame order** (`SceneTree::tick`): update → collision (`Area2D` enter/exit signals) → deferred calls → frees → view update from current `Camera2D`. Then `buildRenderQueue()`.
- **Collision**: `Area2D` detects another when `(mask & other.layer) != 0`; only the detecting side emits signals.
- **Coordinates**: `View2D` shows a fixed world height (`visibleHeight`, default 36 units) regardless of window size; +y is down; rotation is radians, clockwise on screen.
- **Rendering is backend-agnostic**: nodes push world-space triangles into `RenderQueue`, which z-sorts (stable) into `Vertex2D`. `Vertex2D` layout must stay in sync with `src/Vertex.h`.

### App shell / rendering
- `main.cpp` wires it up: `QVulkanInstance` → `Controller` → `SceneTree` with a `Level` root child → `VulkanWindow` → `MainWindow`. Declaration order matters for lifetimes (instance and tree must outlive the window).
- `SceneRenderer` (a `QVulkanWindowRenderer`) **is the game loop**: in `startNextFrame()` it sets `view.screenSize`, ticks the tree, builds the render queue, and uploads one batched triangle list (one vertex buffer per in-flight frame) through `Pipeline`/`Buffer`.
- `VulkanWindow` forwards key events to `SceneTree::input` (`engine::Input`, polled with `isKeyDown`/`isKeyJustPressed`).
- `MainWindow` is a Widgets shell: Vulkan view in a native window container beside a `QQuickWidget` running `Controls.qml`. Native containers stack above siblings, so QML cannot overlay the Vulkan view. Target is Qt 6.4 (no `QQuickRhiItem`).
- **UI → renderer seam**: add a `Q_PROPERTY` to `Controller`, bind it in `Controls.qml` (exposed as context property `controller`), read it in `SceneRenderer`. Everything runs on the main thread, so no locking.
- Include `<vulkan/vulkan.h>` **before** any Qt Vulkan header (Qt defines `VK_NO_PROTOTYPES`).
