cmake_minimum_required( VERSION 3.5 FATAL_ERROR )
project (urdfdom CXX C)

set (URDF_MAJOR_VERSION 5)
set (URDF_MINOR_VERSION 0)
set (URDF_PATCH_VERSION 2)

set (URDF_VERSION ${URDF_MAJOR_VERSION}.${URDF_MINOR_VERSION}.${URDF_PATCH_VERSION})
set (URDF_MAJOR_MINOR_VERSION ${URDF_MAJOR_VERSION}.${URDF_MINOR_VERSION})

message (STATUS "${PROJECT_NAME} version ${URDF_VERSION}")

include(GNUInstallDirs)

option(APPEND_PROJECT_NAME_TO_INCLUDEDIR
  "When ON headers are installed to a folder ending with an extra ${PROJECT_NAME}. \
  This avoids include directory search order issues when overriding this package
  from a merged catkin, ament, or colcon workspace." ON)

if(APPEND_PROJECT_NAME_TO_INCLUDEDIR)
  set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
endif()

# set the default build type
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# If compiler support symbol visibility, enable it.
include(CheckCCompilerFlag)
check_c_compiler_flag(-fvisibility=hidden HAS_VISIBILITY)
if (HAS_VISIBILITY)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Code coverage support
option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE)
  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-g -O0 --coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
    message(STATUS "Code coverage enabled")
  else()
    message(WARNING "Code coverage is only supported with GCC or Clang")
  endif()
endif()

# This shouldn't be necessary, but there has been trouble
# with MSVC being set off, but MSVCXX ON.
if(MSVC OR MSVC90 OR MSVC10)
  set(MSVC ON)
endif (MSVC OR MSVC90 OR MSVC10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(tinyxml2_vendor QUIET)
find_package(TinyXML2 REQUIRED)

find_package(urdfdom_headers REQUIRED)
find_package(console_bridge_vendor QUIET) # Provides console_bridge 0.4.0 on platforms without it.
find_package(console_bridge REQUIRED)

# Control where libraries and executables are placed during the build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

enable_testing()

set(CMAKE_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}/cmake)
string(REGEX REPLACE "[^/]+" ".." RELATIVE_PATH_CMAKE_DIR_TO_PREFIX "${CMAKE_CONFIG_INSTALL_DIR}")
string(REGEX REPLACE "[^/]+" ".." RELATIVE_PATH_LIBDIR_TO_PREFIX "${CMAKE_INSTALL_LIBDIR}")

# Rust integration for gradual migration
option(BUILD_RUST_BINDINGS "Build Rust bindings alongside C++ library" ON)

