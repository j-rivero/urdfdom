//! URDF parsing benchmarks
//! 
//! This module provides benchmarks for comparing URDF parsing performance
//! between the Rust implementation and the original C++ urdfdom library.

use criterion::{black_box, criterion_group, criterion_main, Criterion, BenchmarkId, Throughput};
use urdf_rust::parse_urdf_string;
use std::fs;
use std::path::Path;

/// Sample URDF content for benchmarking
const SIMPLE_URDF: &str = r#"<?xml version="1.0"?>
<robot name="simple_robot">
  <link name="base_link">
    <inertial>
      <mass value="1.0"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
    <visual>
      <geometry>
        <box size="1 1 1"/>
      </geometry>
    </visual>
    <collision>
      <geometry>
        <box size="1 1 1"/>
      </geometry>
    </collision>
  </link>
  
  <link name="child_link">
    <inertial>
      <mass value="0.5"/>
      <inertia ixx="0.5" ixy="0.0" ixz="0.0" iyy="0.5" iyz="0.0" izz="0.5"/>
    </inertial>
    <visual>
      <geometry>
        <cylinder radius="0.1" length="0.3"/>
      </geometry>
    </visual>
  </link>
  
  <joint name="simple_joint" type="revolute">
    <parent link="base_link"/>
    <child link="child_link"/>
    <origin xyz="0 0 1" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57" effort="100" velocity="1.0"/>
  </joint>
</robot>"#;

