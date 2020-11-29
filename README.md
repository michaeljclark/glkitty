# glkitty

Port of the OpenGL gears demo to kitty terminal graphics protocol.

![glkitty](/images/glkitty.gif)

## Introduction

_glkitty_ is a nano framework for creating kitty terminal apps that send
image data to kitty using the kitty terminal graphics protocol.

_kitty_gears_ is a demo app for  _kitty_util.h_ and _gl2_util.h_ which
are designed to make using the OpenGL ES2 programmable shader pipeline
tractible for small self-contained demos running with _kitty_.

## Project Structure

- `src/gl1_gears.c` - original GLFW port of the public domain gears demo.
- `src/gl2_gears.c` - OpenGLES2 port of the public domain gears demo.
- `src/gl2_util.h` - header functions for OpenGL ES2 buffers and shaders.
- `src/kitty_gears.c` - OS Mesa kitty port of the public domain gears demo.
- `src/kitty_util.h` - kitty and terminal request response and IO helpers.
- `src/linmath.h` - public domain linear algebra header functions.

## Examples

The project includes several versions of gears ported to multiple APIs.

### kitty_gears

_kitty_gears_ uses off-screen OpenGL to render images into a buffer and
then send them to the kitty terminal using the terminal graphics protocol.
The demo uses poll to capture keyboard input and kitty protocol responses
while rendering and transmitting double buffered Base64 encoded images.
ZLib compression is enabled with the the `-z` flag.

### gl1_gears

_gl1_gears_ is the OpenGL 1.x port of gears using immediate mode
vertices, call lists and the fixed function lighting model. This
code is derived from the gears port included with GLFW.

### gl2_gears

_gl2_gears_ is the OpenGL 2.x port of gears using GLSL shaders.
It has been ported to use Wolfgang Draxinger's `"linmath.h"` `mat4x4`
for constructing the model, view and projection matrices. `"gl2_util.h"`
contains shader loading and a simple vertex and index buffer implementation.

### gl3_gears

_gl3_gears_ is mostly the same as _gl2_gears_ with the addition of vertex
array objects which were added in OpenGL 3.x.

### vk1_gears

vk1_gears is a Vulkan port of gears which includes options for api dump,
validation, variable swapchain images and variable sample buffers.
This is a very early prototype.

## Build Instructions

The project has been tested on Ubuntu 20.04.1 LTS (Focal Fossa).

#### Building dependencies

- kitty version 0.15
- libosmesa - Mesa Off-screen rendering extension
- gcc, cmake, Ninja

Use the following command to install dependencies on _Ubuntu 20.04_:

```
sudo apt-get install -y cmake ninja-build libosmesa6-dev libopengl-dev libvulkan-dev
```

#### Building the demo

The following cmake variables control which examples are built:

- `-DOSMESA_EXAMPLES=ON` - build the OSMesa examples: `kitty_gears`
- `-DOPENGL_EXAMPLES=ON` - build the OpenGL examples: `gl1_gears`, `gl2_gears`
- `-DVULKAN_EXAMPLES=ON` - build the Vulkan examples: `vk1_gears`
- `-DEXTERNAL_GLFW=ON` - build using external GLFW library
- `-DEXTERNAL_GLAD=ON` - build using external GLAD library

By default, the project will build _kitty_gears_ as well as the OpenGL ports
_gl1_gears, _gl2_gears_ and the Vulkan port _vk1_gears_. This command will
download required dependencoes such as GLFW and build all of the examples:

```
cmake -G Ninja -B build
cmake --build build
```

The host GLFW library can be used by overriding appropriate cmake options:

```
cmake -G Ninja -B build -DEXTERNAL_GLFW=OFF -DEXTERNAL_GLAD=OFF
cmake --build build
```

#### Running the demo

```
./build/kitty_gears -s 512x512
```

## Keyboard Navigation

- `q` - quit
- `x` - toggle animation on/off
- `C` and `c` - zoom in or out
- `Z` and `z` - rotate around Z-axis
- `w`, `a`, `s` and `d` - rotate around X-axis and Y-axis

## Licensing Information

- Mesa gears is public domain software, Brian Paul et al.
- gl2_util header is ISC license, Michael Clark.
- kitty_util header is please license, Michael Clark.
- base64_encode is public domain software, Jon Mayo.
