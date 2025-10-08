/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2025, Performance Test Authors
*  All rights reserved.
*
*  This file contains performance benchmarks for URDF parsing
*  to demonstrate the speed capabilities of the urdfdom library.
*********************************************************************/

#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <memory>

#include <urdf_parser/urdf_parser.h>
#include <urdf_model/model.h>

class URDFPerformanceBenchmark {
public:
    struct BenchmarkResult {
        std::string test_name;
        size_t iterations;
        double total_time_ms;
        double avg_time_ms;
        double min_time_ms;
        double max_time_ms;
        size_t urdf_size_bytes;
        double parsing_rate_mb_per_sec;
    };

public:
    // Simple URDF with minimal content
    static const std::string simple_urdf;
    
    // Medium complexity URDF with multiple links and joints
    static const std::string medium_urdf;
    
    // Complex URDF with many links, joints, and detailed geometry
    static const std::string complex_urdf;
    
    // Generate a large URDF programmatically for stress testing
    static std::string generateLargeURDF(int num_links);
    static BenchmarkResult measureParsingPerformance(const std::string& test_name, 
                                                    const std::string& urdf_content, 
                                                    size_t iterations = 1000) {
        BenchmarkResult result;
        result.test_name = test_name;
        result.iterations = iterations;
        result.urdf_size_bytes = urdf_content.size();
        
        std::vector<double> times;
        times.reserve(iterations);
        
        std::cout << "Running " << test_name << " (" << iterations << " iterations)..." << std::endl;
        
        auto start_total = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            
            // Parse the URDF
            urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_content);
            
            auto end = std::chrono::high_resolution_clock::now();
            
