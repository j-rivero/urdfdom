//! Parsing utilities
//! 
//! This module contains helper functions and utilities for URDF parsing.

use crate::error::{UrdfError, UrdfResult};

/// Parse a floating point number from a string
pub fn parse_f64(s: &str) -> UrdfResult<f64> {
    s.parse()
        .map_err(|_| UrdfError::ParseError(format!("Invalid number: {}", s)))
}

/// Parse a space-separated list of floating point numbers
pub fn parse_f64_list(s: &str) -> UrdfResult<Vec<f64>> {
    s.split_whitespace()
        .map(parse_f64)
        .collect()
}

/// Parse a 3D vector from a space-separated string
pub fn parse_vector3(s: &str) -> UrdfResult<crate::model::Vector3> {
    let values = parse_f64_list(s)?;
    if values.len() != 3 {
        return Err(UrdfError::ParseError(format!(
            "Expected 3 values for vector, got {}", values.len()
        )));
    }
    Ok(crate::model::Vector3::new(values[0], values[1], values[2]))
}

/// Parse a pose from xyz and rpy attribute strings
pub fn parse_pose(xyz: Option<&str>, rpy: Option<&str>) -> UrdfResult<crate::model::Pose> {
    let translation = match xyz {
        Some(s) => parse_vector3(s)?,
        None => crate::model::Vector3::default(),
    };
    
    let rotation = match rpy {
        Some(s) => parse_vector3(s)?,
        None => crate::model::Vector3::default(),
    };
    
    Ok(crate::model::Pose { translation, rotation })
}

/// Parse a boolean from a string
pub fn parse_bool(s: &str) -> UrdfResult<bool> {
    match s.to_lowercase().as_str() {
        "true" | "1" => Ok(true),
        "false" | "0" => Ok(false),
        _ => Err(UrdfError::ParseError(format!("Invalid boolean: {}", s))),
    }
}

/// Parse RGBA color values
pub fn parse_rgba(s: &str) -> UrdfResult<[f64; 4]> {
    let values = parse_f64_list(s)?;
    if values.len() != 4 {
        return Err(UrdfError::ParseError(format!(
            "Expected 4 values for RGBA color, got {}", values.len()
        )));
    }
    Ok([values[0], values[1], values[2], values[3]])
}

/// Validate that a string is a valid identifier (for names, etc.)
pub fn validate_identifier(s: &str) -> UrdfResult<()> {
    if s.is_empty() {
        return Err(UrdfError::InvalidInput("Identifier cannot be empty".to_string()));
    }
    
    if !s.chars().next().unwrap().is_ascii_alphabetic() && s.chars().next().unwrap() != '_' {
        return Err(UrdfError::InvalidInput(
            "Identifier must start with letter or underscore".to_string()
        ));
    }
    
    for c in s.chars() {
        if !c.is_ascii_alphanumeric() && c != '_' && c != '-' {
            return Err(UrdfError::InvalidInput(format!(
                "Invalid character '{}' in identifier", c
            )));
        }
    }
    
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_f64() {
        assert_eq!(parse_f64("3.14").unwrap(), 3.14);
        assert_eq!(parse_f64("-2.5").unwrap(), -2.5);
        assert_eq!(parse_f64("0").unwrap(), 0.0);
        assert!(parse_f64("invalid").is_err());
    }

    #[test]
    fn test_parse_f64_list() {
        let result = parse_f64_list("1.0 2.0 3.0").unwrap();
        assert_eq!(result, vec![1.0, 2.0, 3.0]);
        
        let result = parse_f64_list("  1.5   -2.5  ").unwrap();
        assert_eq!(result, vec![1.5, -2.5]);
        
        assert!(parse_f64_list("1.0 invalid 3.0").is_err());
    }

    #[test]
    fn test_parse_vector3() {
        let vec = parse_vector3("1.0 2.0 3.0").unwrap();
        assert_eq!(vec.x, 1.0);
        assert_eq!(vec.y, 2.0);
        assert_eq!(vec.z, 3.0);
        
        assert!(parse_vector3("1.0 2.0").is_err()); // Too few values
        assert!(parse_vector3("1.0 2.0 3.0 4.0").is_err()); // Too many values
    }

    #[test]
    fn test_parse_pose() {
        let pose = parse_pose(Some("1.0 2.0 3.0"), Some("0.1 0.2 0.3")).unwrap();
        assert_eq!(pose.translation.x, 1.0);
        assert_eq!(pose.rotation.z, 0.3);
        
        let pose = parse_pose(None, None).unwrap();
        assert_eq!(pose.translation.x, 0.0);
        assert_eq!(pose.rotation.x, 0.0);
    }

    #[test]
    fn test_parse_bool() {
        assert_eq!(parse_bool("true").unwrap(), true);
        assert_eq!(parse_bool("True").unwrap(), true);
        assert_eq!(parse_bool("1").unwrap(), true);
        assert_eq!(parse_bool("false").unwrap(), false);
        assert_eq!(parse_bool("FALSE").unwrap(), false);
        assert_eq!(parse_bool("0").unwrap(), false);
        assert!(parse_bool("maybe").is_err());
    }

    #[test]
    fn test_parse_rgba() {
        let color = parse_rgba("1.0 0.5 0.0 0.8").unwrap();
        assert_eq!(color, [1.0, 0.5, 0.0, 0.8]);
        
        assert!(parse_rgba("1.0 0.5").is_err()); // Too few values
    }

    #[test]
    fn test_validate_identifier() {
        assert!(validate_identifier("valid_name").is_ok());
        assert!(validate_identifier("_underscore").is_ok());
        assert!(validate_identifier("name123").is_ok());
        assert!(validate_identifier("with-dash").is_ok());
        
        assert!(validate_identifier("").is_err()); // Empty
        assert!(validate_identifier("123name").is_err()); // Starts with number
        assert!(validate_identifier("invalid space").is_err()); // Contains space
        assert!(validate_identifier("invalid@symbol").is_err()); // Invalid character
    }
}