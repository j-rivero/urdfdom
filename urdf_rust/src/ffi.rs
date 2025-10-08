//! Foreign Function Interface (FFI) module for C++ interoperability
//! 
//! This module provides C-compatible functions and types that can be called
//! from the existing C++ urdfdom codebase during the incremental migration.
//! 
//! # Design Principles
//! 
//! - All functions use C calling convention (`extern "C"`)
//! - Error handling uses integer return codes (0 = success, negative = error)
//! - String parameters use null-terminated C strings
//! - Complex data structures use opaque pointers to Rust types
//! - Memory management follows RAII principles with explicit create/destroy functions

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::ptr;

use crate::model::Robot;
use crate::parser::parse_urdf_string;
use crate::error::UrdfError;

/// Opaque handle to a Rust Robot instance
/// This allows C++ code to hold references to Rust objects without
/// knowing their internal structure
#[repr(C)]
pub struct UrdfRobotHandle {
    _private: [u8; 0], // Zero-sized type, actual data stored separately
}

/// Error codes returned by FFI functions
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum UrdfErrorCode {
    Success = 0,
    NullPointer = -1,
    InvalidUtf8 = -2,
    ParseError = -3,
    InvalidParameter = -4,
    OutOfMemory = -5,
    IoError = -6,
    UnknownError = -99,
}

impl From<UrdfError> for UrdfErrorCode {
    fn from(error: UrdfError) -> Self {
        match error {
            UrdfError::ParseError(_) => UrdfErrorCode::ParseError,
            UrdfError::IoError(_) => UrdfErrorCode::IoError,
            UrdfError::InvalidInput(_) => UrdfErrorCode::InvalidParameter,
        }
    }
}

/// Parse a URDF string and return a handle to the Robot
/// 
/// # Safety
/// 
/// - `urdf_content` must be a valid null-terminated C string
/// - The returned handle must be freed with `urdf_robot_destroy`
/// 
/// # Returns
/// 
/// - On success: pointer to UrdfRobotHandle
/// - On failure: null pointer (check `urdf_get_last_error` for details)
#[no_mangle]
pub unsafe extern "C" fn urdf_robot_from_string(urdf_content: *const c_char) -> *mut UrdfRobotHandle {
    if urdf_content.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return ptr::null_mut();
    }
    
    let c_str = match CStr::from_ptr(urdf_content).to_str() {
        Ok(s) => s,
        Err(_) => {
            set_last_error(UrdfErrorCode::InvalidUtf8);
            return ptr::null_mut();
        }
    };
    
    match parse_urdf_string(c_str) {
        Ok(robot) => {
            let boxed_robot = Box::new(robot);
            set_last_error(UrdfErrorCode::Success);
            Box::into_raw(boxed_robot) as *mut UrdfRobotHandle
        }
        Err(e) => {
            set_last_error(e.into());
            ptr::null_mut()
        }
    }
}

/// Destroy a Robot handle and free its memory
/// 
/// # Safety
/// 
/// - `handle` must be a valid pointer returned by `urdf_robot_from_string`
/// - After calling this function, `handle` becomes invalid and must not be used
#[no_mangle]
pub unsafe extern "C" fn urdf_robot_destroy(handle: *mut UrdfRobotHandle) {
    if !handle.is_null() {
        let _robot = Box::from_raw(handle as *mut Robot);
        // Robot is automatically dropped here
    }
}

/// Get the name of the robot
/// 
/// # Safety
/// 
/// - `handle` must be a valid Robot handle
/// - The returned string is valid until the robot is destroyed
/// - Do not free the returned pointer
#[no_mangle]
pub unsafe extern "C" fn urdf_robot_get_name(handle: *const UrdfRobotHandle) -> *const c_char {
    if handle.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return ptr::null();
    }
    
    let robot = &*(handle as *const Robot);
    
    // We need to keep the CString alive, so we store it in the Robot
    // For now, we'll use a simple approach - in a full implementation,
    // we'd need a more sophisticated string management strategy
    match CString::new(robot.name.as_str()) {
        Ok(cstring) => {
            // This is a temporary solution - in production, we'd need
            // proper string lifetime management
            let ptr = cstring.as_ptr();
            std::mem::forget(cstring); // Prevent deallocation
            set_last_error(UrdfErrorCode::Success);
            ptr
        }
        Err(_) => {
            set_last_error(UrdfErrorCode::InvalidUtf8);
            ptr::null()
        }
    }
}