            // Verify parsing succeeded
            if (!model) {
                std::cerr << "Failed to parse URDF on iteration " << i << std::endl;
                continue;
            }
            
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
            times.push_back(duration.count() / 1e6); // Convert to milliseconds
        }
        
        auto end_total = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_total - start_total);
        result.total_time_ms = total_duration.count() / 1e6;
        
        // Calculate statistics
        if (!times.empty()) {
            double sum = 0.0;
            result.min_time_ms = times[0];
            result.max_time_ms = times[0];
            
            for (double time : times) {
                sum += time;
                result.min_time_ms = std::min(result.min_time_ms, time);
                result.max_time_ms = std::max(result.max_time_ms, time);
            }
            
            result.avg_time_ms = sum / times.size();
            
            // Calculate parsing rate in MB/s
            double data_processed_mb = (result.urdf_size_bytes * iterations) / (1024.0 * 1024.0);
            result.parsing_rate_mb_per_sec = data_processed_mb / (result.total_time_ms / 1000.0);
        }
        
        return result;
    }
    
    static void printResult(const BenchmarkResult& result) {
        std::cout << "\n=== " << result.test_name << " Results ===" << std::endl;
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Iterations:        " << result.iterations << std::endl;
        std::cout << "URDF Size:         " << result.urdf_size_bytes << " bytes" << std::endl;
        std::cout << "Total Time:        " << result.total_time_ms << " ms" << std::endl;
        std::cout << "Average Time:      " << result.avg_time_ms << " ms" << std::endl;
        std::cout << "Min Time:          " << result.min_time_ms << " ms" << std::endl;
        std::cout << "Max Time:          " << result.max_time_ms << " ms" << std::endl;
        std::cout << "Parsing Rate:      " << result.parsing_rate_mb_per_sec << " MB/s" << std::endl;
        std::cout << "Parses per Second: " << (1000.0 / result.avg_time_ms) << std::endl;
        std::cout << std::endl;
    }
    
    static void runAllBenchmarks() {
        std::cout << "URDF Parser Performance Benchmark Suite" << std::endl;
        std::cout << "=========================================" << std::endl;
        
        std::vector<BenchmarkResult> results;
        
        // Test 1: Simple URDF
        results.push_back(measureParsingPerformance("Simple URDF", simple_urdf, 5000));
        printResult(results.back());
        
        // Test 2: Medium complexity URDF
        results.push_back(measureParsingPerformance("Medium URDF", medium_urdf, 2000));
        printResult(results.back());
        
        // Test 3: Complex URDF
        results.push_back(measureParsingPerformance("Complex URDF", complex_urdf, 1000));
        printResult(results.back());
        
        // Test 4: Large programmatically generated URDF
        std::string large_urdf = generateLargeURDF(50);
        results.push_back(measureParsingPerformance("Large URDF (50 links)", large_urdf, 500));
        printResult(results.back());
        
        // Test 5: Very large URDF
        std::string very_large_urdf = generateLargeURDF(100);
        results.push_back(measureParsingPerformance("Very Large URDF (100 links)", very_large_urdf, 200));
        printResult(results.back());
        
        // Summary
        printSummary(results);
    }
    
    static void printSummary(const std::vector<BenchmarkResult>& results) {
        std::cout << "\n=== Performance Summary ===" << std::endl;
        std::cout << std::left << std::setw(25) << "Test Name" 
                  << std::setw(12) << "Size (KB)" 
                  << std::setw(12) << "Avg (ms)" 
                  << std::setw(12) << "Rate (MB/s)" 
                  << std::setw(15) << "Parses/sec" << std::endl;
        std::cout << std::string(76, '-') << std::endl;
        
        for (const auto& result : results) {
            std::cout << std::left << std::setw(25) << result.test_name
                      << std::setw(12) << std::fixed << std::setprecision(1) << (result.urdf_size_bytes / 1024.0)
                      << std::setw(12) << std::fixed << std::setprecision(3) << result.avg_time_ms
                      << std::setw(12) << std::fixed << std::setprecision(2) << result.parsing_rate_mb_per_sec
                      << std::setw(15) << std::fixed << std::setprecision(1) << (1000.0 / result.avg_time_ms) 
                      << std::endl;
        }
        
        // Calculate overall statistics
        double total_data_mb = 0.0;
        double total_time_s = 0.0;
        size_t total_parses = 0;
        
        for (const auto& result : results) {
            total_data_mb += (result.urdf_size_bytes * result.iterations) / (1024.0 * 1024.0);
            total_time_s += result.total_time_ms / 1000.0;
            total_parses += result.iterations;
        }
        
        std::cout << std::string(76, '-') << std::endl;
        std::cout << "Overall Statistics:" << std::endl;
        std::cout << "  Total data processed: " << std::fixed << std::setprecision(1) << total_data_mb << " MB" << std::endl;
        std::cout << "  Total parsing time:   " << std::fixed << std::setprecision(2) << total_time_s << " seconds" << std::endl;
        std::cout << "  Total parses:         " << total_parses << std::endl;
        std::cout << "  Average throughput:   " << std::fixed << std::setprecision(2) << (total_data_mb / total_time_s) << " MB/s" << std::endl;
        std::cout << "  Average parse rate:   " << std::fixed << std::setprecision(1) << (total_parses / total_time_s) << " parses/s" << std::endl;
    }
};

// Simple URDF definition
const std::string URDFPerformanceBenchmark::simple_urdf = R"(
<?xml version="1.0"?>
<robot name="simple_robot">
  <link name="base_link">
    <visual>
      <geometry>
        <box size="1 1 1"/>
      </geometry>
    </visual>
  </link>
</robot>
)";

// Medium complexity URDF definition
const std::string URDFPerformanceBenchmark::medium_urdf = R"(
<?xml version="1.0"?>
<robot name="medium_robot">
  <material name="red">
    <color rgba="1 0 0 1"/>
  </material>
  <material name="blue">
    <color rgba="0 0 1 1"/>
  </material>
  
  <link name="base_link">
    <inertial>
      <mass value="1.0"/>
      <inertia ixx="0.1" ixy="0" ixz="0" iyy="0.1" iyz="0" izz="0.1"/>
    </inertial>
    <visual>
      <geometry>
        <box size="0.5 0.5 0.1"/>
      </geometry>
      <material name="red"/>
    </visual>
    <collision>
      <geometry>
        <box size="0.5 0.5 0.1"/>
      </geometry>
    </collision>
  </link>
  
  <link name="link1">
    <inertial>
      <mass value="0.5"/>
      <inertia ixx="0.05" ixy="0" ixz="0" iyy="0.05" iyz="0" izz="0.05"/>
    </inertial>
    <visual>
      <geometry>
        <cylinder radius="0.1" length="0.3"/>
      </geometry>
      <material name="blue"/>
    </visual>
    <collision>
      <geometry>
        <cylinder radius="0.1" length="0.3"/>
      </geometry>
    </collision>
  </link>
  
  <joint name="joint1" type="revolute">
    <parent link="base_link"/>
    <child link="link1"/>
    <origin xyz="0 0 0.2" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-3.14" upper="3.14" effort="10" velocity="1"/>
    <dynamics damping="0.1" friction="0.01"/>
  </joint>
  
  <link name="link2">
    <inertial>
      <mass value="0.3"/>
      <inertia ixx="0.03" ixy="0" ixz="0" iyy="0.03" iyz="0" izz="0.03"/>
    </inertial>
    <visual>
      <geometry>
        <sphere radius="0.08"/>
      </geometry>
      <material name="red"/>
    </visual>
  </link>
  
  <joint name="joint2" type="prismatic">
    <parent link="link1"/>
    <child link="link2"/>
    <origin xyz="0 0 0.2" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="0" upper="0.1" effort="5" velocity="0.5"/>
  </joint>
