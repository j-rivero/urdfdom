//! URDF XML parsing module
//! 
//! This module contains the XML parsing logic for URDF files.
//! It uses quick-xml for fast, streaming XML parsing.

pub mod urdf;
pub mod utils;

pub use urdf::*;
pub use utils::*;