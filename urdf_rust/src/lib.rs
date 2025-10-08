//! # URDF Rust - Rust Implementation of URDF Parser
//! 
//! This crate provides a Rust implementation of the URDF (Unified Robot Description Format) parser.
//! It's designed as an incremental migration from the existing C++ urdfdom library, providing
//! FFI compatibility for seamless integration during the transition period.
//! 
//! ## Features
//! 
//! - **FFI Compatibility**: C-compatible interface for integration with existing C++ code
//! - **Performance**: Leveraging Rust's zero-cost abstractions and memory safety
//! - **XML Parsing**: Fast XML parsing using quick-xml
//! - **Gradual Migration**: Designed to coexist with existing C++ implementation
//! 
//! ## Architecture
//! 
//! The crate is organized into several modules:
//! - `ffi`: Foreign Function Interface for C++ interoperability
//! - `model`: Core URDF model structures and types
//! - `parser`: XML parsing and validation logic
//! - `error`: Error handling and reporting
//! 
//! ## Usage
//! 
//! From Rust:
//! ```rust
//! use urdf_rust::parse_urdf_string;
//! 
//! let urdf_content = r#"<?xml version="1.0"?>
//! <robot name="test_robot">
//!   <link name="base_link"/>
//! </robot>"#;
//! 
//! match parse_urdf_string(urdf_content) {
//!     Ok(model) => println!("Parsed robot: {}", model.name),
//!     Err(e) => eprintln!("Parse error: {}", e),
//! }
//! ```
//! 
//! From C++:
//! ```cpp
//! #include "urdf_rust.h"
//! 
//! int main() {
//!     int result = urdf_rust_test_function();
//!     printf("Rust function returned: %d\n", result);
//!     return 0;
//! }
//! ```

use std::ffi::CStr;
use std::os::raw::{c_char, c_int};

pub mod ffi;
pub mod model;
pub mod parser;
pub mod utils;
pub mod error;
pub mod pose;

// Re-export main types for convenience
pub use model::{Robot, Link, Joint, Material, Geometry};
pub use parser::{parse_urdf_string};
pub use utils::{parse_vector3};
pub use error::{UrdfError, UrdfResult};
pub use pose::{Vector3, Rotation, Pose};

/// Version information matching the C++ urdfdom version
pub const VERSION: &str = env!("CARGO_PKG_VERSION");
pub const VERSION_MAJOR: u32 = 5;
pub const VERSION_MINOR: u32 = 0;
pub const VERSION_PATCH: u32 = 2;

/// Simple test function to verify FFI integration
/// This function serves as a proof-of-concept for C++ integration
#[no_mangle]
pub extern "C" fn urdf_rust_test_function() -> c_int {
    // Return a known value that C++ can verify
    42
}

/// Get the version string of the urdf_rust library
#[no_mangle]
pub extern "C" fn urdf_rust_get_version() -> *const c_char {
    static VERSION_CSTR: &str = concat!(env!("CARGO_PKG_VERSION"), "\0");
    VERSION_CSTR.as_ptr() as *const c_char
}

/// Initialize the Rust library (for future use)
/// Returns 0 on success, non-zero on error
#[no_mangle]
pub extern "C" fn urdf_rust_init() -> c_int {
    // Initialize logging, allocators, or other global state if needed
    // For now, just return success
    0
}

/// Cleanup the Rust library (for future use)
#[no_mangle]
pub extern "C" fn urdf_rust_cleanup() -> c_int {
    // Cleanup global state if needed
    // For now, just return success
    0
}

/// Parse a URDF string and return a simple validation result
/// This is a placeholder for the full parsing functionality
#[no_mangle]
pub extern "C" fn urdf_rust_parse_string_simple(urdf_content: *const c_char) -> c_int {
    if urdf_content.is_null() {
        return -1; // Error: null pointer
    }
    
    let c_str = unsafe { CStr::from_ptr(urdf_content) };
    let urdf_str = match c_str.to_str() {
        Ok(s) => s,
        Err(_) => return -2, // Error: invalid UTF-8
    };
    
    // Simple validation: check if it contains basic URDF elements
    if urdf_str.contains("<robot") && urdf_str.contains("</robot>") {
        1 // Success: basic structure found
    } else {
        -3 // Error: invalid URDF structure
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::ffi::CString;

    #[test]
    fn test_version_constants() {
        assert_eq!(VERSION_MAJOR, 5);
        assert_eq!(VERSION_MINOR, 0);
        assert_eq!(VERSION_PATCH, 2);
        assert_eq!(VERSION, "5.0.2");
    }

    #[test]
    fn test_ffi_test_function() {
        assert_eq!(urdf_rust_test_function(), 42);
    }

    #[test]
    fn test_init_cleanup() {
        assert_eq!(urdf_rust_init(), 0);
        assert_eq!(urdf_rust_cleanup(), 0);
    }

    #[test]
    fn test_parse_string_simple() {
        let valid_urdf = CString::new(r#"<?xml version="1.0"?>
<robot name="test">
  <link name="base_link"/>
</robot>"#).unwrap();
        
        assert_eq!(urdf_rust_parse_string_simple(valid_urdf.as_ptr()), 1);
        
        let invalid_urdf = CString::new("not a urdf").unwrap();
        assert_eq!(urdf_rust_parse_string_simple(invalid_urdf.as_ptr()), -3);
        
        assert_eq!(urdf_rust_parse_string_simple(std::ptr::null()), -1);
    }

    #[test]
    fn test_version_string() {
        let version_ptr = urdf_rust_get_version();
        let version_cstr = unsafe { CStr::from_ptr(version_ptr) };
        let version_str = version_cstr.to_str().unwrap();
        assert_eq!(version_str, VERSION);
    }
}

/// Integration tests that can be called from C++
#[cfg(test)]
mod integration_tests {
    use super::*;

    #[test]
    fn test_cpp_integration_workflow() {
        // Simulate the workflow that C++ code would follow
        
        // 1. Initialize
        assert_eq!(urdf_rust_init(), 0);
        
        // 2. Test basic functionality
        assert_eq!(urdf_rust_test_function(), 42);
        
        // 3. Check version
        let version_ptr = urdf_rust_get_version();
        assert!(!version_ptr.is_null());
        
        // 4. Test parsing
        let test_urdf = std::ffi::CString::new(r#"
<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link">
    <visual>
      <geometry>
        <box size="1 1 1"/>
      </geometry>
    </visual>
  </link>
</robot>"#).unwrap();
        
        assert_eq!(urdf_rust_parse_string_simple(test_urdf.as_ptr()), 1);
        
        // 5. Cleanup
        assert_eq!(urdf_rust_cleanup(), 0);
    }
}