</robot>
)";

// Complex URDF definition
const std::string URDFPerformanceBenchmark::complex_urdf = R"(
<?xml version="1.0"?>
<robot name="complex_robot">
  <!-- Materials -->
  <material name="black">
    <color rgba="0 0 0 1"/>
  </material>
  <material name="white">
    <color rgba="1 1 1 1"/>
  </material>
  <material name="red">
    <color rgba="1 0 0 1"/>
  </material>
  <material name="green">
    <color rgba="0 1 0 1"/>
  </material>
  <material name="blue">
    <color rgba="0 0 1 1"/>
  </material>
  
  <!-- Base -->
  <link name="base_link">
    <inertial>
      <mass value="10.0"/>
      <origin xyz="0 0 0.05"/>
      <inertia ixx="0.8" ixy="0" ixz="0" iyy="0.8" iyz="0" izz="1.2"/>
    </inertial>
    <visual>
      <geometry>
        <box size="0.6 0.4 0.1"/>
      </geometry>
      <material name="black"/>
    </visual>
    <collision>
      <geometry>
        <box size="0.6 0.4 0.1"/>
      </geometry>
    </collision>
  </link>
  
  <!-- Arm segments -->
  <link name="shoulder_link">
    <inertial>
      <mass value="2.0"/>
      <inertia ixx="0.1" ixy="0" ixz="0" iyy="0.1" iyz="0" izz="0.1"/>
    </inertial>
    <visual>
      <geometry>
        <cylinder radius="0.08" length="0.2"/>
      </geometry>
      <material name="red"/>
    </visual>
    <collision>
      <geometry>
        <cylinder radius="0.08" length="0.2"/>
      </geometry>
    </collision>
  </link>
  
  <joint name="shoulder_joint" type="revolute">
    <parent link="base_link"/>
    <child link="shoulder_link"/>
    <origin xyz="0 0 0.15" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-3.14" upper="3.14" effort="30" velocity="2"/>
    <dynamics damping="0.5" friction="0.1"/>
  </joint>
  
  <link name="upper_arm_link">
    <inertial>
      <mass value="1.5"/>
      <origin xyz="0 0 0.15"/>
      <inertia ixx="0.05" ixy="0" ixz="0" iyy="0.05" iyz="0" izz="0.02"/>
    </inertial>
    <visual>
      <geometry>
        <box size="0.06 0.06 0.3"/>
      </geometry>
      <material name="green"/>
    </visual>
    <collision>
      <geometry>
        <box size="0.06 0.06 0.3"/>
      </geometry>
    </collision>
  </link>
  
  <joint name="shoulder_pitch_joint" type="revolute">
    <parent link="shoulder_link"/>
    <child link="upper_arm_link"/>
    <origin xyz="0 0 0.1" rpy="0 0 0"/>
    <axis xyz="0 1 0"/>
    <limit lower="-1.57" upper="1.57" effort="25" velocity="1.5"/>
    <dynamics damping="0.3" friction="0.05"/>
  </joint>
  
  <link name="forearm_link">
    <inertial>
      <mass value="1.0"/>
      <origin xyz="0 0 0.1"/>
      <inertia ixx="0.03" ixy="0" ixz="0" iyy="0.03" iyz="0" izz="0.01"/>
    </inertial>
    <visual>
      <geometry>
        <cylinder radius="0.03" length="0.2"/>
      </geometry>
      <material name="blue"/>
    </visual>
    <collision>
      <geometry>
        <cylinder radius="0.03" length="0.2"/>
      </geometry>
    </collision>
  </link>
  
  <joint name="elbow_joint" type="revolute">
    <parent link="upper_arm_link"/>
    <child link="forearm_link"/>
    <origin xyz="0 0 0.3" rpy="0 0 0"/>
    <axis xyz="0 1 0"/>
    <limit lower="-2.0" upper="2.0" effort="20" velocity="2"/>
    <dynamics damping="0.2" friction="0.02"/>
  </joint>
  
  <!-- Wrist and end effector -->
  <link name="wrist_link">
    <inertial>
      <mass value="0.5"/>
      <inertia ixx="0.01" ixy="0" ixz="0" iyy="0.01" iyz="0" izz="0.01"/>
    </inertial>
    <visual>
      <geometry>
        <sphere radius="0.04"/>
      </geometry>
      <material name="white"/>
    </visual>
    <collision>
      <geometry>
        <sphere radius="0.04"/>
      </geometry>
    </collision>
  </link>
  
  <joint name="wrist_joint" type="revolute">
    <parent link="forearm_link"/>
    <child link="wrist_link"/>
    <origin xyz="0 0 0.2" rpy="0 0 0"/>
    <axis xyz="1 0 0"/>
    <limit lower="-3.14" upper="3.14" effort="10" velocity="3"/>
    <dynamics damping="0.1" friction="0.01"/>
  </joint>
  
  <link name="gripper_base_link">
    <inertial>
      <mass value="0.3"/>
      <inertia ixx="0.005" ixy="0" ixz="0" iyy="0.005" iyz="0" izz="0.005"/>
    </inertial>
    <visual>
      <geometry>
        <box size="0.08 0.04 0.02"/>
      </geometry>
      <material name="black"/>
    </visual>
  </link>
  
  <joint name="gripper_joint" type="fixed">
    <parent link="wrist_link"/>
    <child link="gripper_base_link"/>
    <origin xyz="0 0 0.06" rpy="0 0 0"/>
  </joint>
  
  <!-- Gripper fingers -->
  <link name="gripper_finger1">
    <inertial>
      <mass value="0.1"/>
      <inertia ixx="0.001" ixy="0" ixz="0" iyy="0.001" iyz="0" izz="0.001"/>
    </inertial>
    <visual>
      <geometry>
        <box size="0.02 0.01 0.04"/>
      </geometry>
      <material name="red"/>
    </visual>
  </link>
  
  <joint name="gripper_finger1_joint" type="prismatic">
    <parent link="gripper_base_link"/>
    <child link="gripper_finger1"/>
    <origin xyz="0.02 0 0" rpy="0 0 0"/>
    <axis xyz="1 0 0"/>
    <limit lower="0" upper="0.02" effort="5" velocity="0.1"/>
  </joint>
  
  <link name="gripper_finger2">
    <inertial>
      <mass value="0.1"/>
      <inertia ixx="0.001" ixy="0" ixz="0" iyy="0.001" iyz="0" izz="0.001"/>
    </inertial>
    <visual>
      <geometry>
        <box size="0.02 0.01 0.04"/>
      </geometry>
      <material name="red"/>
    </visual>
  </link>
  
  <joint name="gripper_finger2_joint" type="prismatic">
    <parent link="gripper_base_link"/>
    <child link="gripper_finger2"/>
    <origin xyz="-0.02 0 0" rpy="0 0 0"/>
    <axis xyz="1 0 0"/>
    <limit lower="-0.02" upper="0" effort="5" velocity="0.1"/>
  </joint>