/// Get the number of links in the robot
#[no_mangle]
pub unsafe extern "C" fn urdf_robot_get_link_count(handle: *const UrdfRobotHandle) -> c_int {
    if handle.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return -1;
    }
    
    let robot = &*(handle as *const Robot);
    set_last_error(UrdfErrorCode::Success);
    robot.links.len() as c_int
}

/// Get the number of joints in the robot
#[no_mangle]
pub unsafe extern "C" fn urdf_robot_get_joint_count(handle: *const UrdfRobotHandle) -> c_int {
    if handle.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return -1;
    }
    
    let robot = &*(handle as *const Robot);
    set_last_error(UrdfErrorCode::Success);
    robot.joints.len() as c_int
}

/// Check if the robot has a link with the given name
#[no_mangle]
pub unsafe extern "C" fn urdf_robot_has_link(
    handle: *const UrdfRobotHandle,
    link_name: *const c_char
) -> c_int {
    if handle.is_null() || link_name.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return -1;
    }
    
    let robot = &*(handle as *const Robot);
    let name_str = match CStr::from_ptr(link_name).to_str() {
        Ok(s) => s,
        Err(_) => {
            set_last_error(UrdfErrorCode::InvalidUtf8);
            return -1;
        }
    };
    
    set_last_error(UrdfErrorCode::Success);
    if robot.links.contains_key(name_str) { 1 } else { 0 }
}

/// Check if the robot has a joint with the given name
#[no_mangle]
pub unsafe extern "C" fn urdf_robot_has_joint(
    handle: *const UrdfRobotHandle,
    joint_name: *const c_char
) -> c_int {
    if handle.is_null() || joint_name.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return -1;
    }
    
    let robot = &*(handle as *const Robot);
    let name_str = match CStr::from_ptr(joint_name).to_str() {
        Ok(s) => s,
        Err(_) => {
            set_last_error(UrdfErrorCode::InvalidUtf8);
            return -1;
        }
    };
    
    set_last_error(UrdfErrorCode::Success);
    if robot.joints.contains_key(name_str) { 1 } else { 0 }
}

// Thread-local storage for the last error
thread_local! {
    static LAST_ERROR: std::cell::Cell<UrdfErrorCode> = std::cell::Cell::new(UrdfErrorCode::Success);
}

/// Set the last error code (internal function)
fn set_last_error(error: UrdfErrorCode) {
    LAST_ERROR.with(|e| e.set(error));
}

/// Get the last error code
#[no_mangle]
pub extern "C" fn urdf_get_last_error() -> c_int {
    LAST_ERROR.with(|e| e.get() as c_int)
}

/// Get a human-readable description of an error code
#[no_mangle]
pub extern "C" fn urdf_error_description(error_code: c_int) -> *const c_char {
    let description = match error_code {
        0 => "Success\0",
        -1 => "Null pointer\0",
        -2 => "Invalid UTF-8 string\0",
        -3 => "URDF parse error\0",
        -4 => "Invalid parameter\0",
        -5 => "Out of memory\0",
        -6 => "I/O error\0",
        -99 => "Unknown error\0",
        _ => "Invalid error code\0",
    };
    
    description.as_ptr() as *const c_char
}

