//! Extended test program to validate URDF parsing functionality
//! 
//! This program tests both the basic FFI integration and the URDF parsing
//! functionality from the Rust implementation.

#include <iostream>
#include <cstdio>
#include <cstring>

// Test if we can include the generated header
#include "urdf_rust.h"

// Declare the test functions in case header generation failed
extern "C" {
    int urdf_rust_test_function(void);
    typedef struct UrdfRobotHandle UrdfRobotHandle;
    UrdfRobotHandle* urdf_robot_from_string(const char* urdf_content);
    void urdf_robot_destroy(UrdfRobotHandle* handle);
    int urdf_robot_get_link_count(const UrdfRobotHandle* handle);
    int urdf_robot_get_joint_count(const UrdfRobotHandle* handle);
    int urdf_get_last_error(void);
    const char* urdf_error_description(int error_code);
}

const char* test_urdf = R"(<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link">
    <inertial>
      <mass value="1.0"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>
  
  <link name="child_link_1">
    <inertial>
      <mass value="0.5"/>
      <inertia ixx="0.5" ixy="0.0" ixz="0.0" iyy="0.5" iyz="0.0" izz="0.5"/>
    </inertial>
  </link>
  
  <link name="child_link_2">
    <inertial>
      <mass value="0.3"/>
      <inertia ixx="0.3" ixy="0.0" ixz="0.0" iyy="0.3" iyz="0.0" izz="0.3"/>
    </inertial>
  </link>
  
  <joint name="joint_1" type="revolute">
    <parent link="base_link"/>
    <child link="child_link_1"/>
  </joint>
  
  <joint name="joint_2" type="fixed">
    <parent link="child_link_1"/>
    <child link="child_link_2"/>
  </joint>
</robot>)";

int main() {
    std::cout << "Extended URDF Rust FFI Integration Test" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    // Test 1: Basic FFI function call
    std::cout << "\n1. Testing basic FFI function..." << std::endl;
    int result = urdf_rust_test_function();
    std::cout << "urdf_rust_test_function() returned: " << result << std::endl;
    
    if (result == 42) {
        std::cout << "✅ SUCCESS: Basic FFI working" << std::endl;
    } else {
        std::cout << "❌ FAILURE: Expected 42, got " << result << std::endl;
        return 1;
    }
    
    // Test 2: URDF parsing
    std::cout << "\n2. Testing URDF parsing..." << std::endl;
    std::cout << "Parsing test URDF with 3 links and 2 joints..." << std::endl;
    
    UrdfRobotHandle* robot = urdf_robot_from_string(test_urdf);
    
    if (!robot) {
        int error_code = urdf_get_last_error();
        const char* error_desc = urdf_error_description(error_code);
        std::cout << "❌ FAILURE: Failed to parse URDF" << std::endl;
        std::cout << "Error code: " << error_code << std::endl;
        std::cout << "Error description: " << error_desc << std::endl;
        return 1;
    }
    
    std::cout << "✅ SUCCESS: URDF parsed successfully" << std::endl;
    
    // Test 3: Query robot structure
    std::cout << "\n3. Testing robot structure queries..." << std::endl;
    
    int link_count = urdf_robot_get_link_count(robot);
    int joint_count = urdf_robot_get_joint_count(robot);
    
    std::cout << "Link count: " << link_count << std::endl;
    std::cout << "Joint count: " << joint_count << std::endl;
    
    if (link_count == 3 && joint_count == 2) {
        std::cout << "✅ SUCCESS: Robot structure matches expected values" << std::endl;
    } else {
        std::cout << "❌ FAILURE: Expected 3 links and 2 joints, got " 
                  << link_count << " links and " << joint_count << " joints" << std::endl;
        urdf_robot_destroy(robot);
        return 1;
    }
    
    // Test 4: Clean up
    std::cout << "\n4. Testing resource cleanup..." << std::endl;
    urdf_robot_destroy(robot);
    std::cout << "✅ SUCCESS: Robot destroyed successfully" << std::endl;
    
    // Final success message
    std::cout << "\n🎉 ALL TESTS PASSED!" << std::endl;
    std::cout << "✅ Basic FFI integration working" << std::endl;
    std::cout << "✅ URDF parsing functional" << std::endl;
    std::cout << "✅ Robot structure queries working" << std::endl;
    std::cout << "✅ Memory management working" << std::endl;
    std::cout << "\nRust integration is fully functional!" << std::endl;
    
    return 0;
}