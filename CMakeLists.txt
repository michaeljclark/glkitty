#                :@@#  -@@*
#                :@@#  -@@*         .%@#
#                :@@#  -@@*         -@@@
#                :@@#  -@@*          #%+   +@@=    +@@=
#                :@@#  -@@*                +@@=    +@@=
#                :@@#  -@@*                +@@=    +@@=
#     +%%%+.*@@. :@@#  -@@*    %@@* :@@# %@@@@@@%%@@@@@@%*@@+     %@@
#   .%@@@@@%#@@. :@@#  -@@*   #@@#  :@@# %@@@@@@%%@@@@@@%:@@%    :@@%
#   #@@%*+*@@@@. :@@#  -@@*  *@@#   :@@# ::*@@+::::*@@+:: %@@:   +@@+
#  -@@%.   -@@@. :@@#  -@@* +@@%.   :@@#   +@@=    +@@=   +@@+   %@@:
#  *@@+     %@@. :@@#  -@@*=@@%:    :@@#   +@@=    +@@=   :@@%  .@@%
#  #@@:     %@@. :@@#  -@@#@@@-     :@@#   +@@=    +@@=    #@@: =@@=
#  %@@.     %@@. :@@#  -@@@@@@=     :@@#   +@@=    +@@=    =@@= #@@.
#  %@@.     %@@. :@@#  -@@@@@@%.    :@@#   +@@=    +@@=    .@@# @@#
#  #@@:     %@@. :@@#  -@@@++@@#    :@@#   +@@=    +@@=     #@@+@@=
#  +@@+     %@@. :@@#  -@@*  %@@+   :@@#   +@@=    +@@=     -@@@@@.
#  -@@%.   -@@@. :@@#  -@@*  :@@@:  :@@#   +@@=    +@@=      %@@@#
#   #@@%++*@@@@. :@@#  -@@*   +@@%  :@@#   =@@%++  =@@%++    *@@@=
#   .%@@@@@%%@@. :@@#  -@@*    #@@+ :@@#    %@@@%   %@@@%    -@@@
#     +%%%+.%@@. :@@#  -@@*    :@@@-:@@#    .*%%#   .*%%#    .@@*
#          .@@@                                              =@@-
#   .*     *@@*                                              %@%
#   %@%+==#@@@:                                           =+%@@+
#   =%@@@@@@%-                                            *@@@#
#    .=#%%#+.                                             +%%+

cmake_minimum_required (VERSION 3.12)
project(glkitty VERSION 0.0.1)

include(CheckLibraryExists)
check_library_exists(m sqrtf "" HAVE_LIB_M)
if (HAVE_LIB_M)
    set(EXTRA_LIBS ${EXTRA_LIBS} m)
endif (HAVE_LIB_M)

find_library(OPENGL_LIBRARY GL)

find_package(PkgConfig)
pkg_check_modules(GLFW3 REQUIRED glfw3)
pkg_check_modules(OSMESA REQUIRED osmesa)
pkg_check_modules(ZLIB REQUIRED zlib)

add_definitions(${GLFW3_CFLAGS})
add_definitions(${OSMESA_CFLAGS})

set(OSMESA_LIBS_ALL ${OSMESA_LDFLAGS} ${EXTRA_LIBS})
set(GLFW_LIBS_ALL ${GLFW3_LDFLAGS} ${OPENGL_LIBRARY} ${EXTRA_LIBS})

if (ZLIB_FOUND)
	list(APPEND OSMESA_LIBS_ALL ${ZLIB_LDFLAGS})
	add_definitions(-DHAVE_ZLIB=1)
endif (ZLIB_FOUND)

if (OSMESA_FOUND)
	add_executable(kitty_gears src/kitty_gears.c)
	target_link_libraries(kitty_gears ${OSMESA_LIBS_ALL})
endif (OSMESA_FOUND)

if (GLFW3_FOUND)
	add_executable(gl1_gears src/gl1_gears.c)
	target_link_libraries(gl1_gears ${GLFW_LIBS_ALL})

	add_executable(gl2_gears src/gl2_gears.c)
	target_link_libraries(gl2_gears ${GLFW_LIBS_ALL})
endif (GLFW3_FOUND)
