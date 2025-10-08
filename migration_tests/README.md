# Migration Tests

This directory contains tests that validate the migration from C++ URDF implementation to Rust.

## Test Files

### Core Integration Tests
- **`test_rust_integration.cpp`** - Basic FFI integration test
  - Validates that Rust library can be called from C++
  - Tests basic function calls and return values

- **`test_urdf_parsing.cpp`** - URDF parsing functionality test  
  - Tests URDF XML parsing with Rust implementation
  - Validates robot model creation and querying

### Migration Validation Tests
- **`vector3_migration_test.cpp`** - Vector3 parsing migration validation
  - Compares C++ `urdf::Vector3::init()` vs Rust `urdf_parse_vector3()`
  - Tests identical behavior for valid and invalid inputs

- **`pose_migration_test.cpp`** - Pose data structures migration validation
  - Compares C++ `urdf::Vector3`, `urdf::Rotation`, `urdf::Pose` vs Rust equivalents
  - Tests mathematical operations, quaternion conversion, RPY angles
  - Validates memory layout compatibility and FFI integration

## Building and Running Tests

### Prerequisites
- CMake 3.5+
- C++11 compatible compiler
- Rust toolchain (rustc, cargo)
- urdfdom_headers package

### Build Configuration
The migration tests are automatically included when:
- `BUILD_TESTING=ON` (default)
- `BUILD_RUST_BINDINGS=ON` (default)

### Build Commands

```bash
# Build all tests
mkdir build && cd build
cmake ..
make

# Build and run all migration tests
make run_migration_tests

# Run specific test
ctest -R test_rust_integration
ctest -R pose_migration_test
```

### Manual Test Execution

```bash
# From build directory
./migration_tests/test_rust_integration
./migration_tests/pose_migration_test
./migration_tests/vector3_migration_test
./migration_tests/test_urdf_parsing
```

## Test Results

### Expected Output
All tests should show:
```
✅ ALL TESTS PASSED!
✅ [Specific functionality] is working correctly
```

### What Each Test Validates

#### `test_rust_integration`
- Basic FFI function calls work
- Rust library links correctly
- Expected return values (42)

#### `vector3_migration_test`
- String parsing: `"1.0 2.0 3.0"` → `Vector3(1.0, 2.0, 3.0)`
- Error handling for invalid inputs
- Identical behavior between C++ and Rust

#### `pose_migration_test`
- Vector3 operations and parsing
- Quaternion mathematics and RPY conversion
- Pose structure composition
- Mathematical operations (multiplication, rotation)
- Memory layout compatibility (`#[repr(C)]`)

#### `test_urdf_parsing`
- URDF XML parsing from strings
- Robot model creation and validation
- Link and joint counting
- Error handling for malformed URDF

## Integration with CI/CD

These tests are designed to be run in continuous integration to ensure:
1. Rust migration maintains C++ compatibility
2. No regression in functionality
3. Mathematical precision is preserved
4. FFI layer works correctly

## Troubleshooting

### Common Issues

**Library not found errors:**
```bash
# Set library path
export LD_LIBRARY_PATH=/path/to/urdf_rust/target/debug:$LD_LIBRARY_PATH
```

**Rust library not built:**
```bash
cd urdf_rust
cargo build
```

**CMake configuration issues:**
```bash
# Clean and reconfigure
rm -rf build
mkdir build && cd build
cmake .. -DBUILD_RUST_BINDINGS=ON -DBUILD_TESTING=ON
```

## Development Notes

- Tests use `#[repr(C)]` structs for memory layout compatibility
- Mathematical precision validated with 1e-5 tolerance
- All FFI functions return C-compatible error codes
- Tests are designed to be deterministic and repeatable

## Adding New Migration Tests

1. Create new `.cpp` file in this directory
2. Add to `CMakeLists.txt` using `add_migration_test()` function
3. Follow the pattern of existing tests for consistency
4. Update this README with test description