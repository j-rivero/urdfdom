# Migration Tests CMake Configuration
# This file defines tests that validate the Rust migration is working correctly

cmake_minimum_required(VERSION 3.5)

# Find required packages
find_package(urdfdom_headers REQUIRED)

# Set up Rust library path and include directories
set(URDF_RUST_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../urdf_rust/include")
set(URDF_RUST_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../urdf_rust/target/debug")
set(URDF_RUST_LIBRARY "${URDF_RUST_LIB_DIR}/liburdf_rust.so")

# Include directories
include_directories(${urdfdom_headers_INCLUDE_DIRS})
include_directories(${URDF_RUST_INCLUDE_DIR})

# Set compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Function to create a migration test executable
function(add_migration_test test_name source_file)
    add_executable(${test_name} ${source_file})
    
    # Link against required libraries
    target_link_libraries(${test_name}
        ${urdfdom_LIBRARIES}
        ${URDF_RUST_LIBRARY}
    )
    
    # Set runtime library path
    set_target_properties(${test_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        INSTALL_RPATH "${URDF_RUST_LIB_DIR}"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    
    # Add as a test
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # Set environment for the test
    set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "LD_LIBRARY_PATH=${URDF_RUST_LIB_DIR}:${urdfdom_LIBRARY_DIRS}:$ENV{LD_LIBRARY_PATH}"
    )
endfunction()

# Build Rust library first (if not already built)
add_custom_target(build_rust_lib
    COMMAND cargo build
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../urdf_rust"
    COMMENT "Building Rust URDF library"
)

# Add migration tests
add_migration_test(pose_migration_test pose_migration_test.cpp)
add_migration_test(vector3_migration_test vector3_migration_test.cpp)
add_migration_test(geometry_migration_test geometry_migration_test.cpp)
add_migration_test(test_rust_integration test_rust_integration.cpp)
add_migration_test(test_urdf_parsing test_urdf_parsing.cpp)

# Make all tests depend on the Rust library being built
add_dependencies(test_rust_integration build_rust_lib)
add_dependencies(test_urdf_parsing build_rust_lib)
add_dependencies(vector3_migration_test build_rust_lib)
add_dependencies(pose_migration_test build_rust_lib)

# Add a target to run all migration tests
add_custom_target(run_migration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_rust_integration test_urdf_parsing vector3_migration_test pose_migration_test
    COMMENT "Running all migration tests"
)

# Display information about migration tests
message(STATUS "Migration tests configured:")
message(STATUS "  - test_rust_integration: Basic FFI integration test")
message(STATUS "  - test_urdf_parsing: URDF parsing validation test")
message(STATUS "  - vector3_migration_test: Vector3 parsing migration test")
message(STATUS "  - pose_migration_test: Pose data structures migration test")
message(STATUS "")
message(STATUS "To build and run migration tests:")
message(STATUS "  make run_migration_tests")
message(STATUS "")
message(STATUS "To run individual tests:")
message(STATUS "  ctest -R <test_name>")