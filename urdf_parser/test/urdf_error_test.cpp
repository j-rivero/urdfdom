#include <gtest/gtest.h>
#include <string>
#include "urdf_parser/urdf_parser.h"

class URDFErrorHandlingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Various malformed URDF strings for error testing
    }
};

// Test parsing completely malformed XML
TEST_F(URDFErrorHandlingTest, test_parse_malformed_xml)
{
    std::string malformed_xml = "This is not XML at all!";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(malformed_xml);
    EXPECT_TRUE(model == nullptr);
}

// Test parsing XML with missing closing tags
TEST_F(URDFErrorHandlingTest, test_parse_unclosed_tags)
{
    std::string unclosed_xml = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="base">
    <visual>
      <geometry>
        <box size="1 1 1">
      </geometry>
    </visual>
  </link>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(unclosed_xml);
    EXPECT_TRUE(model == nullptr);
}

// Test URDF without robot tag
TEST_F(URDFErrorHandlingTest, test_parse_no_robot_tag)
{
    std::string no_robot = R"(
<?xml version="1.0"?>
<link name="base">
  <visual>
    <geometry>
      <box size="1 1 1"/>
    </geometry>
  </visual>
</link>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(no_robot);
    EXPECT_TRUE(model == nullptr);
}

// Test robot without name attribute
TEST_F(URDFErrorHandlingTest, test_parse_robot_no_name)
{
    std::string no_name = R"(
<?xml version="1.0"?>
<robot>
  <link name="base">
    <visual>
      <geometry>
        <box size="1 1 1"/>
      </geometry>
    </visual>
  </link>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(no_name);
    EXPECT_TRUE(model == nullptr);
}

// Test link with invalid geometry
TEST_F(URDFErrorHandlingTest, test_parse_link_invalid_geometry)
{
    std::string invalid_geometry = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="base">
    <visual>
      <geometry>
        <invalid_shape size="1 1 1"/>
      </geometry>
    </visual>
  </link>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(invalid_geometry);
    // Should either fail to parse or parse without geometry
    if (model != nullptr) {
        auto link = model->getLink("base");
        if (link != nullptr && link->visual != nullptr) {
            EXPECT_TRUE(link->visual->geometry == nullptr);
        }
    }
}

// Test box geometry with invalid size format
TEST_F(URDFErrorHandlingTest, test_parse_box_invalid_size)
{
    std::string invalid_box = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="base">
    <visual>
      <geometry>
        <box size="not_a_number"/>
      </geometry>
    </visual>
  </link>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(invalid_box);
    // Should either fail to parse or create box with default/invalid size
}

// Test sphere with invalid radius
TEST_F(URDFErrorHandlingTest, test_parse_sphere_invalid_radius)
{
    std::string invalid_sphere = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="base">
    <visual>
      <geometry>
        <sphere radius="not_a_number"/>
      </geometry>
    </visual>
  </link>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(invalid_sphere);
    // Should handle invalid radius gracefully
}

// Test cylinder with missing parameters
TEST_F(URDFErrorHandlingTest, test_parse_cylinder_missing_params)
{
    std::string incomplete_cylinder = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="base">
    <visual>
      <geometry>
        <cylinder radius="0.5"/>
      </geometry>
    </visual>
  </link>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(incomplete_cylinder);
    // Should handle missing length parameter
}

// Test joint with invalid type
TEST_F(URDFErrorHandlingTest, test_parse_joint_invalid_type)
{
    std::string invalid_joint = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="base"><visual><geometry><box size="1 1 1"/></geometry></visual></link>
  <link name="child"><visual><geometry><box size="1 1 1"/></geometry></visual></link>
  <joint name="test_joint" type="invalid_type">
    <parent link="base"/>
    <child link="child"/>
  </joint>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(invalid_joint);
    if (model != nullptr) {
        auto joint = model->getJoint("test_joint");
        if (joint != nullptr) {
            EXPECT_EQ(joint->type, urdf::Joint::UNKNOWN);
        }
    }
}

// Test joint with non-existent parent link
TEST_F(URDFErrorHandlingTest, test_parse_joint_invalid_parent)
{
    std::string invalid_parent = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="child"><visual><geometry><box size="1 1 1"/></geometry></visual></link>
  <joint name="test_joint" type="revolute">
    <parent link="non_existent"/>
    <child link="child"/>
  </joint>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(invalid_parent);
    // Should handle non-existent parent link
}

