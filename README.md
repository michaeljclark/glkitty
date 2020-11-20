# glkitty

Port of the OpenGL gears demo to kitty terminal graphics protocol.

_kitty_gears_ uses off-secreen OpenGL to render images into a buffer and
then send them to the kitty terminal using the terminal graphics protocol.
The demo uses poll to capture keyboard input and kitty protocol responses
while rendering and transmitting double buffered Base64 encoded images.
ZLib compression is enabled with the the `-z` flag.

![glkitty](/images/glkitty.gif)

## Keyboard Navigation

- `q` - quit
- `x` - toggle animation on/off
- `C` and `c` - zoom in or out
- `Z` and `z` - rotate around Z-axis
- `w`, `a`, `s` and `d` - rotate around X-axis and Y-axis

## Project Structure

- `src/gl1_gears.c` - original GLFW port of the public domain gears demo.
- `src/gl2_gears.c` - OpenGLES2 port of the public domain gears demo.
- `src/gl2_util.h` - header functions for OpenGL ES2 buffers and shaders.
- `src/kitty_gears.c` - OS Mesa kitty port of the public domain gears demo.
- `src/kitty_util.h` - kitty and terminal request response and IO helpers.
- `src/linmath.h` - public domain linear algebra header functions.

## Licensing Information

- Mesa gears is public domain software, Brian Paul et al.
- gl2_util header is ISC license, Michael Clark.
- kitty_util header is please license, Michael Clark.
- base64_encode is public domain software, Jon Mayo.

## Build Dependencies

- kitty version 0.15
- libosmesa - Mesa Off-screen rendering extension
- gcc, cmake, Ninja

## Build Instructions

- Tested on Ubuntu 20.04.1 LTS (Focal Fossa)

#### Building the demo

```
sudo apt-get install -y cmake ninja-build libosmesa6-dev
cmake -G Ninja -B build .
cmake --build build
```

#### Running the demo:

```
./build/kitty_gears -s 512x512
```
