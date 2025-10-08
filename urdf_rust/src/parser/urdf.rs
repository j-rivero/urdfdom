//! URDF XML parsing implementation
//! 
//! This module implements the core URDF parsing logic using quick-xml
//! for high-performance XML parsing compatible with the original urdfdom.

use quick_xml::Reader;
use quick_xml::events::{Event, BytesStart};
use std::io::BufRead;
use std::str;

use crate::model::{Robot, Link, Joint, JointType, Vector3, Pose};
use crate::error::{UrdfError, UrdfResult};

/// Parse a URDF string into a Robot model
pub fn parse_urdf_string(urdf_content: &str) -> UrdfResult<Robot> {
    let mut reader = Reader::from_str(urdf_content);
    reader.trim_text(true);
    
    let mut buf = Vec::new();
    let mut robot = None;
    
    loop {
        match reader.read_event_into(&mut buf) {
            Ok(Event::Start(ref e)) => {
                match e.name().as_ref() {
                    b"robot" => {
                        robot = Some(parse_robot_element(&mut reader, e)?);
                    }
                    _ => {}
                }
            }
            Ok(Event::Eof) => break,
            Err(e) => return Err(UrdfError::ParseError(format!("XML parsing error: {}", e))),
            _ => {}
        }
        buf.clear();
    }
    
    robot.ok_or_else(|| UrdfError::ParseError("No robot element found".to_string()))
}

/// Parse the robot element and its children
fn parse_robot_element(reader: &mut Reader<&[u8]>, start: &BytesStart) -> UrdfResult<Robot> {
    // Get robot name from attributes
    let name = start.attributes()
        .find_map(|attr| {
            let attr = attr.ok()?;
            if attr.key.as_ref() == b"name" {
                Some(String::from_utf8_lossy(&attr.value).to_string())
            } else {
                None
            }
        })
        .ok_or_else(|| UrdfError::ParseError("Robot element missing name attribute".to_string()))?;
    
    let mut robot = Robot::new(name);
    let mut buf = Vec::new();
    
    loop {
        match reader.read_event_into(&mut buf) {
            Ok(Event::Start(ref e)) => {
                match e.name().as_ref() {
                    b"link" => {
                        let link = parse_link_element(reader, e)?;
                        robot.add_link(link);
                    }
                    b"joint" => {
                        let joint = parse_joint_element(reader, e)?;
                        robot.add_joint(joint);
                    }
                    _ => {
                        // Skip unknown elements
                        skip_element(reader, e.name().as_ref())?;
                    }
                }
            }
            Ok(Event::End(ref e)) => {
                if e.name().as_ref() == b"robot" {
                    break;
                }
            }
            Ok(Event::Eof) => {
                return Err(UrdfError::ParseError("Unexpected EOF in robot element".to_string()));
            }
            Err(e) => {
                return Err(UrdfError::ParseError(format!("XML parsing error: {}", e)));
            }
            _ => {}
        }
        buf.clear();
    }
    
    Ok(robot)
}

/// Parse a link element
fn parse_link_element(reader: &mut Reader<&[u8]>, start: &BytesStart) -> UrdfResult<Link> {
    // Get link name from attributes
    let name = start.attributes()
        .find_map(|attr| {
            let attr = attr.ok()?;
            if attr.key.as_ref() == b"name" {
                Some(String::from_utf8_lossy(&attr.value).to_string())
            } else {
                None
            }
        })
        .ok_or_else(|| UrdfError::ParseError("Link element missing name attribute".to_string()))?;
    
    let mut link = Link::new(name);
    let mut buf = Vec::new();
    
    loop {
        match reader.read_event_into(&mut buf) {
            Ok(Event::Start(ref e)) => {
                match e.name().as_ref() {
                    b"inertial" => {
                        // Skip inertial parsing for now - complex implementation
                        skip_element(reader, b"inertial")?;
                    }
                    b"visual" => {
                        // Skip visual parsing for now - complex implementation
                        skip_element(reader, b"visual")?;
                    }
                    b"collision" => {
                        // Skip collision parsing for now - complex implementation
                        skip_element(reader, b"collision")?;
                    }
                    _ => {
                        skip_element(reader, e.name().as_ref())?;
                    }
                }
            }
            Ok(Event::End(ref e)) => {
                if e.name().as_ref() == b"link" {
                    break;
                }
            }
            Ok(Event::Eof) => {
                return Err(UrdfError::ParseError("Unexpected EOF in link element".to_string()));
            }
            Err(e) => {
                return Err(UrdfError::ParseError(format!("XML parsing error: {}", e)));
            }
            _ => {}
        }
        buf.clear();
    }
    
    Ok(link)
}

