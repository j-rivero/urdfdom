//! Simple test program to validate Rust FFI integration
//! 
//! This program tests that the Rust urdf_rust library can be called
//! from C++ code successfully.

#include <iostream>
#include <cstdio>

// Test if we can include the generated header
#include "urdf_rust.h"

// Declare the test function in case header generation failed
extern "C" {
    int urdf_rust_test_function(void);
}

int main() {
    std::cout << "Testing URDF Rust FFI Integration" << std::endl;
    std::cout << "=================================" << std::endl;
    
    // Test basic FFI function call
    int result = urdf_rust_test_function();
    
    std::cout << "urdf_rust_test_function() returned: " << result << std::endl;
    
    if (result == 42) {
        std::cout << "✅ SUCCESS: FFI function returned expected value (42)" << std::endl;
        std::cout << "✅ SUCCESS: Rust library integrated successfully!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ FAILURE: Expected 42, got " << result << std::endl;
        return 1;
    }
}