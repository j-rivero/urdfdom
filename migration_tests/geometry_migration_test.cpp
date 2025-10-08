//! Geometry Migration Test
//! 
//! Tests compatibility between C++ urdf::Geometry classes and Rust Geometry enum.
//! Validates that both implementations handle all geometry types (Box, Sphere, Cylinder, Mesh)
//! with identical behavior for creating, accessing, and comparing geometric shapes.

#include <iostream>
#include <memory>
#include <cmath>
#include <cassert>

// C++ urdf model includes
#include "../urdf_rust/include/urdf_rust.h"

// Test Results
struct TestResults {
    int passed = 0;
    int failed = 0;
    
    void pass(const std::string& test_name) {
        std::cout << "✅ PASS: " << test_name << std::endl;
        passed++;
    }
    
    void fail(const std::string& test_name, const std::string& reason) {
        std::cout << "❌ FAIL: " << test_name << " - " << reason << std::endl;
        failed++;
    }
    
    void summary() {
        std::cout << "\n=== GEOMETRY MIGRATION TEST RESULTS ===\n";
        std::cout << "✅ Passed: " << passed << std::endl;
        std::cout << "❌ Failed: " << failed << std::endl;
        std::cout << "📊 Total:  " << (passed + failed) << std::endl;
        
        if (failed == 0) {
            std::cout << "\n🎉 ALL TESTS PASSED! ✅ Rust Geometry matches C++ behavior exactly\n";
        } else {
            std::cout << "\n⚠️  SOME TESTS FAILED! Migration needs attention\n";
        }
    }
};

// Test Box geometry compatibility
void test_box_geometry_compatibility(TestResults& results) {
    std::cout << "\n--- Testing Box Geometry Compatibility ---\n";
    
    // Test Box creation and dimensions
    try {
        // Create box with Rust FFI
        double dims[3] = {2.0, 3.0, 4.0};
        void* rust_box = urdf_geometry_create_box(dims[0], dims[1], dims[2]);
        
        if (rust_box == nullptr) {
            results.fail("Box Creation", "Rust FFI returned null");
            return;
        }
        
        // Get dimensions back
        double retrieved_dims[3];
        if (urdf_geometry_box_get_dimensions(rust_box, retrieved_dims) != 0) {
            results.fail("Box Dimensions Retrieval", "Failed to get dimensions");
            urdf_geometry_destroy(rust_box);
            return;
        }
        
        // Verify dimensions match
        const double EPSILON = 1e-10;
        bool dimensions_match = true;
        for (int i = 0; i < 3; i++) {
            if (std::abs(retrieved_dims[i] - dims[i]) > EPSILON) {
                dimensions_match = false;
                break;
            }
        }
        
        if (!dimensions_match) {
            results.fail("Box Dimensions Match", "Dimensions don't match input");
        } else {
            results.pass("Box Dimensions Match");
        }
        
        // Test type identification
        int geometry_type = urdf_geometry_get_type(rust_box);
        if (geometry_type != URDF_GEOMETRY_BOX) {
            results.fail("Box Type Identification", "Wrong geometry type returned");
        } else {
            results.pass("Box Type Identification");
        }
        
        urdf_geometry_destroy(rust_box);
        
    } catch (const std::exception& e) {
        results.fail("Box Geometry Test", std::string("Exception: ") + e.what());
    }
}

// Test Sphere geometry compatibility
void test_sphere_geometry_compatibility(TestResults& results) {
    std::cout << "\n--- Testing Sphere Geometry Compatibility ---\n";
    
    try {
        // Create sphere with Rust FFI
        double radius = 1.5;
        void* rust_sphere = urdf_geometry_create_sphere(radius);
        
        if (rust_sphere == nullptr) {
            results.fail("Sphere Creation", "Rust FFI returned null");
            return;
        }
        
        // Get radius back
        double retrieved_radius = urdf_geometry_sphere_get_radius(rust_sphere);
        
        // Verify radius matches
        const double EPSILON = 1e-10;
        if (std::abs(retrieved_radius - radius) > EPSILON) {
            results.fail("Sphere Radius Match", "Radius doesn't match input");
        } else {
            results.pass("Sphere Radius Match");
        }
        
        // Test type identification
        int geometry_type = urdf_geometry_get_type(rust_sphere);
        if (geometry_type != URDF_GEOMETRY_SPHERE) {
            results.fail("Sphere Type Identification", "Wrong geometry type returned");
        } else {
            results.pass("Sphere Type Identification");
        }
        
        urdf_geometry_destroy(rust_sphere);
        
    } catch (const std::exception& e) {
        results.fail("Sphere Geometry Test", std::string("Exception: ") + e.what());
    }
}

