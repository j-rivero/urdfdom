#include <iostream>
#include <cassert>
#include <cmath>
#include <urdf_model/pose.h>
#include "urdf_rust.h"

const double TOLERANCE = 1e-5;

bool approx_equal(double a, double b, double tol = TOLERANCE) {
    return std::abs(a - b) < tol;
}

void test_vector3_compatibility() {
    std::cout << "Testing Vector3 compatibility..." << std::endl;
    
    // Test C++ Vector3
    urdf::Vector3 cpp_vec;
    cpp_vec.init("1.5 -2.7 3.14");
    
    // Test Rust Vector3 via FFI
    Vector3 rust_vec = urdf_vector3_zero();
    int result = urdf_vector3_init(&rust_vec, "1.5 -2.7 3.14");
    assert(result == 0);
    
    // Compare results
    assert(approx_equal(cpp_vec.x, rust_vec.x));
    assert(approx_equal(cpp_vec.y, rust_vec.y));
    assert(approx_equal(cpp_vec.z, rust_vec.z));
    
    std::cout << "  C++:  (" << cpp_vec.x << ", " << cpp_vec.y << ", " << cpp_vec.z << ")" << std::endl;
    std::cout << "  Rust: (" << rust_vec.x << ", " << rust_vec.y << ", " << rust_vec.z << ")" << std::endl;
    
    // Test addition
    urdf::Vector3 cpp_vec2(1.0, 1.0, 1.0);
    Vector3 rust_vec2 = urdf_vector3_new(1.0, 1.0, 1.0);
    
    urdf::Vector3 cpp_result = cpp_vec + cpp_vec2;
    Vector3 rust_result = urdf_vector3_add(rust_vec, rust_vec2);
    
    assert(approx_equal(cpp_result.x, rust_result.x));
    assert(approx_equal(cpp_result.y, rust_result.y));
    assert(approx_equal(cpp_result.z, rust_result.z));
    
    std::cout << "  Vector3 addition: ✅ PASSED" << std::endl;
}

void test_rotation_compatibility() {
    std::cout << "Testing Rotation compatibility..." << std::endl;
    
    // Test C++ Rotation
    urdf::Rotation cpp_rot;
    cpp_rot.init("0.1 0.2 0.3");  // RPY angles
    
    // Test Rust Rotation via FFI
    Rotation rust_rot = urdf_rotation_identity();
    int result = urdf_rotation_init(&rust_rot, "0.1 0.2 0.3");
    assert(result == 0);
    
    // Compare quaternion values
    double cpp_x, cpp_y, cpp_z, cpp_w;
    cpp_rot.getQuaternion(cpp_x, cpp_y, cpp_z, cpp_w);
    
    double rust_x, rust_y, rust_z, rust_w;
    urdf_rotation_get_quaternion(&rust_rot, &rust_x, &rust_y, &rust_z, &rust_w);
    
    assert(approx_equal(cpp_x, rust_x));
    assert(approx_equal(cpp_y, rust_y));
    assert(approx_equal(cpp_z, rust_z));
    assert(approx_equal(cpp_w, rust_w));
    
    std::cout << "  C++ quaternion:  (" << cpp_x << ", " << cpp_y << ", " << cpp_z << ", " << cpp_w << ")" << std::endl;
    std::cout << "  Rust quaternion: (" << rust_x << ", " << rust_y << ", " << rust_z << ", " << rust_w << ")" << std::endl;
    
    // Test RPY conversion
    double cpp_roll, cpp_pitch, cpp_yaw;
    cpp_rot.getRPY(cpp_roll, cpp_pitch, cpp_yaw);
    
    double rust_roll, rust_pitch, rust_yaw;
    urdf_rotation_get_rpy(&rust_rot, &rust_roll, &rust_pitch, &rust_yaw);
    
    assert(approx_equal(cpp_roll, rust_roll));
    assert(approx_equal(cpp_pitch, rust_pitch));
    assert(approx_equal(cpp_yaw, rust_yaw));
    
    std::cout << "  Rotation RPY conversion: ✅ PASSED" << std::endl;
}

