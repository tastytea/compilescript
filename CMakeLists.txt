# Support version 3.6 and above, but use policy settings up to 3.14.
# 3.6 is needed because of IMPORTED_TARGET in pkg_check_modules().
cmake_minimum_required(VERSION 3.6...3.14)
# Ranges are supported from 3.12, set policy to current for < 3.12.
if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(compilescript
  VERSION 0.4.1
  LANGUAGES CXX)
# DESCRIPTION was introduced in version 3.9.
if(NOT (${CMAKE_VERSION} VERSION_LESS 3.9))
  set(PROJECT_DESCRIPTION
    "Execute source files from compiled languages as scripts.")
endif()

option(WITH_MAN "Compile and install manpage." YES)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(DEBUG_CXXFLAGS
  "-Wall"
  "-Wextra"
  "-Wpedantic"
  "-ftrapv"
  "-fsanitize=undefined"
  "-g"
  "-Og"
  "-fno-omit-frame-pointer")
set(DEBUG_LDFLAGS
  "-fsanitize=undefined")
add_compile_options("$<$<CONFIG:Debug>:${DEBUG_CXXFLAGS}>")
# add_link_options was introduced in version 3.13.
if(${CMAKE_VERSION} VERSION_LESS 3.13)
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${DEBUG_LDFLAGS}")
else()
  add_link_options("$<$<CONFIG:Debug>:${DEBUG_LDFLAGS}>")
endif()

add_subdirectory(src)

if (WITH_MAN)
  add_subdirectory(man)
endif()