/// Complex URDF with multiple links and joints
fn generate_complex_urdf(num_links: usize) -> String {
    let mut urdf = String::from(r#"<?xml version="1.0"?>
<robot name="complex_robot">"#);
    
    // Add base link
    urdf.push_str(r#"
  <link name="base_link">
    <inertial>
      <mass value="10.0"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
    <visual>
      <geometry>
        <box size="0.5 0.5 0.1"/>
      </geometry>
    </visual>
    <collision>
      <geometry>
        <box size="0.5 0.5 0.1"/>
      </geometry>
    </collision>
  </link>"#);
    
    // Add multiple links and joints
    for i in 1..num_links {
        urdf.push_str(&format!(r#"
  <link name="link_{}">
    <inertial>
      <mass value="{}"/>
      <inertia ixx="0.1" ixy="0.0" ixz="0.0" iyy="0.1" iyz="0.0" izz="0.1"/>
    </inertial>
    <visual>
      <geometry>
        <cylinder radius="0.05" length="0.2"/>
      </geometry>
    </visual>
    <collision>
      <geometry>
        <cylinder radius="0.05" length="0.2"/>
      </geometry>
    </collision>
  </link>
  
  <joint name="joint_{}" type="revolute">
    <parent link="{}"/>
    <child link="link_{}"/>
    <origin xyz="{} 0 0" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-3.14" upper="3.14" effort="50" velocity="2.0"/>
  </joint>"#, 
            i, 
            1.0 / i as f64,  // varying mass
            i,
            if i == 1 { "base_link".to_string() } else { format!("link_{}", i - 1) },
            i,
            0.2 * i as f64   // varying position
        ));
    }
    
    urdf.push_str("\n</robot>");
    urdf
}

/// Benchmark simple URDF parsing
fn bench_simple_parsing(c: &mut Criterion) {
    let mut group = c.benchmark_group("simple_parsing");
    group.throughput(Throughput::Bytes(SIMPLE_URDF.len() as u64));
    
    group.bench_function("rust_simple", |b| {
        b.iter(|| {
            parse_urdf_string(black_box(SIMPLE_URDF)).unwrap()
        })
    });
    
    group.finish();
}

/// Benchmark complex URDF parsing with varying sizes
fn bench_complex_parsing(c: &mut Criterion) {
    let mut group = c.benchmark_group("complex_parsing");
    
    for size in [10, 50, 100, 200].iter() {
        let urdf_content = generate_complex_urdf(*size);
        group.throughput(Throughput::Bytes(urdf_content.len() as u64));
        
        group.bench_with_input(
            BenchmarkId::new("rust_complex", size),
            &urdf_content,
            |b, urdf| {
                b.iter(|| {
                    parse_urdf_string(black_box(urdf)).unwrap()
                })
            },
        );
    }
    
    group.finish();
}

/// Benchmark parsing from different input sources
fn bench_parsing_sources(c: &mut Criterion) {
    let mut group = c.benchmark_group("parsing_sources");
    
    // Test string parsing (already covered above)
    group.bench_function("from_string", |b| {
        b.iter(|| {
            parse_urdf_string(black_box(SIMPLE_URDF)).unwrap()
        })
    });
    
    // Test parsing with validation
    #[cfg(feature = "validation")]
    group.bench_function("with_validation", |b| {
        b.iter(|| {
            // This would call a validation-enabled parser
            parse_urdf_string(black_box(SIMPLE_URDF)).unwrap()
        })
    });
    
    group.finish();
}

/// Benchmark memory allocation patterns
fn bench_memory_patterns(c: &mut Criterion) {
    let mut group = c.benchmark_group("memory_patterns");
    
    // Test repeated parsing (memory reuse)
    group.bench_function("repeated_parsing", |b| {
        b.iter(|| {
            for _ in 0..10 {
                let _robot = parse_urdf_string(black_box(SIMPLE_URDF)).unwrap();
                // Robot is dropped here, testing memory cleanup
            }
        })
    });
    
    // Test parsing with large strings
    let large_urdf = generate_complex_urdf(500);
    group.throughput(Throughput::Bytes(large_urdf.len() as u64));
    
    group.bench_function("large_urdf", |b| {
        b.iter(|| {
            parse_urdf_string(black_box(&large_urdf)).unwrap()
        })
    });
    
    group.finish();
}

/// Benchmark SIMD optimizations if available
#[cfg(feature = "simd")]
fn bench_simd_optimizations(c: &mut Criterion) {
    let mut group = c.benchmark_group("simd_optimizations");
    
    let urdf_content = generate_complex_urdf(100);
    group.throughput(Throughput::Bytes(urdf_content.len() as u64));
    
    group.bench_function("simd_enabled", |b| {
        b.iter(|| {
            parse_urdf_string(black_box(&urdf_content)).unwrap()
        })
    });
    
    group.finish();
}

/// Load real URDF files for benchmarking if available
fn bench_real_urdf_files(c: &mut Criterion) {
    let mut group = c.benchmark_group("real_urdf_files");
    
    // Try to find URDF files in common ROS locations
    let search_paths = [
        "/opt/ros/humble/share",
        "/opt/ros/iron/share", 
        "/opt/ros/rolling/share",
        "../../../install/share",  // Local colcon workspace
        "test_data",               // Test data directory
    ];
    
    let mut found_files = Vec::new();
    
    for search_path in &search_paths {
        if let Ok(entries) = fs::read_dir(search_path) {
            for entry in entries.flatten() {
                let path = entry.path();
                if path.is_dir() {
                    // Look for URDF files in package directories
                    let urdf_dir = path.join("urdf");
                    if urdf_dir.exists() {
                        if let Ok(urdf_entries) = fs::read_dir(urdf_dir) {
                            for urdf_entry in urdf_entries.flatten() {
                                let urdf_path = urdf_entry.path();
                                if urdf_path.extension().and_then(|s| s.to_str()) == Some("urdf") {
                                    found_files.push(urdf_path);
                                    if found_files.len() >= 5 {  // Limit to 5 files
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    if found_files.is_empty() {
        // If no real URDF files found, use generated ones
        let sizes = [50, 100, 200];
        for &size in &sizes {
            let urdf_content = generate_complex_urdf(size);
            group.throughput(Throughput::Bytes(urdf_content.len() as u64));
            
            group.bench_with_input(
                BenchmarkId::new("generated", size),
                &urdf_content,
                |b, urdf| {
                    b.iter(|| {
                        parse_urdf_string(black_box(urdf)).unwrap()
                    })
                },
            );
        }
    } else {
        // Benchmark real URDF files
        for urdf_path in found_files {
            if let Ok(content) = fs::read_to_string(&urdf_path) {
                let filename = urdf_path.file_name()
                    .unwrap_or_default()
                    .to_string_lossy()
                    .to_string();
                
                group.throughput(Throughput::Bytes(content.len() as u64));
                
                group.bench_with_input(
                    BenchmarkId::new("real_file", &filename),
                    &content,
                    |b, urdf| {
                        b.iter(|| {
                            parse_urdf_string(black_box(urdf)).unwrap()
                        })
                    },
                );
            }
        }
    }
    
    group.finish();
}

// Create benchmark groups
criterion_group!(
    benches,
    bench_simple_parsing,
    bench_complex_parsing,
    bench_parsing_sources,
    bench_memory_patterns,
    bench_real_urdf_files
);

// Add SIMD benchmarks if feature is enabled
#[cfg(feature = "simd")]
criterion_group!(
    simd_benches,
    bench_simd_optimizations
);

// Main benchmark runner
#[cfg(feature = "simd")]
criterion_main!(benches, simd_benches);

#[cfg(not(feature = "simd"))]
criterion_main!(benches);