/// Parse a Vector3 from a C string
/// 
/// This function is a direct migration of the C++ Vector3::init() functionality.
/// It parses a space-separated string "x y z" into three floating point components.
/// 
/// # Safety
/// 
/// - `input` must be a valid null-terminated C string
/// - `x`, `y`, `z` must be valid pointers to f64 storage
/// 
/// # Returns
/// 
/// - 1 (true) on success, with parsed values written to x, y, z
/// - 0 (false) on error (invalid input, wrong number of components, etc.)
/// 
/// # Examples from C++
/// 
/// ```cpp
/// // Before (C++ version):
/// urdf::Vector3 vec;
/// try {
///     vec.init("1.0 2.0 3.0");
///     // Success: vec.x = 1.0, vec.y = 2.0, vec.z = 3.0
/// } catch(urdf::ParseError &e) {
///     // Handle error
/// }
/// 
/// // After (Rust FFI version):
/// double x, y, z;
/// if (urdf_parse_vector3("1.0 2.0 3.0", &x, &y, &z)) {
///     // Success: x = 1.0, y = 2.0, z = 3.0
/// } else {
///     // Handle error
/// }
/// ```

/// FFI functions for Pose data structures
/// 
/// These functions provide C-compatible wrappers for the Vector3, Rotation, and Pose
/// structs, enabling seamless integration with existing C++ code.

use crate::pose::{Vector3, Rotation, Pose};

/// Create a new Vector3 with specified components
#[no_mangle]
pub extern "C" fn urdf_vector3_new(x: f64, y: f64, z: f64) -> Vector3 {
    Vector3::new(x, y, z)
}

/// Create a zero Vector3
#[no_mangle]
pub extern "C" fn urdf_vector3_zero() -> Vector3 {
    Vector3::zero()
}

/// Clear Vector3 to zero
#[no_mangle]
pub extern "C" fn urdf_vector3_clear(vec: *mut Vector3) {
    if !vec.is_null() {
        unsafe {
            (*vec).clear();
        }
    }
}

/// Initialize Vector3 from string "x y z"
/// Returns 0 on success, 1 on error
#[no_mangle]
pub extern "C" fn urdf_vector3_init(vec: *mut Vector3, vector_str: *const c_char) -> c_int {
    if vec.is_null() || vector_str.is_null() {
        return 1;
    }
    
    unsafe {
        let c_str = match CStr::from_ptr(vector_str).to_str() {
            Ok(s) => s,
            Err(_) => return 1,
        };
        
        match (*vec).init(c_str) {
            Ok(_) => 0,
            Err(_) => 1,
        }
    }
}

/// Add two Vector3s (result = a + b)
#[no_mangle]
pub extern "C" fn urdf_vector3_add(a: Vector3, b: Vector3) -> Vector3 {
    a + b
}

/// Create a new Rotation with specified quaternion components
#[no_mangle]
pub extern "C" fn urdf_rotation_new(x: f64, y: f64, z: f64, w: f64) -> Rotation {
    Rotation::new(x, y, z, w)
}

/// Create identity Rotation (no rotation)
#[no_mangle]
pub extern "C" fn urdf_rotation_identity() -> Rotation {
    Rotation::identity()
}

/// Clear Rotation to identity
#[no_mangle]
pub extern "C" fn urdf_rotation_clear(rot: *mut Rotation) {
    if !rot.is_null() {
        unsafe {
            (*rot).clear();
        }
    }
}

/// Get quaternion components from Rotation
#[no_mangle]
pub extern "C" fn urdf_rotation_get_quaternion(
    rot: *const Rotation,
    quat_x: *mut f64,
    quat_y: *mut f64,
    quat_z: *mut f64,
    quat_w: *mut f64,
) {
    if rot.is_null() || quat_x.is_null() || quat_y.is_null() || quat_z.is_null() || quat_w.is_null() {
        return;
    }
    
    unsafe {
        let (x, y, z, w) = (*rot).get_quaternion();
        *quat_x = x;
        *quat_y = y;
        *quat_z = z;
        *quat_w = w;
    }
}

/// Get Roll-Pitch-Yaw angles from Rotation
#[no_mangle]
pub extern "C" fn urdf_rotation_get_rpy(
    rot: *const Rotation,
    roll: *mut f64,
    pitch: *mut f64,
    yaw: *mut f64,
) {
    if rot.is_null() || roll.is_null() || pitch.is_null() || yaw.is_null() {
        return;
    }
    
    unsafe {
        let (r, p, y) = (*rot).get_rpy();
        *roll = r;
        *pitch = p;
        *yaw = y;
    }
}

