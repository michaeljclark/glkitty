cmake_minimum_required (VERSION 3.12)

project(glkitty VERSION 0.0.1)

add_executable(kitty_gears src/kitty_gears.c)
target_link_libraries(kitty_gears OSMesa m)

add_executable(gl1_gears src/gl1_gears.c)
target_link_libraries(gl1_gears glfw GL m)

add_executable(gl2_gears src/gl2_gears.c)
target_link_libraries(gl2_gears glfw GL m)
