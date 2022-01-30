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
pkg_check_modules(OSMESA osmesa)
pkg_check_modules(ZLIB zlib)
pkg_check_modules(VULKAN vulkan)

# Find OpenGL library
include(FindOpenGL)

# set defaults for options
set (OSMESA_EXAMPLES_DEFAULT ${OSMESA_FOUND})
set (OPENGL_EXAMPLES_DEFAULT ${OpenGL_OpenGL_FOUND})
set (VULKAN_EXAMPLES_DEFAULT ${VULKAN_FOUND})

# user configurable options
option(OSMESA_EXAMPLES "Build OSMesa examples" ${OSMESA_EXAMPLES_DEFAULT})
option(OPENGL_EXAMPLES "Build OpenGL examples" ${OPENGL_EXAMPLES_DEFAULT})
option(VULKAN_EXAMPLES "Build Vulkan examples" ${VULKAN_EXAMPLES_DEFAULT})
option(EXTERNAL_GLFW "Use external GLFW project" ON)
option(EXTERNAL_GLAD "Use external GLAD project" ON)

message(STATUS "OSMESA_EXAMPLES = ${OSMESA_EXAMPLES}")
message(STATUS "OPENGL_EXAMPLES = ${OPENGL_EXAMPLES}")
message(STATUS "VULKAN_EXAMPLES = ${VULKAN_EXAMPLES}")
message(STATUS "EXTERNAL_GLFW = ${EXTERNAL_GLFW}")
message(STATUS "EXTERNAL_GLAD = ${EXTERNAL_GLAD}")

# Add Apple libraries on macOS
if(APPLE)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  find_library(IOKIT_LIBRARY IOKit)
  find_library(COCOA_LIBRARY Cocoa)
  list(APPEND GLFW_LIBS_ALL ${COREFOUNDATION_LIBRARY} ${IOKIT_LIBRARY} ${COCOA_LIBRARY})
endif()

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
    list(APPEND GLFW_LIBS_ALL ${GLFW_LIBRARIES} ${EXTRA_LIBS})
else ()
    pkg_check_modules(GLFW3 REQUIRED glfw3)

    add_definitions(${GLFW3_CFLAGS})
    list(APPEND GLFW_LIBS_ALL ${GLFW3_LDFLAGS} ${EXTRA_LIBS})
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
    list(APPEND OPENGL_LOADER_LIBS ${GLAD_LIBRARIES})
else ()
    list(APPEND OPENGL_LOADER_LIBS ${OPENGL_opengl_LIBRARY})
endif ()

if (OSMESA_EXAMPLES)
    message("-- Adding: glkitty")
    add_executable(kitty_gears src/kitty_gears.c)
    target_link_libraries(kitty_gears ${OSMESA_LIBS_ALL})
endif (OSMESA_EXAMPLES)

if (OPENGL_EXAMPLES)
    foreach(prog IN ITEMS gl1_gears gl2_gears gl3_gears gl4_gears)
        message("-- Adding: ${prog}")
        add_executable(${prog} src/${prog}.c)
        target_link_libraries(${prog} ${GLFW_LIBS_ALL} ${OPENGL_LOADER_LIBS})
        if (EXTERNAL_GLAD)
            target_compile_definitions(${prog} PRIVATE -DHAVE_GLAD)
        endif ()
        if (EXTERNAL_GLFW)
            add_dependencies(${prog} GLFW-build)
        endif ()
        if (EXTERNAL_GLAD)
            add_dependencies(${prog} GLAD-build)
        endif ()
    endforeach(prog)
endif (OPENGL_EXAMPLES)

# ▗▄     ▗▄           ▐███      ▐█
# ▝█▌    ▟█           ▝▀▜█      ▐█
#  █▌    █▌             ▐█      ▐█
#  ▐█   ▗█▘  █▌   ▐▌    ▐█      ▐█   ▟▛   ▄███▄    █▌▄██▄
#  ▐█   ▐█   █▌   ▐▌    ▐█      ▐█  ▟▛   ▐█▀▀▀█▙   █▙▛▀▜█▖
#   █▌  ▟▛   █▌   ▐▌    ▐█      ▐█ ▟▛    ▝    ▝█   █▛   █▌
#   ▜▙  █▌   █▌   ▐▌    ▐█      ▐█▟█       ▄▄▄▄█▖  █▌   ▜▌
#   ▐█ ▐█    █▌   ▐▌    ▐█      ▐█▛█▙    ▗██████▌  █▌   ▐▌
#   ▝█▖▐▛    █▌   █▌    ▐█      ▐█ ▝█▖   █▛▘   █▌  █▌   ▐▌
#    █▌█▌    █▙  ▗█▌    ▐█      ▐█  ▜█▖  █▌   ▐█▌  █▌   ▐▌
#    ▐██▘    ▐█▄▄█▜▌     █▙▄▄   ▐█   ▜▙  ▜█▄▄▟▛█▌  █▌   ▐▌
#    ▝██      ▜██▘▐▌     ▝███   ▐█   ▝█▌  ▜██▀ █▌  █▌   ▐▌

set (VULKAN_SDK "/opt/vulkan/1.2.154.0/x86_64")
find_program (GLSL_LANG_VALIDATOR NAMES glslangValidator PATHS ${VULKAN_SDK}/bin)
set (SHADER_DIR shaders)

if(NOT VULKAN_FOUND)
    message("Vulkan library not found")
    set(VULKAN_EXAMPLES 0)
elseif(NOT GLSL_LANG_VALIDATOR)
    message("glslangValidator not found")
    set(VULKAN_EXAMPLES 0)
endif()

if (VULKAN_EXAMPLES)
    file(GLOB_RECURSE VERTEX_SHADER_SOURCE "${SHADER_DIR}/*.vert")
    file(GLOB_RECURSE FRAGMENT_SHADER_SOURCE "${SHADER_DIR}/*.frag")
    set(SHADER_SOURCE ${VERTEX_SHADER_SOURCE} ${FRAGMENT_SHADER_SOURCE})
    source_group("shaders" FILES ${SHADER_SOURCE})

    foreach(shader ${SHADER_SOURCE})
        add_custom_command(OUTPUT ${shader}.spv DEPENDS ${shader}
            COMMAND ${GLSL_LANG_VALIDATOR} -V ${shader} -o ${shader}.spv)
        list(APPEND SHADERS ${shader}.spv)
    endforeach(shader)

    message("-- Adding: vk1_gears")
    add_executable(vk1_gears src/vk1_gears.c ${SHADERS})
    target_link_libraries(vk1_gears vulkan ${GLFW_LIBS_ALL})

    if (EXTERNAL_GLFW)
        add_dependencies(vk1_gears GLFW-build)
    endif ()
endif (VULKAN_EXAMPLES)
