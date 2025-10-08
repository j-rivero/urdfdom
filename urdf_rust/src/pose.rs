//! Pose data structures for URDF
//! 
//! This module provides Rust implementations of the Vector3, Rotation, and Pose
//! classes from the C++ urdfdom library, maintaining identical behavior and API.

use std::f64::consts::PI;
use crate::error::UrdfResult;
use crate::utils::parse_vector3;

/// 3D Vector representation
/// 
/// Equivalent to urdf::Vector3 from the C++ implementation.
/// Uses C-compatible memory layout for FFI integration.
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Vector3 {
    pub x: f64,
    pub y: f64,
    pub z: f64,
}

impl Vector3 {
    /// Create new Vector3 with specified components
    pub fn new(x: f64, y: f64, z: f64) -> Self {
        Vector3 { x, y, z }
    }
    
    /// Create new Vector3 with all components set to zero
    pub fn zero() -> Self {
        Vector3 { x: 0.0, y: 0.0, z: 0.0 }
    }
    
    /// Clear all components to zero
    pub fn clear(&mut self) {
        self.x = 0.0;
        self.y = 0.0;
        self.z = 0.0;
    }
    
    /// Initialize from string representation "x y z"
    /// 
    /// Uses the existing parse_vector3 utility function to maintain
    /// identical behavior to the C++ Vector3::init() method.
    pub fn init(&mut self, vector_str: &str) -> UrdfResult<()> {
        let (x, y, z) = parse_vector3(vector_str)?;
        self.x = x;
        self.y = y;
        self.z = z;
        Ok(())
    }
    
    /// Create Vector3 from string representation
    pub fn from_string(vector_str: &str) -> UrdfResult<Self> {
        let (x, y, z) = parse_vector3(vector_str)?;
        Ok(Vector3::new(x, y, z))
    }
}

impl Default for Vector3 {
    fn default() -> Self {
        Vector3::zero()
    }
}

impl std::ops::Add for Vector3 {
    type Output = Vector3;
    
    fn add(self, other: Vector3) -> Vector3 {
        Vector3::new(self.x + other.x, self.y + other.y, self.z + other.z)
    }
}

/// Quaternion-based rotation representation
/// 
/// Equivalent to urdf::Rotation from the C++ implementation.
/// Uses C-compatible memory layout for FFI integration.
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Rotation {
    pub x: f64,  // Quaternion i component
    pub y: f64,  // Quaternion j component  
    pub z: f64,  // Quaternion k component
    pub w: f64,  // Quaternion scalar component
}

impl Rotation {
    /// Create new Rotation with specified quaternion components
    pub fn new(x: f64, y: f64, z: f64, w: f64) -> Self {
        let mut rotation = Rotation { x, y, z, w };
        rotation.normalize();
        rotation
    }
    
    /// Create identity rotation (no rotation)
    pub fn identity() -> Self {
        Rotation { x: 0.0, y: 0.0, z: 0.0, w: 1.0 }
    }
    
    /// Clear to identity rotation
    pub fn clear(&mut self) {
        self.x = 0.0;
        self.y = 0.0;
        self.z = 0.0;
        self.w = 1.0;
    }
    
    /// Get quaternion components
    pub fn get_quaternion(&self) -> (f64, f64, f64, f64) {
        (self.x, self.y, self.z, self.w)
    }
    
