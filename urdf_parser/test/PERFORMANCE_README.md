# URDF Parser Performance Testing

This directory contains performance benchmarks for the URDF parser library, designed to demonstrate and measure the parsing speed capabilities of the urdfdom software.

## Overview

The performance test suite measures URDF parsing speed across different complexity levels:

- **Simple URDF**: Minimal robot description (1 link)
- **Medium URDF**: Multi-link robot with materials and joints
- **Complex URDF**: Detailed robot arm with complete specifications
- **Large URDF**: Programmatically generated robots with 50+ links
- **Very Large URDF**: Stress test with 100+ links

## Running Performance Tests

### Quick Test
For a rapid performance check:
```bash
make performance-test-quick
```

### Full Benchmark Suite
For comprehensive performance analysis:
```bash
make performance-test
```

### Manual Execution
Run the performance test binary directly:
```bash
# Quick test
./bin/urdf_performance_test --quick

# Full benchmark
./bin/urdf_performance_test

# Help
./bin/urdf_performance_test --help
```

## Integration with Build System

The performance tests are integrated with the CMake build system:

1. **Build Integration**: The performance test is built alongside other tests
2. **Custom Targets**: Dedicated `performance-test` and `performance-test-quick` targets
3. **Standalone Execution**: Can be run independently without affecting unit tests

## Performance Metrics

The benchmark reports several key metrics:

- **Parsing Time**: Average, minimum, and maximum parse times
- **Throughput**: Data processing rate in MB/s
- **Parse Rate**: Number of URDF files parsed per second
- **Scalability**: Performance across different URDF sizes

## Example Results

```
=== Performance Summary ===
Test Name                Size (KB)   Avg (ms)    Rate (MB/s) Parses/sec     
----------------------------------------------------------------------------
Simple URDF              0.2         0.007       24.12       148645.2       
Medium URDF              1.8         0.075       22.25       13375.5        
Complex URDF             5.4         0.202       25.34       4959.2         
Large URDF (50 links)    33.9        1.216       26.42       822.3          
Very Large URDF (100 links)67.3        2.390       26.62       418.4          
----------------------------------------------------------------------------
Overall Statistics:
  Total data processed: 39.3 MB
  Total parsing time:   1.52 seconds
  Total parses:         8700
  Average throughput:   25.86 MB/s
  Average parse rate:   5724.7 parses/s
```

## Technical Details

### Test Implementation
- **Language**: C++14 compatible
- **Timing**: High-resolution clock for microsecond precision  
- **Memory Management**: Uses smart pointers for automatic cleanup
- **Error Handling**: Comprehensive exception handling and validation

### Test Data
- **Embedded URDF**: Test data is embedded in source code for portability
- **Dynamic Generation**: Large URDFs are generated programmatically
- **Realistic Content**: Tests use valid URDF structures with proper geometry, materials, and physics properties

### Build Requirements
- CMake 3.5+
- C++14 compatible compiler
- urdfdom libraries
- TinyXML2 (for XML parsing)

## Continuous Integration

The performance tests can be integrated into CI/CD pipelines:

```yaml
# Example CI step
- name: Run Performance Benchmarks
  run: |
    cd build
    make performance-test-quick
```

## Interpreting Results

- **High parse rates** indicate efficient XML processing
- **Consistent timing** shows stable performance across runs
- **Scaling behavior** reveals how performance changes with URDF complexity
- **Throughput measurements** demonstrate real-world processing capabilities

The benchmarks help validate that the URDF parser maintains high performance while handling complex robot descriptions efficiently.