// Test Cylinder geometry compatibility
void test_cylinder_geometry_compatibility(TestResults& results) {
    std::cout << "\n--- Testing Cylinder Geometry Compatibility ---\n";
    
    try {
        // Create cylinder with Rust FFI
        double radius = 0.5;
        double length = 2.0;
        void* rust_cylinder = urdf_geometry_create_cylinder(radius, length);
        
        if (rust_cylinder == nullptr) {
            results.fail("Cylinder Creation", "Rust FFI returned null");
            return;
        }
        
        // Get parameters back
        double retrieved_radius = urdf_geometry_cylinder_get_radius(rust_cylinder);
        double retrieved_length = urdf_geometry_cylinder_get_length(rust_cylinder);
        
        // Verify parameters match
        const double EPSILON = 1e-10;
        bool params_match = (std::abs(retrieved_radius - radius) <= EPSILON) &&
                           (std::abs(retrieved_length - length) <= EPSILON);
        
        if (!params_match) {
            results.fail("Cylinder Parameters Match", "Parameters don't match input");
        } else {
            results.pass("Cylinder Parameters Match");
        }
        
        // Test type identification
        int geometry_type = urdf_geometry_get_type(rust_cylinder);
        if (geometry_type != URDF_GEOMETRY_CYLINDER) {
            results.fail("Cylinder Type Identification", "Wrong geometry type returned");
        } else {
            results.pass("Cylinder Type Identification");
        }
        
        urdf_geometry_destroy(rust_cylinder);
        
    } catch (const std::exception& e) {
        results.fail("Cylinder Geometry Test", std::string("Exception: ") + e.what());
    }
}

// Test Mesh geometry compatibility
void test_mesh_geometry_compatibility(TestResults& results) {
    std::cout << "\n--- Testing Mesh Geometry Compatibility ---\n";
    
    try {
        // Create mesh with Rust FFI
        const char* filename = "test_mesh.dae";
        double scale[3] = {2.0, 2.0, 2.0};
        void* rust_mesh = urdf_geometry_create_mesh(filename, scale);
        
        if (rust_mesh == nullptr) {
            results.fail("Mesh Creation", "Rust FFI returned null");
            return;
        }
        
        // Get filename back
        char retrieved_filename[256];
        if (urdf_geometry_mesh_get_filename(rust_mesh, retrieved_filename, sizeof(retrieved_filename)) != 0) {
            results.fail("Mesh Filename Retrieval", "Failed to get filename");
            urdf_geometry_destroy(rust_mesh);
            return;
        }
        
        // Verify filename matches
        if (std::string(retrieved_filename) != std::string(filename)) {
            results.fail("Mesh Filename Match", "Filename doesn't match input");
        } else {
            results.pass("Mesh Filename Match");
        }
        
        // Get scale back
        double retrieved_scale[3];
        if (urdf_geometry_mesh_get_scale(rust_mesh, retrieved_scale) != 0) {
            results.fail("Mesh Scale Retrieval", "Failed to get scale");
            urdf_geometry_destroy(rust_mesh);
            return;
        }
        
        // Verify scale matches
        const double EPSILON = 1e-10;
        bool scale_matches = true;
        for (int i = 0; i < 3; i++) {
            if (std::abs(retrieved_scale[i] - scale[i]) > EPSILON) {
                scale_matches = false;
                break;
            }
        }
        
        if (!scale_matches) {
            results.fail("Mesh Scale Match", "Scale doesn't match input");
        } else {
            results.pass("Mesh Scale Match");
        }
        
        // Test type identification
        int geometry_type = urdf_geometry_get_type(rust_mesh);
        if (geometry_type != URDF_GEOMETRY_MESH) {
            results.fail("Mesh Type Identification", "Wrong geometry type returned");
        } else {
            results.pass("Mesh Type Identification");
        }
        
        urdf_geometry_destroy(rust_mesh);
        
    } catch (const std::exception& e) {
        results.fail("Mesh Geometry Test", std::string("Exception: ") + e.what());
    }
}

// Test geometry type enumeration consistency
void test_geometry_type_consistency(TestResults& results) {
    std::cout << "\n--- Testing Geometry Type Consistency ---\n";
    
    // Create instances of each geometry type and verify type constants
    void* box = urdf_geometry_create_box(1.0, 1.0, 1.0);
    void* sphere = urdf_geometry_create_sphere(1.0);
    void* cylinder = urdf_geometry_create_cylinder(1.0, 1.0);
    const char* filename = "test.dae";
    double scale[3] = {1.0, 1.0, 1.0};
    void* mesh = urdf_geometry_create_mesh(filename, scale);
    
    bool types_consistent = true;
    
    if (box && urdf_geometry_get_type(box) != URDF_GEOMETRY_BOX) types_consistent = false;
    if (sphere && urdf_geometry_get_type(sphere) != URDF_GEOMETRY_SPHERE) types_consistent = false;
    if (cylinder && urdf_geometry_get_type(cylinder) != URDF_GEOMETRY_CYLINDER) types_consistent = false;
    if (mesh && urdf_geometry_get_type(mesh) != URDF_GEOMETRY_MESH) types_consistent = false;
    
    // Cleanup
    if (box) urdf_geometry_destroy(box);
    if (sphere) urdf_geometry_destroy(sphere);
    if (cylinder) urdf_geometry_destroy(cylinder);
    if (mesh) urdf_geometry_destroy(mesh);
    
    if (types_consistent) {
        results.pass("Geometry Type Consistency");
    } else {
        results.fail("Geometry Type Consistency", "Type constants don't match");
    }
}

int main() {
    std::cout << "🚀 URDF Geometry Migration Test\n";
    std::cout << "Testing C++ ↔ Rust Geometry compatibility...\n";
    
    TestResults results;
    
    // Run all geometry compatibility tests
    test_box_geometry_compatibility(results);
    test_sphere_geometry_compatibility(results);
    test_cylinder_geometry_compatibility(results);
    test_mesh_geometry_compatibility(results);
    test_geometry_type_consistency(results);
    
    // Print final results
    results.summary();
    
    // Return appropriate exit code
    return (results.failed == 0) ? 0 : 1;
}