</robot>
)";

std::string URDFPerformanceBenchmark::generateLargeURDF(int num_links) {
    std::ostringstream urdf;
    
    urdf << R"(<?xml version="1.0"?>
<robot name="large_generated_robot">
  <material name="red">
    <color rgba="1 0 0 1"/>
  </material>
  <material name="blue">
    <color rgba="0 0 1 1"/>
  </material>
  
  <link name="base_link">
    <inertial>
      <mass value="1.0"/>
      <inertia ixx="0.1" ixy="0" ixz="0" iyy="0.1" iyz="0" izz="0.1"/>
    </inertial>
    <visual>
      <geometry>
        <box size="0.1 0.1 0.1"/>
      </geometry>
      <material name="red"/>
    </visual>
    <collision>
      <geometry>
        <box size="0.1 0.1 0.1"/>
      </geometry>
    </collision>
  </link>
)";
    
    for (int i = 1; i <= num_links; ++i) {
        urdf << "\n  <link name=\"link" << i << "\">\n"
             << "    <inertial>\n"
             << "      <mass value=\"0.5\"/>\n"
             << "      <inertia ixx=\"0.05\" ixy=\"0\" ixz=\"0\" iyy=\"0.05\" iyz=\"0\" izz=\"0.05\"/>\n"
             << "    </inertial>\n"
             << "    <visual>\n"
             << "      <geometry>\n";
        
        // Vary geometry types
        switch (i % 3) {
            case 0:
                urdf << "        <box size=\"0.1 0.1 0.1\"/>\n";
                break;
            case 1:
                urdf << "        <cylinder radius=\"0.05\" length=\"0.1\"/>\n";
                break;
            case 2:
                urdf << "        <sphere radius=\"0.05\"/>\n";
                break;
        }
        
        urdf << "      </geometry>\n"
             << "      <material name=\"" << (i % 2 == 0 ? "red" : "blue") << "\"/>\n"
             << "    </visual>\n"
             << "    <collision>\n"
             << "      <geometry>\n";
        
        // Same geometry for collision
        switch (i % 3) {
            case 0:
                urdf << "        <box size=\"0.1 0.1 0.1\"/>\n";
                break;
            case 1:
                urdf << "        <cylinder radius=\"0.05\" length=\"0.1\"/>\n";
                break;
            case 2:
                urdf << "        <sphere radius=\"0.05\"/>\n";
                break;
        }
        
        urdf << "      </geometry>\n"
             << "    </collision>\n"
             << "  </link>\n";
        
        // Add joint
        std::string parent = (i == 1) ? "base_link" : ("link" + std::to_string(i-1));
        urdf << "\n  <joint name=\"joint" << i << "\" type=\"revolute\">\n"
             << "    <parent link=\"" << parent << "\"/>\n"
             << "    <child link=\"link" << i << "\"/>\n"
             << "    <origin xyz=\"0.1 0 0\" rpy=\"0 0 0\"/>\n"
             << "    <axis xyz=\"0 0 1\"/>\n"
             << "    <limit lower=\"-3.14\" upper=\"3.14\" effort=\"10\" velocity=\"1\"/>\n"
             << "    <dynamics damping=\"0.1\" friction=\"0.01\"/>\n"
             << "  </joint>\n";
    }
    
    urdf << "\n</robot>\n";
    return urdf.str();
}

