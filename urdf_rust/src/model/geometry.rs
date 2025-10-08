//! Geometry definitions for URDF elements
//! 
//! This module defines geometric shapes used in visual and 
//! collision elements of URDF links.

use serde::{Deserialize, Serialize};

/// Geometric shapes
#[derive(Debug, Clone, PartialEq)]
#[cfg_attr(feature = "serde", derive(Serialize, Deserialize))]
pub enum Geometry {
    /// Box geometry
    Box { size: [f64; 3] },
    
    /// Cylinder geometry
    Cylinder { radius: f64, length: f64 },
    
    /// Sphere geometry
    Sphere { radius: f64 },
    
    /// Mesh geometry
    Mesh { 
        filename: String, 
        scale: Option<[f64; 3]> 
    },
}

impl Geometry {
    /// Create a box geometry
    pub fn box_geometry(x: f64, y: f64, z: f64) -> Self {
        Self::Box { size: [x, y, z] }
    }
    
    /// Create a cylinder geometry
    pub fn cylinder(radius: f64, length: f64) -> Self {
        Self::Cylinder { radius, length }
    }
    
    /// Create a sphere geometry
    pub fn sphere(radius: f64) -> Self {
        Self::Sphere { radius }
    }
    
    /// Create a mesh geometry
    pub fn mesh(filename: String, scale: Option<[f64; 3]>) -> Self {
        Self::Mesh { filename, scale }
    }
    
    /// Get the type name of the geometry
    pub fn type_name(&self) -> &'static str {
        match self {
            Self::Box { .. } => "box",
            Self::Cylinder { .. } => "cylinder", 
            Self::Sphere { .. } => "sphere",
            Self::Mesh { .. } => "mesh",
        }
    }
}