    /// Get Roll-Pitch-Yaw angles from quaternion
    /// 
    /// Implements the exact algorithm from the C++ version, including
    /// gimbal lock handling and edge cases.
    pub fn get_rpy(&self) -> (f64, f64, f64) {
        let sqw = self.w * self.w;
        let sqx = self.x * self.x;
        let sqy = self.y * self.y;
        let sqz = self.z * self.z;
        
        // Cases derived from https://orbitalstation.wordpress.com/tag/quaternion/
        let sarg = -2.0 * (self.x * self.z - self.w * self.y);
        let pi_2 = PI / 2.0;
        
        let (roll, pitch, yaw) = if sarg <= -0.99999 {
            // Gimbal lock case: pitch = -90 degrees
            let pitch = -pi_2;
            let roll = 0.0;
            let yaw = 2.0 * self.x.atan2(-self.y);
            (roll, pitch, yaw)
        } else if sarg >= 0.99999 {
            // Gimbal lock case: pitch = +90 degrees
            let pitch = pi_2;
            let roll = 0.0;
            let yaw = 2.0 * (-self.x).atan2(self.y);
            (roll, pitch, yaw)
        } else {
            // Normal case
            let pitch = sarg.asin();
            let roll = (2.0 * (self.y * self.z + self.w * self.x)).atan2(sqw - sqx - sqy + sqz);
            let yaw = (2.0 * (self.x * self.y + self.w * self.z)).atan2(sqw + sqx - sqy - sqz);
            (roll, pitch, yaw)
        };
        
        (roll, pitch, yaw)
    }
    
    /// Set quaternion from individual components
    pub fn set_from_quaternion(&mut self, quat_x: f64, quat_y: f64, quat_z: f64, quat_w: f64) {
        self.x = quat_x;
        self.y = quat_y;
        self.z = quat_z;
        self.w = quat_w;
        self.normalize();
    }
    
    /// Set quaternion from Roll-Pitch-Yaw angles
    pub fn set_from_rpy(&mut self, roll: f64, pitch: f64, yaw: f64) {
        let phi = roll / 2.0;
        let the = pitch / 2.0;
        let psi = yaw / 2.0;
        
        self.x = phi.sin() * the.cos() * psi.cos() - phi.cos() * the.sin() * psi.sin();
        self.y = phi.cos() * the.sin() * psi.cos() + phi.sin() * the.cos() * psi.sin();
        self.z = phi.cos() * the.cos() * psi.sin() - phi.sin() * the.sin() * psi.cos();
        self.w = phi.cos() * the.cos() * psi.cos() + phi.sin() * the.sin() * psi.sin();
        
        self.normalize();
    }
    
    /// Initialize from string representation of RPY angles "roll pitch yaw"
    pub fn init(&mut self, rotation_str: &str) -> UrdfResult<()> {
        let (roll, pitch, yaw) = parse_vector3(rotation_str)?;
        self.set_from_rpy(roll, pitch, yaw);
        Ok(())
    }
    
    /// Create Rotation from string representation of RPY angles
    pub fn from_string(rotation_str: &str) -> UrdfResult<Self> {
        let (roll, pitch, yaw) = parse_vector3(rotation_str)?;
        let mut rotation = Rotation::identity();
        rotation.set_from_rpy(roll, pitch, yaw);
        Ok(rotation)
    }
    
    /// Normalize the quaternion
    /// 
    /// Implements the exact normalization logic from C++, including
    /// the edge case handling for zero quaternions.
    pub fn normalize(&mut self) {
        let squared_norm = self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w;
        
        // Use <= instead of == to match C++ behavior and avoid float comparison warnings
        if squared_norm <= 0.0 {
            self.x = 0.0;
            self.y = 0.0;
            self.z = 0.0;  
            self.w = 1.0;
        } else {
            let s = squared_norm.sqrt();
            self.x /= s;
            self.y /= s;
            self.z /= s;
            self.w /= s;
        }
    }
    
    /// Get the inverse of this quaternion
    pub fn get_inverse(&self) -> Rotation {
        let norm = self.w * self.w + self.x * self.x + self.y * self.y + self.z * self.z;
        
        if norm > 0.0 {
            Rotation {
                w: self.w / norm,
                x: -self.x / norm,
                y: -self.y / norm,
                z: -self.z / norm,
            }
        } else {
            Rotation::identity()
        }
    }
}

impl Default for Rotation {
    fn default() -> Self {
        Rotation::identity()
    }
}

impl std::ops::Mul for Rotation {
    type Output = Rotation;
    
    /// Quaternion multiplication (copied from gazebo implementation in C++)
    fn mul(self, qt: Rotation) -> Rotation {
        Rotation {
            x: self.w * qt.x + self.x * qt.w + self.y * qt.z - self.z * qt.y,
            y: self.w * qt.y - self.x * qt.z + self.y * qt.w + self.z * qt.x,
            z: self.w * qt.z + self.x * qt.y - self.y * qt.x + self.z * qt.w,
            w: self.w * qt.w - self.x * qt.x - self.y * qt.y - self.z * qt.z,
        }
    }
}

