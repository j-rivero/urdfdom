#include <gtest/gtest.h>
#include <string>
#include <sstream>
#include "urdf_parser/urdf_parser.h"
#include "urdf_model/model.h"

class URDFModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Valid minimal URDF for testing
        valid_minimal_urdf = R"(
<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link">
    <visual>
      <geometry>
        <box size="1 1 1"/>
      </geometry>
    </visual>
  </link>
</robot>
)";

        // URDF with version
        urdf_with_version = R"(
<?xml version="1.0"?>
<robot name="test_robot" version="1.0">
  <link name="base_link">
    <visual>
      <geometry>
        <box size="1 1 1"/>
      </geometry>
    </visual>
  </link>
</robot>
)";

        // URDF with materials
        urdf_with_materials = R"(
<?xml version="1.0"?>
<robot name="test_robot">
  <material name="red">
    <color rgba="1 0 0 1"/>
  </material>
  <link name="base_link">
    <visual>
      <geometry>
        <box size="1 1 1"/>
      </geometry>
      <material name="red"/>
    </visual>
  </link>
</robot>
)";

        // URDF with joints
        urdf_with_joint = R"(
<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link">
    <visual>
      <geometry>
        <box size="1 1 1"/>
      </geometry>
    </visual>
  </link>
  <link name="child_link">
    <visual>
      <geometry>
        <cylinder radius="0.5" length="1.0"/>
      </geometry>
    </visual>
  </link>
  <joint name="test_joint" type="revolute">
    <parent link="base_link"/>
    <child link="child_link"/>
    <origin xyz="0 0 1" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57" effort="100" velocity="1"/>
  </joint>
</robot>
)";

        // Invalid URDFs for error testing
        invalid_xml = "This is not valid XML";
        
        empty_robot = R"(
<?xml version="1.0"?>
<robot name="empty_robot">
</robot>
)";
    }

    std::string valid_minimal_urdf;
    std::string urdf_with_version;
    std::string urdf_with_materials;
    std::string urdf_with_joint;
    std::string invalid_xml;
    std::string empty_robot;
};

// Test basic URDF parsing functionality
TEST_F(URDFModelTest, test_parse_valid_minimal_urdf)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(valid_minimal_urdf);
    ASSERT_TRUE(model != nullptr);
    EXPECT_EQ(model->getName(), "test_robot");
    EXPECT_EQ(model->links_.size(), 1u);
    EXPECT_TRUE(model->getLink("base_link") != nullptr);
}

// Test URDF with version parsing
TEST_F(URDFModelTest, test_parse_urdf_with_version)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_with_version);
    ASSERT_TRUE(model != nullptr);
    EXPECT_EQ(model->getName(), "test_robot");
    // Version should be parsed and stored
}

// Test invalid XML handling
TEST_F(URDFModelTest, test_parse_invalid_xml)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(invalid_xml);
    EXPECT_TRUE(model == nullptr);
}

// Test empty XML handling
TEST_F(URDFModelTest, test_parse_empty_xml)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF("");
    EXPECT_TRUE(model == nullptr);
}

// Test URDF with materials
TEST_F(URDFModelTest, test_parse_urdf_with_materials)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_with_materials);
    ASSERT_TRUE(model != nullptr);
    EXPECT_EQ(model->materials_.size(), 1u);
    
    auto material = model->getMaterial("red");
    ASSERT_TRUE(material != nullptr);
    EXPECT_EQ(material->name, "red");
    EXPECT_FLOAT_EQ(material->color.r, 1.0f);
    EXPECT_FLOAT_EQ(material->color.g, 0.0f);
    EXPECT_FLOAT_EQ(material->color.b, 0.0f);
    EXPECT_FLOAT_EQ(material->color.a, 1.0f);
}

// Test URDF with joints
TEST_F(URDFModelTest, test_parse_urdf_with_joint)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_with_joint);
    ASSERT_TRUE(model != nullptr);
    EXPECT_EQ(model->joints_.size(), 1u);
    EXPECT_EQ(model->links_.size(), 2u);
    
    auto joint = model->getJoint("test_joint");
    ASSERT_TRUE(joint != nullptr);
    EXPECT_EQ(joint->name, "test_joint");
    EXPECT_EQ(joint->type, urdf::Joint::REVOLUTE);
    EXPECT_EQ(joint->parent_link_name, "base_link");
    EXPECT_EQ(joint->child_link_name, "child_link");
}

// Test model clear functionality
TEST_F(URDFModelTest, test_model_clear)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_with_joint);
    ASSERT_TRUE(model != nullptr);
    EXPECT_FALSE(model->links_.empty());
    EXPECT_FALSE(model->joints_.empty());
    
    model->clear();
    EXPECT_TRUE(model->links_.empty());
    EXPECT_TRUE(model->joints_.empty());
    EXPECT_TRUE(model->materials_.empty());
    EXPECT_TRUE(model->getName().empty());
}

// Test getLink functionality
TEST_F(URDFModelTest, test_get_link)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_with_joint);
    ASSERT_TRUE(model != nullptr);
    
    // Test existing link
    auto link = model->getLink("base_link");
    EXPECT_TRUE(link != nullptr);
    EXPECT_EQ(link->name, "base_link");
    
    // Test non-existing link
    auto non_existing = model->getLink("non_existing_link");
    EXPECT_TRUE(non_existing == nullptr);
}

// Test getJoint functionality
TEST_F(URDFModelTest, test_get_joint)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_with_joint);
    ASSERT_TRUE(model != nullptr);
    
    // Test existing joint
    auto joint = model->getJoint("test_joint");
    EXPECT_TRUE(joint != nullptr);
    EXPECT_EQ(joint->name, "test_joint");
    
    // Test non-existing joint
    auto non_existing = model->getJoint("non_existing_joint");
    EXPECT_TRUE(non_existing == nullptr);
}

// Test getMaterial functionality
TEST_F(URDFModelTest, test_get_material)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_with_materials);
    ASSERT_TRUE(model != nullptr);
    
    // Test existing material
    auto material = model->getMaterial("red");
    EXPECT_TRUE(material != nullptr);
    EXPECT_EQ(material->name, "red");
    
    // Test non-existing material
    auto non_existing = model->getMaterial("non_existing_material");
    EXPECT_TRUE(non_existing == nullptr);
}

// Test URDF file parsing (requires creating a temporary file)
TEST_F(URDFModelTest, test_parse_urdf_file_non_existent)
{
    // Test parsing non-existent file
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDFFile("/non/existent/file.urdf");
    EXPECT_TRUE(model == nullptr);
}

// Test root link identification
TEST_F(URDFModelTest, test_get_root_link)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_with_joint);
    ASSERT_TRUE(model != nullptr);
    
    auto root = model->getRoot();
    EXPECT_TRUE(root != nullptr);
    EXPECT_EQ(root->name, "base_link");
}

// Test URDF with malformed robot tag
TEST_F(URDFModelTest, test_parse_urdf_no_robot_tag)
{
    std::string no_robot_tag = R"(
<?xml version="1.0"?>
<not_robot name="test">
  <link name="base_link"/>
</not_robot>
)";
    
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(no_robot_tag);
    EXPECT_TRUE(model == nullptr);
}

// Test URDF with no name attribute
TEST_F(URDFModelTest, test_parse_urdf_no_name)
{
    std::string no_name_urdf = R"(
<?xml version="1.0"?>
<robot>
  <link name="base_link">
    <visual>
      <geometry>
        <box size="1 1 1"/>
      </geometry>
    </visual>
  </link>
</robot>
)";
    
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(no_name_urdf);
    EXPECT_TRUE(model == nullptr);
}