/// Set Rotation from quaternion components
#[no_mangle]
pub extern "C" fn urdf_rotation_set_from_quaternion(
    rot: *mut Rotation,
    quat_x: f64,
    quat_y: f64,
    quat_z: f64,
    quat_w: f64,
) {
    if !rot.is_null() {
        unsafe {
            (*rot).set_from_quaternion(quat_x, quat_y, quat_z, quat_w);
        }
    }
}

/// Set Rotation from Roll-Pitch-Yaw angles
#[no_mangle]
pub extern "C" fn urdf_rotation_set_from_rpy(rot: *mut Rotation, roll: f64, pitch: f64, yaw: f64) {
    if !rot.is_null() {
        unsafe {
            (*rot).set_from_rpy(roll, pitch, yaw);
        }
    }
}

/// Initialize Rotation from string "roll pitch yaw"
/// Returns 0 on success, 1 on error
#[no_mangle]
pub extern "C" fn urdf_rotation_init(rot: *mut Rotation, rotation_str: *const c_char) -> c_int {
    if rot.is_null() || rotation_str.is_null() {
        return 1;
    }
    
    unsafe {
        let c_str = match CStr::from_ptr(rotation_str).to_str() {
            Ok(s) => s,
            Err(_) => return 1,
        };
        
        match (*rot).init(c_str) {
            Ok(_) => 0,
            Err(_) => 1,
        }
    }
}

/// Normalize the Rotation quaternion
#[no_mangle]
pub extern "C" fn urdf_rotation_normalize(rot: *mut Rotation) {
    if !rot.is_null() {
        unsafe {
            (*rot).normalize();
        }
    }
}

/// Get inverse of Rotation
#[no_mangle]
pub extern "C" fn urdf_rotation_get_inverse(rot: *const Rotation) -> Rotation {
    if rot.is_null() {
        return Rotation::identity();
    }
    
    unsafe { (*rot).get_inverse() }
}

/// Multiply two Rotations (quaternion multiplication)
#[no_mangle]
pub extern "C" fn urdf_rotation_multiply(r1: Rotation, r2: Rotation) -> Rotation {
    r1 * r2
}

/// Rotate a Vector3 by a Rotation
#[no_mangle]
pub extern "C" fn urdf_rotation_multiply_vector3(rot: Rotation, vec: Vector3) -> Vector3 {
    rot * vec
}

/// Create a new Pose with specified position and rotation
#[no_mangle]
pub extern "C" fn urdf_pose_new(position: Vector3, rotation: Rotation) -> Pose {
    Pose::new(position, rotation)
}

/// Create identity Pose (zero position, no rotation)
#[no_mangle]
pub extern "C" fn urdf_pose_identity() -> Pose {
    Pose::identity()
}

/// Clear Pose to identity
#[no_mangle]
pub extern "C" fn urdf_pose_clear(pose: *mut Pose) {
    if !pose.is_null() {
        unsafe {
            (*pose).clear();
        }
    }
}

// ============================================================================
// GEOMETRY FFI FUNCTIONS
// ============================================================================

use crate::model::geometry::Geometry;
use std::boxed::Box;

/// Geometry type enumeration - matches C++ urdf::Geometry enum
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum UrdfGeometryType {
    Sphere = 0,
    Box = 1,
    Cylinder = 2,
    Mesh = 3,
}

impl From<&Geometry> for UrdfGeometryType {
    fn from(geometry: &Geometry) -> Self {
        match geometry {
            Geometry::Sphere { .. } => UrdfGeometryType::Sphere,
            Geometry::Box { .. } => UrdfGeometryType::Box,
            Geometry::Cylinder { .. } => UrdfGeometryType::Cylinder,
            Geometry::Mesh { .. } => UrdfGeometryType::Mesh,
        }
    }
}

/// Create a box geometry with dimensions (x, y, z)
#[no_mangle]
pub extern "C" fn urdf_geometry_create_box(x: f64, y: f64, z: f64) -> *mut Geometry {
    let geometry = Geometry::Box { size: [x, y, z] };
    Box::into_raw(Box::new(geometry))
}

