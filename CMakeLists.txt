cmake_minimum_required (VERSION 3.12)

project(glkitty VERSION 0.0.1)

add_executable(kitty_gears src/kitty_gears.c)
target_link_libraries(kitty_gears OSMesa m)
