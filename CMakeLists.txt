cmake_minimum_required (VERSION 3.2)
project(compilescript
        VERSION 0.3.3
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
    "${CMAKE_CXX_FLAGS_DEBUG} -Wpedantic -Wall -Wextra -g -Og -fno-omit-frame-pointer")

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

add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.1"
                   WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
                   DEPENDS "${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.1.adoc"
                   COMMAND ${CMAKE_SOURCE_DIR}/build_manpage.sh
                   ARGS ${PROJECT_VERSION})
add_custom_target(run ALL
                  DEPENDS "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.1")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.1
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
