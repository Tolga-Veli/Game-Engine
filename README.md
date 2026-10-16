# Jupiter Game Engine

Jupiter is an experimental, performance-oriented C++23 game engine. The current
renderer uses OpenGL 4.5 with GLFW, GLAD, GLM, ImGui, and stb.

The included `JupiterExample` displays a rotating, color-cycling cube and is the
quickest way to verify that the engine, asset system, render thread, and shutdown
path are working together.

## Requirements

- Git
- CMake 3.20 or newer
- A C++23 compiler with `std::format` support
- Ninja or another CMake-supported build tool
- An OpenGL 4.5-capable driver

On Debian or WSL2 Debian, the typical native dependencies are:

```sh
sudo apt update
sudo apt install build-essential cmake ninja-build libgl1-mesa-dev xorg-dev
```

WSL users also need graphics support such as WSLg and an up-to-date Windows GPU
driver.

## Build and run

Clone the repository with its pinned dependencies:

```sh
git clone --recurse-submodules https://github.com/Tolga-Veli/Game-Engine.git
cd Game-Engine
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/bin/JupiterExample
```

For a release build, configure a separate directory with
`-DCMAKE_BUILD_TYPE=Release`.

On Windows, run `build\\bin\\JupiterExample.exe` from the repository root or
from `build\\bin`; the asset directory is copied next to the executable after
every build.

To build only the engine library, configure with
`-DJUPITER_BUILD_EXAMPLE=OFF`.

For an automated launch/render/shutdown check, run
`JupiterExample --smoke-test`. The example exits cleanly after submitting 120
frames; shutdown may cancel frames still in the queue.

`PlanetRendererExample --smoke-test` does the same for the instanced renderer.

## Render-thread work

`RendererAPI::SubmitRenderTask` accepts move-only callables from any producer
thread. The callable receives a `RenderExecutionContext` with backend-neutral
renderer and resource-factory interfaces. The render thread executes it with
its graphics context current. Captures must own their CPU input data; pass
asset handles instead of sharing mutable GPU-backed objects across threads.
`AssetManager::Resolve` is render-thread-only.
`SubmitRenderTaskAsync` also returns a future for CPU completion or task
failure; cancelled tasks make that future report `broken_promise`.

`FrameQueue` holds complete frame snapshots and `RendererCommandQueue` holds
render tasks. `RenderScheduler` gives each accepted task and committed frame
a ticket under one mutex, then runs them in ticket order. Frames execute as
units through presentation. A task submitted before a frame is committed runs
before that frame; a task submitted afterward runs after it. There is no
implicit ordering between concurrent producer threads: join dependent
simulation work before committing its frame.

The existing `BeginFrame` / `SubmitDrawCommand` / `EndFrame` API is for one
frame coordinator. Workers can prepare separate draw lists; the coordinator
may assemble a `Frame` and call `CommitFrame` after they finish. At most two
frames are outstanding, so committing can block until the render thread
releases a frame. Stop wakes blocked producers and rejects new submissions.
Pending task captures are destroyed on the render thread before the graphics
context shuts down. Task completion means CPU execution on the render thread;
it does not imply GPU completion.

The command queue accepts up to 4096 pending tasks. Submission returns `false`
when stopped or full; producers should batch work and handle rejection without
blocking the render consumer. Completed frame vectors are recycled. For large
draw lists, reserve a local `Frame::commands` vector and publish it once with
`CommitFrame`. Builder calls must stay on the thread that began the frame.

Asset file reads and parsing run on a loader thread with up to 256 pending
jobs. GPU creation and shader compilation remain on the render thread.
`AssetManager::Load` returns a pending handle; use `GetState(handle)` to check
`Pending`, `Ready`, `Failed`, or `Invalid` from any thread. A later render task
does not wait for a file load to finish. Runtime `Create` moves its owned data
into an ordered render task; pass large descriptors with `std::move` when the
caller no longer needs them. Queue saturation returns an invalid handle or
makes a pending load fail. A failed hot reload keeps the previous working
resource. `Unload(handle)` schedules release in render order and invalidates
the handle's generation; a new load of the same name then gets a new handle.

Mesh capacities allocate their corresponding byte storage, and invalid updates
or draw counts are rejected. Dynamic mesh updates replace the complete active
vertex/index data. Stream buffers wait for their previous GPU consumers before
overwriting mapped storage. Mesh draws mark these consumers automatically;
custom users of `IBuffer` must call `MarkUsed()` after their GPU commands.
`DrawCommand::DisableDepthWrite()` supports transparent passes independently of
depth testing. Draw order, culling, and transparent sorting remain caller-owned.

The OpenGL renderer caches adjacent resource/state bindings and unchanged
material parameters within a frame and
invalidates them around tasks and overlays. Call `context.renderer.InvalidateState()`
when mixing raw graphics API calls with renderer submissions inside one task.
Texture material parameters support `TextureBinding` with an optional sampler
created through `AssetManager::Create<Sampler>`.

The only implemented graphics backend is OpenGL. The planet example's ImGui
overlay uses its OpenGL backend directly; a second graphics backend will need
its own UI renderer. GLFW input/cursor setup and teardown run on the main
thread. UI context access is synchronized, and draw data remains alive until
the previous overlay finishes before ImGui starts another frame.
The bundled GLFW submodule also has a local fix releasing the previous
Wayland keyboard compose state when a new keymap arrives.

## Regression tests

The headless queue tests use fake RHI interfaces. They cover concurrent
producers, command/frame order, blocked frame submissions, cancellation,
capture destruction, and queue restart without needing a display.

```sh
cmake -S . -B build -DJUPITER_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
