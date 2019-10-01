include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libxdg-basedir REQUIRED IMPORTED_TARGET libxdg-basedir)
pkg_check_modules(libconfig++ REQUIRED IMPORTED_TARGET libconfig++)

# Write version in header
configure_file("version.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version.hpp")

set(sources "main.cpp" "../xdgcfg/src/xdgcfg.cpp")

add_executable(${CMAKE_PROJECT_NAME} "${sources}")

target_include_directories(${PROJECT_NAME}
  PRIVATE
  "${PROJECT_SOURCE_DIR}/xdgcfg/include"
  "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(${CMAKE_PROJECT_NAME}
  PRIVATE PkgConfig::libxdg-basedir PkgConfig::libconfig++ stdc++fs)

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}")
