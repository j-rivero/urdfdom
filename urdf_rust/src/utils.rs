//! Utility functions for URDF parsing
//! 
//! This module contains utility functions that were migrated from the C++ urdfdom
//! implementation, maintaining identical behavior and error handling.

use std::ffi::{CStr, c_char};
use std::os::raw::c_int;
use crate::error::{UrdfError, UrdfResult};

/// Parse a Vector3 from a space-separated string
/// 
/// This function parses a string in the format "x y z" where x, y, z are floating point numbers.
/// It handles whitespace variations and returns errors for invalid input, matching the behavior
/// of the original C++ Vector3::init() function.
/// 
/// # Arguments
/// * `vector_str` - A string containing three space-separated floating point numbers
/// 
/// # Returns
/// * `Ok((x, y, z))` - Tuple containing the parsed x, y, z values
/// * `Err(UrdfError)` - Parse error with detailed message
/// 
/// # Examples
/// ```rust
/// use urdf_rust::parse_vector3;
/// 
/// // Valid input
/// let (x, y, z) = parse_vector3("1.0 2.0 3.0").unwrap();
/// assert_eq!(x, 1.0);
/// assert_eq!(y, 2.0);
/// assert_eq!(z, 3.0);
/// 
/// // Handles extra whitespace
/// let (x, y, z) = parse_vector3("  1.5   -2.7   3.14  ").unwrap();
/// assert_eq!(x, 1.5);
/// assert_eq!(y, -2.7);
/// assert_eq!(z, 3.14);
/// 
/// // Error cases
/// assert!(parse_vector3("1.0 2.0").is_err());        // Not enough numbers
/// assert!(parse_vector3("1.0 2.0 3.0 4.0").is_err()); // Too many numbers
/// assert!(parse_vector3("1.0 foo 3.0").is_err());     // Invalid number
/// ```
pub fn parse_vector3(vector_str: &str) -> UrdfResult<(f64, f64, f64)> {
    // Split string by whitespace and filter out empty strings
    let pieces: Vec<&str> = vector_str
        .split_whitespace()
        .filter(|s| !s.is_empty())
        .collect();
    
    // Parse each piece to a double
    let mut xyz = Vec::new();
    for (i, piece) in pieces.iter().enumerate() {
        match piece.parse::<f64>() {
            Ok(value) => xyz.push(value),
            Err(_) => {
                return Err(UrdfError::ParseError(format!(
                    "Unable to parse component [{}] to a double (while parsing a vector value)",
                    piece
                )));
            }
        }
    }
    
    // Check that we have exactly 3 components
    if xyz.len() != 3 {
        return Err(UrdfError::ParseError(format!(
            "Parser found {} elements but 3 expected while parsing vector [{}]",
            xyz.len(),
            vector_str
        )));
    }
    
    Ok((xyz[0], xyz[1], xyz[2]))
}