if(BUILD_RUST_BINDINGS)
  # Check for Rust toolchain
  find_program(RUSTC_EXECUTABLE rustc)
  find_program(CARGO_EXECUTABLE cargo)
  
  if(RUSTC_EXECUTABLE AND CARGO_EXECUTABLE)
    message(STATUS "Found Rust toolchain: ${RUSTC_EXECUTABLE}")
    
    # Get Rust version for validation
    execute_process(
      COMMAND ${RUSTC_EXECUTABLE} --version
      OUTPUT_VARIABLE RUST_VERSION_OUTPUT
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Rust version: ${RUST_VERSION_OUTPUT}")
    
    # Check if we have the urdf_rust crate
    set(RUST_CRATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/urdf_rust")
    if(EXISTS "${RUST_CRATE_DIR}/Cargo.toml")
      message(STATUS "Found urdf_rust crate at: ${RUST_CRATE_DIR}")
      
      # Set up Rust build configuration
      if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CARGO_BUILD_TYPE "debug")
        set(RUST_BUILD_FLAG "")
      else()
        set(CARGO_BUILD_TYPE "release")
        set(RUST_BUILD_FLAG "--release")
      endif()
      
      # Set target directory to avoid conflicts
      set(RUST_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/rust_target")
      
      # Configure Rust build with CMake integration
      set(RUST_CMAKE_PREFIX "${CMAKE_CURRENT_BINARY_DIR}")
      set(RUST_CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
      set(RUST_CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
      set(RUST_CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
      set(RUST_CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
      set(RUST_CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
      
      # Custom target to build Rust library
      add_custom_target(urdf_rust_build
        COMMAND ${CMAKE_COMMAND} -E env
          "CMAKE_PREFIX_PATH=${RUST_CMAKE_PREFIX}"
          "CMAKE_LIBRARY_OUTPUT_DIRECTORY=${RUST_CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
          "CMAKE_ARCHIVE_OUTPUT_DIRECTORY=${RUST_CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
          "CMAKE_INSTALL_PREFIX=${RUST_CMAKE_INSTALL_PREFIX}"
          "CMAKE_INSTALL_LIBDIR=${RUST_CMAKE_INSTALL_LIBDIR}"
          "CMAKE_INSTALL_INCLUDEDIR=${RUST_CMAKE_INSTALL_INCLUDEDIR}"
          ${CARGO_EXECUTABLE} build ${RUST_BUILD_FLAG}
          --target-dir "${RUST_TARGET_DIR}"
        WORKING_DIRECTORY "${RUST_CRATE_DIR}"
        COMMENT "Building urdf_rust library"
        VERBATIM
      )
      
      # Define library paths
      set(RUST_LIB_NAME "urdf_rust")
      if(WIN32)
        set(RUST_LIB_FILE "${RUST_TARGET_DIR}/${CARGO_BUILD_TYPE}/${RUST_LIB_NAME}.lib")
      else()
        set(RUST_LIB_FILE "${RUST_TARGET_DIR}/${CARGO_BUILD_TYPE}/lib${RUST_LIB_NAME}.a")
      endif()
      
      # Create imported library target
      add_library(urdf_rust STATIC IMPORTED GLOBAL)
      set_target_properties(urdf_rust PROPERTIES
        IMPORTED_LOCATION "${RUST_LIB_FILE}"
      )
      
      # Ensure Rust library is built before anything that depends on it
      add_dependencies(urdf_rust urdf_rust_build)
      
      # Check if cbindgen generated headers exist
      set(RUST_HEADER_FILE "${RUST_CRATE_DIR}/include/urdf_rust.h")
      
      # Custom target to generate C headers
      add_custom_target(urdf_rust_headers
        COMMAND ${CMAKE_COMMAND} -E env
          ${CARGO_EXECUTABLE} build ${RUST_BUILD_FLAG}
          --target-dir "${RUST_TARGET_DIR}"
        WORKING_DIRECTORY "${RUST_CRATE_DIR}"
        COMMENT "Generating C headers for urdf_rust"
        VERBATIM
      )
      
      # Install Rust library and headers
      install(FILES "${RUST_LIB_FILE}"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT libraries
        OPTIONAL  # Don't fail if file doesn't exist during configure
      )
      
      if(EXISTS "${RUST_HEADER_FILE}")
        install(FILES "${RUST_HEADER_FILE}"
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
          COMPONENT headers
        )
      endif()
      
      # Export Rust library for downstream projects
      set(PKG_RUST_LIBRARIES "urdf_rust")
      
      # Include Rust headers in build
      if(EXISTS "${RUST_HEADER_FILE}")
        include_directories("${RUST_CRATE_DIR}/include")
      endif()
      
      message(STATUS "Rust integration configured successfully")
      message(STATUS "  Build target: urdf_rust_build")
      message(STATUS "  Library: ${RUST_LIB_FILE}")
      message(STATUS "  Headers: ${RUST_HEADER_FILE}")
      
    else()
      message(WARNING "urdf_rust crate not found at ${RUST_CRATE_DIR}")
      message(WARNING "Rust bindings will not be built")
      set(BUILD_RUST_BINDINGS OFF)
    endif()
    
  else()
    message(WARNING "Rust toolchain not found (rustc: ${RUSTC_EXECUTABLE}, cargo: ${CARGO_EXECUTABLE})")
    message(WARNING "Rust bindings will not be built")
    message(STATUS "To install Rust: curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh")
    set(BUILD_RUST_BINDINGS OFF)
  endif()
else()
  message(STATUS "Rust bindings disabled (BUILD_RUST_BINDINGS=OFF)")
endif()

add_subdirectory(urdf_parser)

set(PKG_NAME ${PROJECT_NAME})
set(PKG_LIBRARIES urdfdom_sensor urdfdom_model urdfdom_world)
if(BUILD_RUST_BINDINGS AND PKG_RUST_LIBRARIES)
  list(APPEND PKG_LIBRARIES ${PKG_RUST_LIBRARIES})
endif()
set(PKG_DEPENDS urdfdom_headers)
set(PKG_EXPORTS urdfdom)
set(cmake_conf_file "cmake/urdfdom-config")
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${cmake_conf_file}Version.cmake"
  VERSION ${URDF_VERSION}
  COMPATIBILITY SameMajorVersion
)
configure_package_config_file("${cmake_conf_file}.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${cmake_conf_file}.cmake"
  INSTALL_DESTINATION ${CMAKE_CONFIG_INSTALL_DIR}
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${cmake_conf_file}.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${cmake_conf_file}Version.cmake"
  DESTINATION ${CMAKE_CONFIG_INSTALL_DIR}
)

install(FILES package.xml DESTINATION share/${PROJECT_NAME})

# Make the package config file
set(PKG_DESC "Unified Robot Description Format")
set(PKG_DEPENDS "urdfdom_headers") # make the list separated by spaces instead of ;
set(PKG_URDF_LIBS "-lurdfdom_sensor -lurdfdom_model -lurdfdom_world")
if(BUILD_RUST_BINDINGS AND PKG_RUST_LIBRARIES)
  set(PKG_URDF_LIBS "${PKG_URDF_LIBS} -lurdf_rust")
endif()
set(pkg_conf_file "cmake/pkgconfig/urdfdom.pc")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${pkg_conf_file}.in" "${CMAKE_BINARY_DIR}/${pkg_conf_file}" @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${pkg_conf_file}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig/ COMPONENT pkgconfig)


# Add uninstall target
# Ref: http://www.cmake.org/Wiki/CMake_FAQ#Can_I_do_.22make_uninstall.22_with_CMake.3F
configure_file(
  "${PROJECT_SOURCE_DIR}/cmake/uninstall.cmake.in"
  "${PROJECT_BINARY_DIR}/uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${PROJECT_BINARY_DIR}/uninstall.cmake")

# Coverage targets
if(ENABLE_COVERAGE)
  # Coverage reset target
  add_custom_target(coverage-reset
    COMMAND lcov --directory . --zerocounters
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Resetting coverage counters"
  )

  # Coverage data collection and HTML report generation
  add_custom_target(coverage
    COMMAND lcov --directory . --capture --output-file coverage.info --ignore-errors mismatch,gcov
    COMMAND lcov --remove coverage.info '/usr/*' '*/gtest/*' '*/test/*' --output-file coverage_filtered.info --ignore-errors mismatch,gcov
    COMMAND genhtml coverage_filtered.info --output-directory coverage_html --ignore-errors mismatch
    COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in coverage_html/"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating coverage report"
  )

  # Combined target to run tests and generate coverage
  add_custom_target(test-coverage
    COMMAND ${CMAKE_COMMAND} --build . --target coverage-reset
    COMMAND ${CMAKE_CTEST_COMMAND} -V
    COMMAND ${CMAKE_COMMAND} --build . --target coverage
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests and generating coverage report"
  )
endif()

# Performance testing info
message(STATUS "Performance testing targets available:")
message(STATUS "  make performance-test       - Run full URDF parser benchmark suite")
message(STATUS "  make performance-test-quick  - Run quick performance test")

message(STATUS "Configuration successful. Type make to compile urdfdom")

SET_DIRECTORY_PROPERTIES(PROPERTIES
  ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/cmake/urdfdom-config.cmake
  ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/cmake/pkgconfig/urdfdom.pc)
