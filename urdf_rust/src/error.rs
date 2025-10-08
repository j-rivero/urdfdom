//! Error types and handling for the URDF Rust library
//! 
//! This module defines the error types used throughout the library
//! and provides conversion traits for interoperability with other
//! error handling systems.

use std::fmt;
use std::error::Error as StdError;

/// Main error type for URDF operations
#[derive(Debug, Clone)]
pub enum UrdfError {
    /// Error during XML parsing
    ParseError(String),
    
    /// I/O related error (file reading, etc.)
    IoError(String),
    
    /// Invalid input or parameter
    InvalidInput(String),
}

impl fmt::Display for UrdfError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            UrdfError::ParseError(msg) => write!(f, "URDF parse error: {}", msg),
            UrdfError::IoError(msg) => write!(f, "I/O error: {}", msg),
            UrdfError::InvalidInput(msg) => write!(f, "Invalid input: {}", msg),
        }
    }
}

impl StdError for UrdfError {
    fn source(&self) -> Option<&(dyn StdError + 'static)> {
        None
    }
}

/// Result type for URDF operations
pub type UrdfResult<T> = Result<T, UrdfError>;

impl From<std::io::Error> for UrdfError {
    fn from(error: std::io::Error) -> Self {
        UrdfError::IoError(error.to_string())
    }
}

impl From<quick_xml::Error> for UrdfError {
    fn from(error: quick_xml::Error) -> Self {
        UrdfError::ParseError(error.to_string())
    }
}

#[cfg(feature = "serde")]
impl From<serde_xml_rs::Error> for UrdfError {
    fn from(error: serde_xml_rs::Error) -> Self {
        UrdfError::ParseError(error.to_string())
    }
}