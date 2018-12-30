cmake_minimum_required (VERSION 3.2)
project(compilescript
        VERSION 0.3.0
        LANGUAGES CXX
)

include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBXDG_BASEDIR REQUIRED libxdg-basedir)
pkg_check_modules(LIBCONFIG REQUIRED libconfig++)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pedantic -Wextra -g -Og -fno-omit-frame-pointer")

include_directories(${PROJECT_BINARY_DIR})
include_directories(${LIBXDG_BASEDIR_INCLUDE_DIRS})
include_directories(${LIBCONFIG_INCLUDE_DIRS})

link_directories(${LIBXDG_BASEDIR_LIBRARY_DIRS})
link_directories(${LIBCONFIG_LIBRARY_DIRS})

# Write version in header
configure_file(
    "${PROJECT_SOURCE_DIR}/src/version.hpp.in"
    "${PROJECT_BINARY_DIR}/version.hpp"
)

file(GLOB sources src/*.cpp)
add_executable(${CMAKE_PROJECT_NAME} "${sources}")
target_link_libraries(${CMAKE_PROJECT_NAME}
                      "${LIBXDG_BASEDIR_LDFLAGS} ${LIBCONFIG_LDFLAGS}"
                      "-lstdc++fs")
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