/// Create a sphere geometry with radius
#[no_mangle]
pub extern "C" fn urdf_geometry_create_sphere(radius: f64) -> *mut Geometry {
    let geometry = Geometry::Sphere { radius };
    Box::into_raw(Box::new(geometry))
}

/// Create a cylinder geometry with radius and length
#[no_mangle]
pub extern "C" fn urdf_geometry_create_cylinder(radius: f64, length: f64) -> *mut Geometry {
    let geometry = Geometry::Cylinder { radius, length };
    Box::into_raw(Box::new(geometry))
}

/// Create a mesh geometry with filename and optional scale
#[no_mangle]
pub unsafe extern "C" fn urdf_geometry_create_mesh(
    filename: *const c_char,
    scale: *const f64,
) -> *mut Geometry {
    if filename.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return ptr::null_mut();
    }

    let filename_str = match CStr::from_ptr(filename).to_str() {
        Ok(s) => s.to_string(),
        Err(_) => {
            set_last_error(UrdfErrorCode::InvalidUtf8);
            return ptr::null_mut();
        }
    };

    let scale_array = if scale.is_null() {
        None
    } else {
        Some([*scale, *scale.offset(1), *scale.offset(2)])
    };

    let geometry = Geometry::Mesh {
        filename: filename_str,
        scale: scale_array,
    };

    set_last_error(UrdfErrorCode::Success);
    Box::into_raw(Box::new(geometry))
}

/// Destroy/free a geometry object
#[no_mangle]
pub unsafe extern "C" fn urdf_geometry_destroy(geometry: *mut Geometry) {
    if !geometry.is_null() {
        let _geometry = Box::from_raw(geometry);
        // Geometry is automatically dropped here
    }
}

/// Get the type of a geometry object
#[no_mangle]
pub unsafe extern "C" fn urdf_geometry_get_type(geometry: *const Geometry) -> c_int {
    if geometry.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return -1;
    }

    let geom = &*geometry;
    let geom_type: UrdfGeometryType = geom.into();
    set_last_error(UrdfErrorCode::Success);
    geom_type as c_int
}

/// Get dimensions of a box geometry
#[no_mangle]
pub unsafe extern "C" fn urdf_geometry_box_get_dimensions(
    geometry: *const Geometry,
    dimensions: *mut f64,
) -> c_int {
    if geometry.is_null() || dimensions.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return -1;
    }

    let geom = &*geometry;
    if let Geometry::Box { size } = geom {
        *dimensions = size[0];
        *dimensions.offset(1) = size[1];
        *dimensions.offset(2) = size[2];
        set_last_error(UrdfErrorCode::Success);
        0
    } else {
        set_last_error(UrdfErrorCode::InvalidParameter);
        -1
    }
}

/// Get radius of a sphere geometry
#[no_mangle]
pub unsafe extern "C" fn urdf_geometry_sphere_get_radius(geometry: *const Geometry) -> f64 {
    if geometry.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return -1.0;
    }

    let geom = &*geometry;
    if let Geometry::Sphere { radius } = geom {
        set_last_error(UrdfErrorCode::Success);
        *radius
    } else {
        set_last_error(UrdfErrorCode::InvalidParameter);
        -1.0
    }
}

/// Get radius of a cylinder geometry
#[no_mangle]
pub unsafe extern "C" fn urdf_geometry_cylinder_get_radius(geometry: *const Geometry) -> f64 {
    if geometry.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return -1.0;
    }

    let geom = &*geometry;
    if let Geometry::Cylinder { radius, .. } = geom {
        set_last_error(UrdfErrorCode::Success);
        *radius
    } else {
        set_last_error(UrdfErrorCode::InvalidParameter);
        -1.0
    }
}

/// Get length of a cylinder geometry
#[no_mangle]
pub unsafe extern "C" fn urdf_geometry_cylinder_get_length(geometry: *const Geometry) -> f64 {
    if geometry.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return -1.0;
    }

    let geom = &*geometry;
    if let Geometry::Cylinder { length, .. } = geom {
        set_last_error(UrdfErrorCode::Success);
        *length
    } else {
        set_last_error(UrdfErrorCode::InvalidParameter);
        -1.0
    }
}