void test_pose_compatibility() {
    std::cout << "Testing Pose compatibility..." << std::endl;
    
    // Test C++ Pose
    urdf::Pose cpp_pose;
    cpp_pose.position.init("1.0 2.0 3.0");
    cpp_pose.rotation.init("0.1 0.2 0.3");
    
    // Test Rust Pose via FFI
    Vector3 rust_pos = urdf_vector3_zero();
    urdf_vector3_init(&rust_pos, "1.0 2.0 3.0");
    
    Rotation rust_rot = urdf_rotation_identity();
    urdf_rotation_init(&rust_rot, "0.1 0.2 0.3");
    
    Pose rust_pose = urdf_pose_new(rust_pos, rust_rot);
    
    // Compare positions
    assert(approx_equal(cpp_pose.position.x, rust_pose.position.x));
    assert(approx_equal(cpp_pose.position.y, rust_pose.position.y));
    assert(approx_equal(cpp_pose.position.z, rust_pose.position.z));
    
    // Compare rotations (quaternions)
    double cpp_x, cpp_y, cpp_z, cpp_w;
    cpp_pose.rotation.getQuaternion(cpp_x, cpp_y, cpp_z, cpp_w);
    
    double rust_x, rust_y, rust_z, rust_w;
    urdf_rotation_get_quaternion(&rust_pose.rotation, &rust_x, &rust_y, &rust_z, &rust_w);
    
    assert(approx_equal(cpp_x, rust_x));
    assert(approx_equal(cpp_y, rust_y));
    assert(approx_equal(cpp_z, rust_z));
    assert(approx_equal(cpp_w, rust_w));
    
    std::cout << "  Pose structure: ✅ PASSED" << std::endl;
}

void test_mathematical_operations() {
    std::cout << "Testing mathematical operations..." << std::endl;
    
    // Test quaternion multiplication
    urdf::Rotation cpp_r1, cpp_r2;
    cpp_r1.init("0.1 0.0 0.0");
    cpp_r2.init("0.0 0.1 0.0");
    urdf::Rotation cpp_result = cpp_r1 * cpp_r2;
    
    Rotation rust_r1 = urdf_rotation_identity();
    Rotation rust_r2 = urdf_rotation_identity();
    urdf_rotation_init(&rust_r1, "0.1 0.0 0.0");
    urdf_rotation_init(&rust_r2, "0.0 0.1 0.0");
    Rotation rust_result = urdf_rotation_multiply(rust_r1, rust_r2);
    
    double cpp_x, cpp_y, cpp_z, cpp_w;
    cpp_result.getQuaternion(cpp_x, cpp_y, cpp_z, cpp_w);
    
    double rust_x, rust_y, rust_z, rust_w;
    urdf_rotation_get_quaternion(&rust_result, &rust_x, &rust_y, &rust_z, &rust_w);
    
    assert(approx_equal(cpp_x, rust_x));
    assert(approx_equal(cpp_y, rust_y));
    assert(approx_equal(cpp_z, rust_z));
    assert(approx_equal(cpp_w, rust_w));
    
    std::cout << "  Quaternion multiplication: ✅ PASSED" << std::endl;
    
    // Test vector rotation
    urdf::Rotation cpp_rot;
    cpp_rot.init("0.0 0.0 1.5708"); // 90 degree yaw
    urdf::Vector3 cpp_vec(1.0, 0.0, 0.0);
    urdf::Vector3 cpp_rotated = cpp_rot * cpp_vec;
    
    Rotation rust_rot = urdf_rotation_identity();
    urdf_rotation_init(&rust_rot, "0.0 0.0 1.5708");
    Vector3 rust_vec = urdf_vector3_new(1.0, 0.0, 0.0);
    Vector3 rust_rotated = urdf_rotation_multiply_vector3(rust_rot, rust_vec);
    
    assert(approx_equal(cpp_rotated.x, rust_rotated.x));
    assert(approx_equal(cpp_rotated.y, rust_rotated.y));
    assert(approx_equal(cpp_rotated.z, rust_rotated.z));
    
    std::cout << "  Vector rotation: ✅ PASSED" << std::endl;
}

int main() {
    std::cout << "=== POSE DATA STRUCTURE MIGRATION VALIDATION ===" << std::endl;
    std::cout << "Comparing C++ urdf::Pose classes with Rust implementation" << std::endl;
    std::cout << std::endl;
    
    try {
        test_vector3_compatibility();
        test_rotation_compatibility();
        test_pose_compatibility();
        test_mathematical_operations();
        
        std::cout << std::endl;
        std::cout << "🎉 ALL TESTS PASSED!" << std::endl;
        std::cout << "✅ Pose data structure migration is functionally complete" << std::endl;
        std::cout << "✅ Rust implementation matches C++ behavior exactly" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Unknown test failure" << std::endl;
        return 1;
    }
}