int main(int argc, char** argv) {
    try {
        std::cout << "URDF Parser Performance Test" << std::endl;
        std::cout << "Build Date: " << __DATE__ << " " << __TIME__ << std::endl;
        std::cout << "=========================================" << std::endl;
        
        // Check for command line arguments to customize test
        if (argc > 1) {
            std::string arg = argv[1];
            if (arg == "--quick") {
                std::cout << "Running quick benchmark..." << std::endl;
                auto result = URDFPerformanceBenchmark::measureParsingPerformance("Quick Test", 
                    URDFPerformanceBenchmark::simple_urdf, 100);
                URDFPerformanceBenchmark::printResult(result);
                return 0;
            } else if (arg == "--help") {
                std::cout << "Usage: " << argv[0] << " [--quick|--help]" << std::endl;
                std::cout << "  --quick: Run a quick benchmark with fewer iterations" << std::endl;
                std::cout << "  --help:  Show this help message" << std::endl;
                std::cout << "  (no args): Run full benchmark suite" << std::endl;
                return 0;
            }
        }
        
        // Run full benchmark suite
        URDFPerformanceBenchmark::runAllBenchmarks();
        
        std::cout << "\nPerformance test completed successfully!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Performance test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Performance test failed with unknown exception" << std::endl;
        return 1;
    }
}