# glkitty

Port of the OpenGL gears demo to kitty terminal graphics protocol.

_kitty_gears_ uses off-secreen OpenGL to render images into a buffer and
then send them to the kitty terminal using the terminal graphics protocol.
The demo uses poll to capture keyboard input and kitty protocol responses
while rendering and transmitting double buffered Base64 encoded images.

## Keyboard Navigation

- `q` - quit
- `x` - toggle animation on/off
- `C` and `c` - zoom in or out
- `Z` and `z` - rotate around Z-axis
- `w`, `a`, `s` and `d` - rotate around X-axis and Y-axis

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