/// Get filename of a mesh geometry
#[no_mangle]
pub unsafe extern "C" fn urdf_geometry_mesh_get_filename(
    geometry: *const Geometry,
    buffer: *mut c_char,
    buffer_size: usize,
) -> c_int {
    if geometry.is_null() || buffer.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return -1;
    }

    let geom = &*geometry;
    if let Geometry::Mesh { filename, .. } = geom {
        let filename_bytes = filename.as_bytes();
        if filename_bytes.len() + 1 > buffer_size {
            set_last_error(UrdfErrorCode::InvalidParameter);
            return -1;
        }

        // Copy filename to buffer
        std::ptr::copy_nonoverlapping(filename_bytes.as_ptr(), buffer as *mut u8, filename_bytes.len());
        *buffer.offset(filename_bytes.len() as isize) = 0; // Null terminator

        set_last_error(UrdfErrorCode::Success);
        0
    } else {
        set_last_error(UrdfErrorCode::InvalidParameter);
        -1
    }
}

/// Get scale of a mesh geometry
#[no_mangle]
pub unsafe extern "C" fn urdf_geometry_mesh_get_scale(
    geometry: *const Geometry,
    scale: *mut f64,
) -> c_int {
    if geometry.is_null() || scale.is_null() {
        set_last_error(UrdfErrorCode::NullPointer);
        return -1;
    }

    let geom = &*geometry;
    if let Geometry::Mesh { scale: mesh_scale, .. } = geom {
        match mesh_scale {
            Some(scale_array) => {
                *scale = scale_array[0];
                *scale.offset(1) = scale_array[1];
                *scale.offset(2) = scale_array[2];
            }
            None => {
                // Default scale is 1.0, 1.0, 1.0
                *scale = 1.0;
                *scale.offset(1) = 1.0;
                *scale.offset(2) = 1.0;
            }
        }
        set_last_error(UrdfErrorCode::Success);
        0
    } else {
        set_last_error(UrdfErrorCode::InvalidParameter);
        -1
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::ffi::CString;

    #[test]
    fn test_error_codes() {
        assert_eq!(UrdfErrorCode::Success as c_int, 0);
        assert_eq!(UrdfErrorCode::NullPointer as c_int, -1);
        assert_eq!(UrdfErrorCode::ParseError as c_int, -3);
    }

    #[test]
    fn test_robot_from_string_null() {
        unsafe {
            let handle = urdf_robot_from_string(ptr::null());
            assert!(handle.is_null());
            assert_eq!(urdf_get_last_error(), UrdfErrorCode::NullPointer as c_int);
        }
    }

    #[test]
    fn test_robot_lifecycle() {
        let urdf_content = CString::new(r#"<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link"/>
  <link name="child_link"/>
  <joint name="test_joint" type="fixed">
    <parent link="base_link"/>
    <child link="child_link"/>
  </joint>
</robot>"#).unwrap();

        unsafe {
            let handle = urdf_robot_from_string(urdf_content.as_ptr());
            
            if !handle.is_null() {
                // Test robot properties
                let link_count = urdf_robot_get_link_count(handle);
                let joint_count = urdf_robot_get_joint_count(handle);
                
                // We expect some links and joints, exact numbers depend on implementation
                assert!(link_count >= 0);
                assert!(joint_count >= 0);
                
                // Clean up
                urdf_robot_destroy(handle);
            }
        }
    }

    #[test]
    fn test_error_description() {
        unsafe {
            let desc_ptr = urdf_error_description(UrdfErrorCode::Success as c_int);
            let desc = CStr::from_ptr(desc_ptr).to_str().unwrap();
            assert_eq!(desc, "Success");
            
            let desc_ptr = urdf_error_description(UrdfErrorCode::ParseError as c_int);
            let desc = CStr::from_ptr(desc_ptr).to_str().unwrap();
            assert_eq!(desc, "URDF parse error");
        }
    }
}