/// Parse a Vector3 from a C string (FFI version)
/// 
/// This is the C FFI wrapper for parse_vector3, designed to be called from C++ code.
/// Returns true on success, false on error. On success, the parsed values are written
/// to the provided output parameters.
/// 
/// # Safety
/// * `input` must be a valid null-terminated C string
/// * `x`, `y`, `z` must be valid pointers to f64 storage
/// 
/// # Arguments
/// * `input` - Null-terminated C string containing "x y z" format
/// * `x` - Output parameter for x component
/// * `y` - Output parameter for y component  
/// * `z` - Output parameter for z component
/// 
/// # Returns
/// * `1` (true) on success
/// * `0` (false) on error
#[no_mangle]
pub unsafe extern "C" fn urdf_parse_vector3(
    input: *const c_char,
    x: *mut f64,
    y: *mut f64,
    z: *mut f64,
) -> c_int {
    // Validate input pointers
    if input.is_null() || x.is_null() || y.is_null() || z.is_null() {
        return 0; // false
    }
    
    // Convert C string to Rust string
    let c_str = match CStr::from_ptr(input).to_str() {
        Ok(s) => s,
        Err(_) => return 0, // Invalid UTF-8
    };
    
    // Parse the vector
    match parse_vector3(c_str) {
        Ok((px, py, pz)) => {
            // Write results to output parameters
            *x = px;
            *y = py;
            *z = pz;
            1 // true
        }
        Err(_) => 0, // false
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::ffi::CString;

    #[test]
    fn test_vector3_simple() {
        let (x, y, z) = parse_vector3("1.0 2.0 3.0").unwrap();
        assert_eq!(x, 1.0);
        assert_eq!(y, 2.0);
        assert_eq!(z, 3.0);
    }

    #[test]
    fn test_vector3_float() {
        let (x, y, z) = parse_vector3("0.1 0.2 0.3").unwrap();
        assert_eq!(x, 0.1);
        assert_eq!(y, 0.2);
        assert_eq!(z, 0.3);
    }

    #[test]
    fn test_vector3_negative_values() {
        let (x, y, z) = parse_vector3("-1.5 2.7 -3.14").unwrap();
        assert_eq!(x, -1.5);
        assert_eq!(y, 2.7);
        assert_eq!(z, -3.14);
    }

    #[test]
    fn test_vector3_scientific_notation() {
        let (x, y, z) = parse_vector3("1e-3 2.5e2 -1.23e+1").unwrap();
        assert_eq!(x, 0.001);
        assert_eq!(y, 250.0);
        assert_eq!(z, -12.3);
    }

    #[test]
    fn test_vector3_extra_whitespace() {
        let (x, y, z) = parse_vector3("  1.5   -2.7   3.14  ").unwrap();
        assert_eq!(x, 1.5);
        assert_eq!(y, -2.7);
        assert_eq!(z, 3.14);
    }

    #[test]
    fn test_vector3_tabs_and_newlines() {
        let (x, y, z) = parse_vector3("1.0\t2.0\n3.0").unwrap();
        assert_eq!(x, 1.0);
        assert_eq!(y, 2.0);
        assert_eq!(z, 3.0);
    }

    #[test]
    fn test_vector3_bad_string() {
        let result = parse_vector3("1.0 foo 3.0");
        assert!(result.is_err());
        if let Err(UrdfError::ParseError(msg)) = result {
            assert!(msg.contains("Unable to parse component [foo] to a double"));
        }
    }

    #[test]
    fn test_vector3_invalid_number() {
        let result = parse_vector3("1.0 2.10.110 3.0");
        assert!(result.is_err());
        if let Err(UrdfError::ParseError(msg)) = result {
            assert!(msg.contains("Unable to parse component [2.10.110] to a double"));
        }
    }

    #[test]
    fn test_vector3_not_enough_numbers() {
        let result = parse_vector3("1.0 2.0");
        assert!(result.is_err());
        if let Err(UrdfError::ParseError(msg)) = result {
            assert!(msg.contains("Parser found 2 elements but 3 expected"));
        }
    }

    #[test]
    fn test_vector3_too_many_numbers() {
        let result = parse_vector3("1.0 2.0 3.0 4.0");
        assert!(result.is_err());
        if let Err(UrdfError::ParseError(msg)) = result {
            assert!(msg.contains("Parser found 4 elements but 3 expected"));
        }
    }

    #[test] 
    fn test_vector3_empty_string() {
        let result = parse_vector3("");
        assert!(result.is_err());
        if let Err(UrdfError::ParseError(msg)) = result {
            assert!(msg.contains("Parser found 0 elements but 3 expected"));
        }
    }

    #[test]
    fn test_vector3_only_whitespace() {
        let result = parse_vector3("   \t\n  ");
        assert!(result.is_err());
        if let Err(UrdfError::ParseError(msg)) = result {
            assert!(msg.contains("Parser found 0 elements but 3 expected"));
        }
    }

    // FFI Tests
    #[test]
    fn test_ffi_vector3_simple() {
        let input = CString::new("1.0 2.0 3.0").unwrap();
        let mut x = 0.0;
        let mut y = 0.0;
        let mut z = 0.0;
        
        let result = unsafe {
            urdf_parse_vector3(input.as_ptr(), &mut x, &mut y, &mut z)
        };
        
        assert_eq!(result, 1); // true
        assert_eq!(x, 1.0);
        assert_eq!(y, 2.0);
        assert_eq!(z, 3.0);
    }

    #[test]
    fn test_ffi_vector3_error() {
        let input = CString::new("1.0 foo 3.0").unwrap();
        let mut x = 0.0;
        let mut y = 0.0;
        let mut z = 0.0;
        
        let result = unsafe {
            urdf_parse_vector3(input.as_ptr(), &mut x, &mut y, &mut z)
        };
        
        assert_eq!(result, 0); // false
    }

    #[test]
    fn test_ffi_null_pointers() {
        let result = unsafe {
            urdf_parse_vector3(std::ptr::null(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
        };
        assert_eq!(result, 0); // false
    }
}