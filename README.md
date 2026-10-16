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
`JupiterExample --smoke-test`. The example exits cleanly after rendering 120
frames.

## Regression tests

The headless render failure tests use fake window and graphics backends with the
real application, render thread, and queue. They cover failure propagation,
blocked submissions, cleanup ordering, and restart without needing a display.

```sh
cmake -S . -B build -DJUPITER_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
