//! Robot data structures
//! 
//! This module defines the core robot model structures including
//! Robot, Link, Joint, and related types.

use std::collections::HashMap;
use serde::{Deserialize, Serialize};

/// Main robot description
#[derive(Debug, Clone, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct Robot {
    /// Robot name
    pub name: String,
    
    /// Named links in the robot
    pub links: HashMap<String, Link>,
    
    /// Named joints in the robot  
    pub joints: HashMap<String, Joint>,
    
    /// Robot version (optional)
    pub version: Option<String>,
}

impl Robot {
    /// Create a new empty robot
    pub fn new(name: String) -> Self {
        Self {
            name,
            links: HashMap::new(),
            joints: HashMap::new(),
            version: None,
        }
    }
    
    /// Add a link to the robot
    pub fn add_link(&mut self, link: Link) {
        self.links.insert(link.name.clone(), link);
    }
    
    /// Add a joint to the robot
    pub fn add_joint(&mut self, joint: Joint) {
        self.joints.insert(joint.name.clone(), joint);
    }
    
    /// Get a link by name
    pub fn get_link(&self, name: &str) -> Option<&Link> {
        self.links.get(name)
    }
    
    /// Get a joint by name
    pub fn get_joint(&self, name: &str) -> Option<&Joint> {
        self.joints.get(name)
    }
    
    /// Check if robot has a link with given name
    pub fn has_link(&self, name: &str) -> bool {
        self.links.contains_key(name)
    }
    
    /// Check if robot has a joint with given name
    pub fn has_joint(&self, name: &str) -> bool {
        self.joints.contains_key(name)
    }
}

/// Robot link description
#[derive(Debug, Clone, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct Link {
    /// Link name
    pub name: String,
    
    /// Inertial properties (optional)
    pub inertial: Option<Inertial>,
    
    /// Visual elements
    pub visual: Vec<Visual>,
    
    /// Collision elements
    pub collision: Vec<Collision>,
}

impl Link {
    /// Create a new link
    pub fn new(name: String) -> Self {
        Self {
            name,
            inertial: None,
            visual: Vec::new(),
            collision: Vec::new(),
        }
    }
}

/// Robot joint description
#[derive(Debug, Clone, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct Joint {
    /// Joint name
    pub name: String,
    
    /// Joint type
    pub joint_type: JointType,
    
    /// Parent link name
    pub parent: String,
    
    /// Child link name
    pub child: String,
    
    /// Joint origin transform
    pub origin: Option<Pose>,
    
    /// Joint axis (for revolute/prismatic joints)
    pub axis: Option<Vector3>,
    
    /// Joint limits (for revolute/prismatic joints)
    pub limit: Option<JointLimit>,
    
    /// Joint dynamics (optional)
    pub dynamics: Option<JointDynamics>,
    
    /// Safety controller (optional)
    pub safety: Option<JointSafety>,
    
    /// Calibration reference (optional)
    pub calibration: Option<JointCalibration>,
    
    /// Mimic joint (optional)
    pub mimic: Option<JointMimic>,
}

impl Joint {
    /// Create a new joint
    pub fn new(name: String, joint_type: JointType, parent: String, child: String) -> Self {
        Self {
            name,
            joint_type,
            parent,
            child,
            origin: None,
            axis: None,
            limit: None,
            dynamics: None,
            safety: None,
            calibration: None,
            mimic: None,
        }
    }
}

/// Joint types
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub enum JointType {
    /// Fixed joint (no movement)
    Fixed,
    
    /// Revolute joint (rotation around axis)
    Revolute,
    
    /// Continuous joint (unlimited rotation)
    Continuous,
    
    /// Prismatic joint (translation along axis)
    Prismatic,
    
    /// Floating joint (6-DOF)
    Floating,
    
    /// Planar joint (2D movement)
    Planar,
}

/// 3D pose (position + orientation)
#[derive(Debug, Clone, Copy, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct Pose {
    /// Translation
    pub translation: Vector3,
    
    /// Rotation (as Euler angles XYZ)
    pub rotation: Vector3,
}

impl Default for Pose {
    fn default() -> Self {
        Self {
            translation: Vector3::default(),
            rotation: Vector3::default(),
        }
    }
}

/// 3D vector
#[derive(Debug, Clone, Copy, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct Vector3 {
    pub x: f64,
    pub y: f64,
    pub z: f64,
}

impl Default for Vector3 {
    fn default() -> Self {
        Self { x: 0.0, y: 0.0, z: 0.0 }
    }
}

impl Vector3 {
    /// Create a new vector
    pub fn new(x: f64, y: f64, z: f64) -> Self {
        Self { x, y, z }
    }
}

/// Inertial properties
#[derive(Debug, Clone, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct Inertial {
    /// Mass
    pub mass: f64,
    
    /// Center of mass offset
    pub origin: Option<Pose>,
    
    /// Inertia matrix
    pub inertia: InertiaMatrix,
}

/// 3x3 Inertia matrix (symmetric)
#[derive(Debug, Clone, Copy, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct InertiaMatrix {
    pub ixx: f64,
    pub ixy: f64,
    pub ixz: f64,
    pub iyy: f64,
    pub iyz: f64,
    pub izz: f64,
}

impl Default for InertiaMatrix {
    fn default() -> Self {
        Self {
            ixx: 0.0, ixy: 0.0, ixz: 0.0,
            iyy: 0.0, iyz: 0.0, izz: 0.0,
        }
    }
}

/// Visual element
#[derive(Debug, Clone, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct Visual {
    /// Visual name (optional)
    pub name: Option<String>,
    
    /// Visual origin
    pub origin: Option<Pose>,
    
    /// Geometry
    pub geometry: super::geometry::Geometry,
    
    /// Material (optional)
    pub material: Option<Material>,
}

/// Collision element
#[derive(Debug, Clone, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct Collision {
    /// Collision name (optional)
    pub name: Option<String>,
    
    /// Collision origin
    pub origin: Option<Pose>,
    
    /// Geometry
    pub geometry: super::geometry::Geometry,
}

/// Material properties
#[derive(Debug, Clone, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct Material {
    /// Material name
    pub name: String,
    
    /// Color (RGBA)
    pub color: Option<[f64; 4]>,
    
    /// Texture filename
    pub texture: Option<String>,
}

/// Joint limit
#[derive(Debug, Clone, Copy, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct JointLimit {
    /// Lower limit
    pub lower: f64,
    
    /// Upper limit
    pub upper: f64,
    
    /// Effort limit
    pub effort: f64,
    
    /// Velocity limit
    pub velocity: f64,
}

/// Joint dynamics
#[derive(Debug, Clone, Copy, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct JointDynamics {
    /// Damping coefficient
    pub damping: f64,
    
    /// Friction coefficient
    pub friction: f64,
}

/// Joint safety controller
#[derive(Debug, Clone, Copy, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct JointSafety {
    /// Soft lower limit
    pub soft_lower_limit: f64,
    
    /// Soft upper limit
    pub soft_upper_limit: f64,
    
    /// Position tolerance
    pub k_position: f64,
    
    /// Velocity tolerance
    pub k_velocity: f64,
}

/// Joint calibration
#[derive(Debug, Clone, Copy, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct JointCalibration {
    /// Reference position
    pub reference_position: f64,
    
    /// Rising edge position
    pub rising: Option<f64>,
    
    /// Falling edge position
    pub falling: Option<f64>,
}

/// Joint mimic
#[derive(Debug, Clone, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub struct JointMimic {
    /// Joint to mimic
    pub joint: String,
    
    /// Multiplier
    pub multiplier: f64,
    
    /// Offset
    pub offset: f64,
}