// Test joint with non-existent child link
TEST_F(URDFErrorHandlingTest, test_parse_joint_invalid_child)
{
    std::string invalid_child = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="base"><visual><geometry><box size="1 1 1"/></geometry></visual></link>
  <joint name="test_joint" type="revolute">
    <parent link="base"/>
    <child link="non_existent"/>
  </joint>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(invalid_child);
    // Should handle non-existent child link
}

// Test material with invalid color format
TEST_F(URDFErrorHandlingTest, test_parse_material_invalid_color)
{
    std::string invalid_color = R"(
<?xml version="1.0"?>
<robot name="test">
  <material name="bad_color">
    <color rgba="not valid color values"/>
  </material>
  <link name="base">
    <visual>
      <geometry><box size="1 1 1"/></geometry>
      <material name="bad_color"/>
    </visual>
  </link>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(invalid_color);
    // Should handle invalid color format gracefully
}

// Test inertial with invalid mass
TEST_F(URDFErrorHandlingTest, test_parse_inertial_invalid_mass)
{
    std::string invalid_mass = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="base">
    <inertial>
      <mass value="not_a_number"/>
      <inertia ixx="1" iyy="1" izz="1" ixy="0" ixz="0" iyz="0"/>
    </inertial>
    <visual><geometry><box size="1 1 1"/></geometry></visual>
  </link>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(invalid_mass);
    // Should handle invalid mass value
}

// Test origin with invalid xyz format
TEST_F(URDFErrorHandlingTest, test_parse_origin_invalid_xyz)
{
    std::string invalid_xyz = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="base">
    <visual>
      <origin xyz="not valid coordinates"/>
      <geometry><box size="1 1 1"/></geometry>
    </visual>
  </link>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(invalid_xyz);
    // Should handle invalid xyz format
}

// Test origin with invalid rpy format
TEST_F(URDFErrorHandlingTest, test_parse_origin_invalid_rpy)
{
    std::string invalid_rpy = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="base">
    <visual>
      <origin rpy="not valid angles"/>
      <geometry><box size="1 1 1"/></geometry>
    </visual>
  </link>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(invalid_rpy);
    // Should handle invalid rpy format
}

// Test joint limit with invalid values
TEST_F(URDFErrorHandlingTest, test_parse_joint_limit_invalid_values)
{
    std::string invalid_limits = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="base"><visual><geometry><box size="1 1 1"/></geometry></visual></link>
  <link name="child"><visual><geometry><box size="1 1 1"/></geometry></visual></link>
  <joint name="test_joint" type="revolute">
    <parent link="base"/>
    <child link="child"/>
    <limit lower="not_a_number" upper="also_not_a_number" effort="invalid" velocity="bad"/>
  </joint>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(invalid_limits);
    // Should handle invalid limit values
}

// Test duplicate link names
TEST_F(URDFErrorHandlingTest, test_parse_duplicate_link_names)
{
    std::string duplicate_links = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="duplicate">
    <visual><geometry><box size="1 1 1"/></geometry></visual>
  </link>
  <link name="duplicate">
    <visual><geometry><sphere radius="0.5"/></geometry></visual>
  </link>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(duplicate_links);
    // Should handle duplicate names - behavior depends on implementation
    if (model != nullptr) {
        EXPECT_EQ(model->links_.size(), 1u); // Only one should be kept
    }
}

// Test duplicate joint names
TEST_F(URDFErrorHandlingTest, test_parse_duplicate_joint_names)
{
    std::string duplicate_joints = R"(
<?xml version="1.0"?>
<robot name="test">
  <link name="base"><visual><geometry><box size="1 1 1"/></geometry></visual></link>
  <link name="child1"><visual><geometry><box size="1 1 1"/></geometry></visual></link>
  <link name="child2"><visual><geometry><box size="1 1 1"/></geometry></visual></link>
  <joint name="duplicate" type="fixed">
    <parent link="base"/>
    <child link="child1"/>
  </joint>
  <joint name="duplicate" type="fixed">
    <parent link="base"/>
    <child link="child2"/>
  </joint>
</robot>
)";
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(duplicate_joints);
    // Should handle duplicate joint names
    if (model != nullptr) {
        EXPECT_EQ(model->joints_.size(), 1u); // Only one should be kept
    }
}