#           ███▌     █▌        ▐█      ▗▄       ▄▖
#           ▀▀█▌     █▌        ▐█      ▐█       █▌
#             █▌     █▌                ▐█       █▌
#   ▗██▙▐█    █▌     █▌  ▗█▘ ▐███    ███████ ▐██████▌ █▌    ▟▛
#  ▗█▛▀▜██    █▌     █▌ ▗█▘  ▝▀▜█    ▀▀▜█▀▀▀ ▝▀▀█▛▀▀▘ ▐█    █▌
#  ▟█   ▜█    █▌     █▌▗█▘     ▐█      ▐█       █▌    ▝█▖  ▐█
#  █▌   ▐█    █▌     █▙█▌      ▐█      ▐█       █▌     ▜▌  ▟▛
#  █▌   ▐█    █▌     ██▜█▖     ▐█      ▐█       █▌     ▐█  █▘
#  █▌   ▐█    █▌     █▌ ▜▙     ▐█      ▐█       █▌      █▌▐█
#  ▜█   ▟█    █▌     █▌ ▝█▙    ▐█      ▐█       █▌      ▜▙█▌
#  ▝█▙▄▟██    ▐█▄▄▖  █▌  ▝█▖ ▄▄▟█▄▄▄   ▐█▙▄▄    ██▄▄▖   ▝██
#   ▝██▛▐█     ▜██▌  █▌   ▜█ ███████    ▀███    ▝▜██▌    █▛
#       ▐█                                               █▌
#   ▄  ▗▟▛                                              ▟█
#   █████▘                                            ▐██▌
#    ▀▀▀                                              ▝▀▘

cmake_minimum_required (VERSION 3.12)
project(glkitty VERSION 0.0.1)

include(CheckLibraryExists)
check_library_exists(m sqrtf "" HAVE_LIB_M)
if (HAVE_LIB_M)
    set(EXTRA_LIBS ${EXTRA_LIBS} m)
endif (HAVE_LIB_M)

find_package(PkgConfig)
pkg_check_modules(OSMESA REQUIRED osmesa)
pkg_check_modules(GLFW3 REQUIRED glfw3)
pkg_check_modules(ZLIB REQUIRED zlib)

# Find OpenGL library
include(FindOpenGL)

# set defaults for options
set (OSMESA_EXAMPLES_DEFAULT ON)
set (OPENGL_EXAMPLES_DEFAULT ON)
set (VULKAN_EXAMPLES_DEFAULT ON)
if (NOT OSMESA_FOUND)
    set (OSMESA_EXAMPLES_DEFAULT OFF)
endif ()
if (NOT OpenGL_OpenGL_FOUND)
    set (OPENGL_EXAMPLES_DEFAULT OFF)
endif ()
if (NOT VULKAN_FOUND)
    set (VULKAN_EXAMPLES_DEFAULT OFF)
endif ()

# user configurable options
option(OSMESA_EXAMPLES "Build OSMesa examples" ${OSMESA_EXAMPLES_DEFAULT})
option(OPENGL_EXAMPLES "Build OpenGL examples" ${OPENGL_EXAMPLES_DEFAULT})
option(EXTERNAL_GLFW "Use external GLFW project" ON)
option(EXTERNAL_GLAD "Use external GLAD project" ON)

message(STATUS "OSMESA_EXAMPLES = ${OSMESA_EXAMPLES}")
message(STATUS "OPENGL_EXAMPLES = ${OPENGL_EXAMPLES}")
message(STATUS "VULKAN_EXAMPLES = ${VULKAN_EXAMPLES}")
message(STATUS "EXTERNAL_GLFW = ${EXTERNAL_GLFW}")
message(STATUS "EXTERNAL_GLAD = ${EXTERNAL_GLAD}")

# Add OSMesa library and flags if found
if (OSMESA_FOUND)
    add_definitions(${OSMESA_CFLAGS})
    set(OSMESA_LIBS_ALL ${OSMESA_LDFLAGS} ${EXTRA_LIBS})
endif ()

# Add ZLib library and flags if found
if (ZLIB_FOUND)
    list(APPEND OSMESA_LIBS_ALL ${ZLIB_LDFLAGS})
    add_definitions(-DHAVE_ZLIB)
endif ()

# Support for external glfw
if (EXTERNAL_GLFW)
    include(ExternalProject)
    ExternalProject_Add(
        GLFW
        GIT_REPOSITORY "https://github.com/glfw/glfw.git"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third_party/glfw3"
        STEP_TARGETS build
        EXCLUDE_FROM_ALL TRUE
        CMAKE_ARGS -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF
                   -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF
    )
    set(GLFW_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/third_party/glfw3/include")
    set(GLFW_LIBRARY_DIR "${CMAKE_BINARY_DIR}/GLFW-prefix/src/GLFW-build/src/")
    set(GLFW_LIBRARIES glfw3 pthread dl)
    include_directories(${GLFW_INCLUDE_DIRS})
    link_directories(${GLFW_LIBRARY_DIR})
    set(GLFW_LIBS_ALL ${GLFW_LIBRARIES} ${OPENGL_opengl_LIBRARY} ${EXTRA_LIBS})
else ()
    add_definitions(${GLFW3_CFLAGS})
    set(GLFW_LIBS_ALL ${GLFW3_LDFLAGS} ${OPENGL_opengl_LIBRARY} ${EXTRA_LIBS})
endif ()

# Support for external glad
if (EXTERNAL_GLAD)
    include(ExternalProject)
    ExternalProject_Add(
        GLAD
        GIT_REPOSITORY "https://github.com/Dav1dde/glad.git"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third_party/glad"
        STEP_TARGETS build
        EXCLUDE_FROM_ALL TRUE
        CMAKE_ARGS -DGLAD_EXPORT=OFF
    )
    set(GLAD_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/GLAD-prefix/src/GLAD-build/include/")
    set(GLAD_LIBRARY_DIR "${CMAKE_BINARY_DIR}/GLAD-prefix/src/GLAD-build/")
    set(GLAD_LIBRARIES glad)
    include_directories(${GLAD_INCLUDE_DIRS})
    link_directories(${GLAD_LIBRARY_DIR})
    add_definitions(-DHAVE_GLAD)
endif ()

if (OSMESA_EXAMPLES)
    message("-- Adding: glkitty")
    add_executable(kitty_gears src/kitty_gears.c)
    target_link_libraries(kitty_gears ${OSMESA_LIBS_ALL})
endif (OSMESA_EXAMPLES)

if (OPENGL_EXAMPLES)
    message("-- Adding: gl1_gears")
    add_executable(gl1_gears src/gl1_gears.c)
    target_link_libraries(gl1_gears ${GLFW_LIBS_ALL})

    message("-- Adding: gl2_gears")
    add_executable(gl2_gears src/gl2_gears.c)
    target_link_libraries(gl2_gears ${GLFW_LIBS_ALL})

    if (EXTERNAL_GLFW)
        add_dependencies(gl1_gears GLFW-build)
        add_dependencies(gl2_gears GLFW-build)
    endif ()

    if (EXTERNAL_GLAD)
        add_dependencies(gl1_gears GLAD-build)
        add_dependencies(gl2_gears GLAD-build)
        target_link_libraries(gl1_gears ${GLAD_LIBRARIES})
        target_link_libraries(gl2_gears ${GLAD_LIBRARIES})
    endif ()
endif (OPENGL_EXAMPLES)
