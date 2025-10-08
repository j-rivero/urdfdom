//! Vector3 Migration Test - Before/After Comparison
//! 
//! This file demonstrates the migration from C++ Vector3::init() to Rust urdf_parse_vector3()
//! showing identical behavior and error handling.

#include <iostream>
#include <stdexcept>
#include <string>

// Original C++ headers
#include <urdf_model/pose.h>

// New Rust FFI header
extern "C" {
    int urdf_parse_vector3(const char* input, double* x, double* y, double* z);
}

void test_vector3_migration() {
    std::cout << "=== Vector3 Parsing Migration Test ===" << std::endl;
    std::cout << "Comparing C++ urdf::Vector3::init() vs Rust urdf_parse_vector3()" << std::endl;
    std::cout << std::endl;

    // Test cases that should succeed
    const char* valid_tests[] = {
        "1.0 2.0 3.0",
        "0.1 0.2 0.3", 
        "-1.5 2.7 -3.14",
        "1e-3 2.5e2 -1.23e+1",
        "  1.5   -2.7   3.14  ",  // Extra whitespace
        "1.0\t2.0\n3.0"           // Tabs and newlines
    };

    // Test cases that should fail
    const char* invalid_tests[] = {
        "1.0 foo 3.0",           // Invalid number
        "1.0 2.10.110 3.0",      // Invalid number format
        "1.0 2.0",               // Not enough numbers
        "1.0 2.0 3.0 4.0",       // Too many numbers
        "",                      // Empty string
        "   \t\n  "              // Only whitespace
    };

    std::cout << "--- VALID INPUT TESTS ---" << std::endl;
    
    for (size_t i = 0; i < sizeof(valid_tests)/sizeof(valid_tests[0]); ++i) {
        const char* test_input = valid_tests[i];
        std::cout << "Testing: \"" << test_input << "\"" << std::endl;
        
        // === BEFORE: C++ VERSION ===
        std::cout << "  C++ Version: ";
        try {
            urdf::Vector3 vec_cpp;
            vec_cpp.init(test_input);
            std::cout << "SUCCESS (" << vec_cpp.x << ", " << vec_cpp.y << ", " << vec_cpp.z << ")" << std::endl;
            
            // === AFTER: RUST VERSION ===
            std::cout << "  Rust Version: ";
            double x, y, z;
            if (urdf_parse_vector3(test_input, &x, &y, &z)) {
                std::cout << "SUCCESS (" << x << ", " << y << ", " << z << ")";
                
                // Verify identical results
                if (vec_cpp.x == x && vec_cpp.y == y && vec_cpp.z == z) {
                    std::cout << " ✅ IDENTICAL" << std::endl;
                } else {
                    std::cout << " ❌ DIFFERENT!" << std::endl;
                }
            } else {
                std::cout << "FAILED ❌ (Rust failed but C++ succeeded)" << std::endl;
            }
            
        } catch (const urdf::ParseError& e) {
            std::cout << "FAILED: " << e.what() << std::endl;
            
            // Test Rust version too
            std::cout << "  Rust Version: ";
            double x, y, z;
            if (urdf_parse_vector3(test_input, &x, &y, &z)) {
                std::cout << "SUCCESS ❌ (Rust succeeded but C++ failed)" << std::endl;
            } else {
                std::cout << "FAILED ✅ CONSISTENT" << std::endl;
            }
        }
        std::cout << std::endl;
    }

    std::cout << "--- INVALID INPUT TESTS ---" << std::endl;
    
    for (size_t i = 0; i < sizeof(invalid_tests)/sizeof(invalid_tests[0]); ++i) {
        const char* test_input = invalid_tests[i];
        std::cout << "Testing: \"" << test_input << "\"" << std::endl;
        
        // === BEFORE: C++ VERSION ===
        std::cout << "  C++ Version: ";
        bool cpp_failed = false;
        try {
            urdf::Vector3 vec_cpp;
            vec_cpp.init(test_input);
            std::cout << "SUCCESS ❌ (should have failed)" << std::endl;
        } catch (const urdf::ParseError& e) {
            std::cout << "FAILED ✅ (expected)" << std::endl;
            cpp_failed = true;
        }
        
        // === AFTER: RUST VERSION ===
        std::cout << "  Rust Version: ";
        double x, y, z;
        if (urdf_parse_vector3(test_input, &x, &y, &z)) {
            std::cout << "SUCCESS ❌ (should have failed)";
            if (cpp_failed) {
                std::cout << " - INCONSISTENT!" << std::endl;
            } else {
                std::cout << " - BOTH SUCCEEDED" << std::endl;
            }
        } else {
            std::cout << "FAILED ✅ (expected)";
            if (cpp_failed) {
                std::cout << " - CONSISTENT" << std::endl;
            } else {
                std::cout << " - INCONSISTENT!" << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

// Example of how to migrate existing C++ code
void migration_example() {
    std::cout << "=== MIGRATION EXAMPLE ===" << std::endl;
    std::cout << std::endl;
    
    std::cout << "// BEFORE (C++ version):" << std::endl;
    std::cout << "urdf::Vector3 vec;" << std::endl;
    std::cout << "try {" << std::endl;
    std::cout << "    vec.init(\"1.0 2.0 3.0\");" << std::endl;
    std::cout << "    // Use vec.x, vec.y, vec.z" << std::endl;
    std::cout << "} catch(urdf::ParseError &e) {" << std::endl;
    std::cout << "    // Handle error" << std::endl;
    std::cout << "}" << std::endl;
    std::cout << std::endl;
    
    std::cout << "// AFTER (Rust FFI version):" << std::endl;
    std::cout << "double x, y, z;" << std::endl;
    std::cout << "if (urdf_parse_vector3(\"1.0 2.0 3.0\", &x, &y, &z)) {" << std::endl;
    std::cout << "    // Use x, y, z" << std::endl;
    std::cout << "} else {" << std::endl;
    std::cout << "    // Handle error" << std::endl;
    std::cout << "}" << std::endl;
    std::cout << std::endl;
    
    // Demonstrate actual migration
    std::cout << "Actual execution:" << std::endl;
    
    // C++ version
    try {
        urdf::Vector3 vec;
        vec.init("1.0 2.0 3.0");
        std::cout << "C++: (" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;
    } catch (const urdf::ParseError& e) {
        std::cout << "C++ failed: " << e.what() << std::endl;
    }
    
    // Rust version  
    double x, y, z;
    if (urdf_parse_vector3("1.0 2.0 3.0", &x, &y, &z)) {
        std::cout << "Rust: (" << x << ", " << y << ", " << z << ")" << std::endl;
    } else {
        std::cout << "Rust failed" << std::endl;
    }
}

int main() {
    test_vector3_migration();
    std::cout << std::endl;
    migration_example();
    
    return 0;
}