/// Parse a joint element
fn parse_joint_element(reader: &mut Reader<&[u8]>, start: &BytesStart) -> UrdfResult<Joint> {
    // Get joint name and type from attributes
    let mut name = None;
    let mut joint_type = None;
    
    for attr in start.attributes() {
        let attr = attr.map_err(|e| UrdfError::ParseError(format!("Attribute error: {}", e)))?;
        match attr.key.as_ref() {
            b"name" => {
                name = Some(String::from_utf8_lossy(&attr.value).to_string());
            }
            b"type" => {
                let type_str = String::from_utf8_lossy(&attr.value);
                joint_type = Some(match type_str.as_ref() {
                    "fixed" => JointType::Fixed,
                    "revolute" => JointType::Revolute,
                    "continuous" => JointType::Continuous,
                    "prismatic" => JointType::Prismatic,
                    "floating" => JointType::Floating,
                    "planar" => JointType::Planar,
                    _ => return Err(UrdfError::ParseError(format!("Unknown joint type: {}", type_str))),
                });
            }
            _ => {}
        }
    }
    
    let name = name.ok_or_else(|| UrdfError::ParseError("Joint element missing name attribute".to_string()))?;
    let joint_type = joint_type.ok_or_else(|| UrdfError::ParseError("Joint element missing type attribute".to_string()))?;
    
    // Parse joint children to get parent and child links
    let mut parent = None;
    let mut child = None;
    let mut buf = Vec::new();
    
    loop {
        match reader.read_event_into(&mut buf) {
            Ok(Event::Start(ref e)) => {
                match e.name().as_ref() {
                    b"parent" => {
                        parent = parse_link_reference(e)?;
                    }
                    b"child" => {
                        child = parse_link_reference(e)?;
                    }
                    _ => {
                        // Skip other elements for now (origin, axis, limit, etc.)
                        skip_element(reader, e.name().as_ref())?;
                    }
                }
            }
            Ok(Event::Empty(ref e)) => {
                match e.name().as_ref() {
                    b"parent" => {
                        parent = parse_link_reference(e)?;
                    }
                    b"child" => {
                        child = parse_link_reference(e)?;
                    }
                    _ => {}
                }
            }
            Ok(Event::End(ref e)) => {
                if e.name().as_ref() == b"joint" {
                    break;
                }
            }
            Ok(Event::Eof) => {
                return Err(UrdfError::ParseError("Unexpected EOF in joint element".to_string()));
            }
            Err(e) => {
                return Err(UrdfError::ParseError(format!("XML parsing error: {}", e)));
            }
            _ => {}
        }
        buf.clear();
    }
    
    let parent = parent.ok_or_else(|| UrdfError::ParseError("Joint missing parent link".to_string()))?;
    let child = child.ok_or_else(|| UrdfError::ParseError("Joint missing child link".to_string()))?;
    
    Ok(Joint::new(name, joint_type, parent, child))
}

/// Parse a link reference (parent/child element)
fn parse_link_reference(element: &BytesStart) -> UrdfResult<Option<String>> {
    for attr in element.attributes() {
        let attr = attr.map_err(|e| UrdfError::ParseError(format!("Attribute error: {}", e)))?;
        if attr.key.as_ref() == b"link" {
            return Ok(Some(String::from_utf8_lossy(&attr.value).to_string()));
        }
    }
    Ok(None)
}

/// Skip an XML element and all its children
fn skip_element(reader: &mut Reader<&[u8]>, element_name: &[u8]) -> UrdfResult<()> {
    let mut depth = 1;
    let mut buf = Vec::new();
    
    loop {
        match reader.read_event_into(&mut buf) {
            Ok(Event::Start(ref e)) => {
                if e.name().as_ref() == element_name {
                    depth += 1;
                }
            }
            Ok(Event::End(ref e)) => {
                if e.name().as_ref() == element_name {
                    depth -= 1;
                    if depth == 0 {
                        break;
                    }
                }
            }
            Ok(Event::Eof) => {
                return Err(UrdfError::ParseError(format!(
                    "Unexpected EOF while skipping element: {}", 
                    String::from_utf8_lossy(element_name)
                )));
            }
            Err(e) => {
                return Err(UrdfError::ParseError(format!("XML parsing error: {}", e)));
            }
            _ => {}
        }
        buf.clear();
    }
    
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_simple_robot() {
        let urdf = r#"<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link"/>
  <link name="child_link"/>
  <joint name="test_joint" type="fixed">
    <parent link="base_link"/>
    <child link="child_link"/>
  </joint>
</robot>"#;

        let robot = parse_urdf_string(urdf).unwrap();
        assert_eq!(robot.name, "test_robot");
        assert_eq!(robot.links.len(), 2);
        assert_eq!(robot.joints.len(), 1);
        assert!(robot.has_link("base_link"));
        assert!(robot.has_link("child_link"));
        assert!(robot.has_joint("test_joint"));
    }

    #[test]
    fn test_parse_empty_robot() {
        let urdf = r#"<?xml version="1.0"?>
<robot name="empty_robot">
</robot>"#;

        let robot = parse_urdf_string(urdf).unwrap();
        assert_eq!(robot.name, "empty_robot");
        assert_eq!(robot.links.len(), 0);
        assert_eq!(robot.joints.len(), 0);
    }

    #[test]
    fn test_parse_invalid_xml() {
        let urdf = r#"<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link"
</robot>"#;

        assert!(parse_urdf_string(urdf).is_err());
    }

    #[test]
    fn test_parse_missing_robot_name() {
        let urdf = r#"<?xml version="1.0"?>
<robot>
  <link name="base_link"/>
</robot>"#;

        assert!(parse_urdf_string(urdf).is_err());
    }
}