impl std::ops::Mul<Vector3> for Rotation {
    type Output = Vector3;
    
    /// Rotate a vector using the quaternion
    fn mul(self, vec: Vector3) -> Vector3 {
        // Create temporary quaternion from vector
        let tmp = Rotation {
            w: 0.0,
            x: vec.x,
            y: vec.y, 
            z: vec.z,
        };
        
        // Apply rotation: self * tmp * self.inverse()
        let result = self * (tmp * self.get_inverse());
        
        Vector3::new(result.x, result.y, result.z)
    }
}

/// Combined position and orientation representation
/// 
/// Equivalent to urdf::Pose from the C++ implementation.
/// Uses C-compatible memory layout for FFI integration.
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Pose {
    pub position: Vector3,
    pub rotation: Rotation,
}

impl Pose {
    /// Create new Pose with specified position and rotation
    pub fn new(position: Vector3, rotation: Rotation) -> Self {
        Pose { position, rotation }
    }
    
    /// Create identity pose (zero position, no rotation)
    pub fn identity() -> Self {
        Pose {
            position: Vector3::zero(),
            rotation: Rotation::identity(),
        }
    }
    
    /// Clear to identity pose
    pub fn clear(&mut self) {
        self.position.clear();
        self.rotation.clear();
    }
}

impl Default for Pose {
    fn default() -> Self {
        Pose::identity()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::f64::EPSILON;
    
    fn approx_eq(a: f64, b: f64, tolerance: f64) -> bool {
        (a - b).abs() < tolerance
    }
    
    fn approx_eq_vec3(a: Vector3, b: Vector3, tolerance: f64) -> bool {
        approx_eq(a.x, b.x, tolerance) && approx_eq(a.y, b.y, tolerance) && approx_eq(a.z, b.z, tolerance)
    }
    
    #[test]
    fn test_vector3_construction() {
        let v1 = Vector3::new(1.0, 2.0, 3.0);
        assert_eq!(v1.x, 1.0);
        assert_eq!(v1.y, 2.0);
        assert_eq!(v1.z, 3.0);
        
        let v2 = Vector3::zero();
        assert_eq!(v2.x, 0.0);
        assert_eq!(v2.y, 0.0);
        assert_eq!(v2.z, 0.0);
    }
    
    #[test]
    fn test_vector3_operations() {
        let v1 = Vector3::new(1.0, 2.0, 3.0);
        let v2 = Vector3::new(4.0, 5.0, 6.0);
        let result = v1 + v2;
        
        assert_eq!(result.x, 5.0);
        assert_eq!(result.y, 7.0);
        assert_eq!(result.z, 9.0);
    }
    
    #[test]
    fn test_vector3_init_from_string() {
        let mut v = Vector3::zero();
        v.init("1.5 -2.7 3.14").unwrap();
        
        assert!(approx_eq(v.x, 1.5, EPSILON));
        assert!(approx_eq(v.y, -2.7, EPSILON));
        assert!(approx_eq(v.z, 3.14, EPSILON));
    }
    
    #[test]
    fn test_vector3_from_string() {
        let v = Vector3::from_string("1.0 2.0 3.0").unwrap();
        assert_eq!(v.x, 1.0);
        assert_eq!(v.y, 2.0); 
        assert_eq!(v.z, 3.0);
    }
    
    #[test]
    fn test_rotation_identity() {
        let r = Rotation::identity();
        assert_eq!(r.x, 0.0);
        assert_eq!(r.y, 0.0);
        assert_eq!(r.z, 0.0);
        assert_eq!(r.w, 1.0);
    }
    
    #[test]
    fn test_rotation_normalization() {
        let mut r = Rotation { x: 1.0, y: 1.0, z: 1.0, w: 1.0 };
        r.normalize();
        
        let norm = (r.x * r.x + r.y * r.y + r.z * r.z + r.w * r.w).sqrt();
        assert!(approx_eq(norm, 1.0, EPSILON));
    }
    
    #[test]
    fn test_rotation_rpy_conversion() {
        let mut r = Rotation::identity();
        r.set_from_rpy(0.1, 0.2, 0.3);
        
        let (roll, pitch, yaw) = r.get_rpy();
        assert!(approx_eq(roll, 0.1, 1e-10));
        assert!(approx_eq(pitch, 0.2, 1e-10));
        assert!(approx_eq(yaw, 0.3, 1e-10));
    }
    
    #[test]
    fn test_rotation_quaternion_conversion() {
        let mut r = Rotation::identity();
        r.set_from_quaternion(0.1, 0.2, 0.3, 0.9);
        
        let (x, y, z, w) = r.get_quaternion();
        // Should be normalized
        let norm = (x * x + y * y + z * z + w * w).sqrt();
        assert!(approx_eq(norm, 1.0, EPSILON));
    }
    
    #[test]
    fn test_rotation_from_string() {
        let r = Rotation::from_string("0.1 0.2 0.3").unwrap();
        let (roll, pitch, yaw) = r.get_rpy();
        
        assert!(approx_eq(roll, 0.1, 1e-10));
        assert!(approx_eq(pitch, 0.2, 1e-10));
        assert!(approx_eq(yaw, 0.3, 1e-10));
    }
    
    #[test]
    fn test_rotation_multiplication() {
        let r1 = Rotation::from_string("0.1 0.0 0.0").unwrap();
        let r2 = Rotation::from_string("0.0 0.1 0.0").unwrap();
        let result = r1 * r2;
        
        // Should be a valid normalized quaternion
        let (x, y, z, w) = result.get_quaternion();
        let norm = (x * x + y * y + z * z + w * w).sqrt();
        assert!(approx_eq(norm, 1.0, EPSILON));
    }
    
    #[test]
    fn test_rotation_vector_multiplication() {
        let r = Rotation::from_string("0.0 0.0 1.5708").unwrap(); // 90 degree yaw
        let v = Vector3::new(1.0, 0.0, 0.0);
        let result = r * v;
        

        
        // 90 degree rotation around Z should turn (1,0,0) into (0,1,0)
        // Use a reasonable tolerance for floating point comparison with quaternion operations
        assert!(approx_eq(result.x, 0.0, 1e-5));
        assert!(approx_eq(result.y, 1.0, 1e-5));
        assert!(approx_eq(result.z, 0.0, 1e-5));
    }
    
    #[test]
    fn test_rotation_inverse() {
        let r = Rotation::from_string("0.1 0.2 0.3").unwrap();
        let r_inv = r.get_inverse();
        let identity = r * r_inv;
        
        // r * r^-1 should be identity (or close to it)
        assert!(approx_eq(identity.x, 0.0, 1e-10));
        assert!(approx_eq(identity.y, 0.0, 1e-10));
        assert!(approx_eq(identity.z, 0.0, 1e-10));
        assert!(approx_eq(identity.w, 1.0, 1e-10));
    }
    
    #[test]
    fn test_pose_construction() {
        let pos = Vector3::new(1.0, 2.0, 3.0);
        let rot = Rotation::from_string("0.1 0.2 0.3").unwrap();
        let pose = Pose::new(pos, rot);
        
        assert_eq!(pose.position.x, 1.0);
        assert_eq!(pose.position.y, 2.0);
        assert_eq!(pose.position.z, 3.0);
    }
    
    #[test]
    fn test_pose_identity() {
        let pose = Pose::identity();
        assert_eq!(pose.position, Vector3::zero());
        assert_eq!(pose.rotation, Rotation::identity());
    }
    
    #[test] 
    fn test_pose_clear() {
        let mut pose = Pose::new(
            Vector3::new(1.0, 2.0, 3.0),
            Rotation::from_string("0.1 0.2 0.3").unwrap()
        );
        
        pose.clear();
        assert_eq!(pose.position, Vector3::zero());
        assert_eq!(pose.rotation, Rotation::identity());
    }
}