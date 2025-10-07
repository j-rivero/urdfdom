#include <gtest/gtest.h>
#include <urdf_model/model.h>
#include <urdf_parser/urdf_parser.h>

class URDFJointDynamicsTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test joint with dynamics - damping only
TEST_F(URDFJointDynamicsTest, test_parse_joint_dynamics_damping_only)
{
    std::string urdf_string = R"(
<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link"/>
  <link name="child_link"/>
  <joint name="test_joint" type="revolute">
    <parent link="base_link"/>
    <child link="child_link"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57" effort="10" velocity="1"/>
    <dynamics damping="0.5"/>
  </joint>
</robot>
)";

    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_string);
    ASSERT_TRUE(model != nullptr);

    auto joint = model->getJoint("test_joint");
    ASSERT_TRUE(joint != nullptr);

    ASSERT_TRUE(joint->dynamics != nullptr);
    EXPECT_DOUBLE_EQ(joint->dynamics->damping, 0.5);
    EXPECT_DOUBLE_EQ(joint->dynamics->friction, 0.0);  // Should default to 0
}

// Test joint with dynamics - friction only
TEST_F(URDFJointDynamicsTest, test_parse_joint_dynamics_friction_only)
{
    std::string urdf_string = R"(
<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link"/>
  <link name="child_link"/>
  <joint name="test_joint" type="revolute">
    <parent link="base_link"/>
    <child link="child_link"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57" effort="10" velocity="1"/>
    <dynamics friction="0.3"/>
  </joint>
</robot>
)";

    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_string);
    ASSERT_TRUE(model != nullptr);

    auto joint = model->getJoint("test_joint");
    ASSERT_TRUE(joint != nullptr);

    ASSERT_TRUE(joint->dynamics != nullptr);
    EXPECT_DOUBLE_EQ(joint->dynamics->damping, 0.0);  // Should default to 0
    EXPECT_DOUBLE_EQ(joint->dynamics->friction, 0.3);
}

// Test joint with dynamics - both damping and friction
TEST_F(URDFJointDynamicsTest, test_parse_joint_dynamics_both_values)
{
    std::string urdf_string = R"(
<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link"/>
  <link name="child_link"/>
  <joint name="test_joint" type="revolute">
    <parent link="base_link"/>
    <child link="child_link"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57" effort="10" velocity="1"/>
    <dynamics damping="1.2" friction="0.8"/>
  </joint>
</robot>
)";

    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_string);
    ASSERT_TRUE(model != nullptr);

    auto joint = model->getJoint("test_joint");
    ASSERT_TRUE(joint != nullptr);

    ASSERT_TRUE(joint->dynamics != nullptr);
    EXPECT_DOUBLE_EQ(joint->dynamics->damping, 1.2);
    EXPECT_DOUBLE_EQ(joint->dynamics->friction, 0.8);
}

// Test joint with invalid dynamics - empty dynamics element (should fail)
TEST_F(URDFJointDynamicsTest, test_parse_joint_dynamics_empty_element)
{
    std::string urdf_string = R"(
<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link"/>
  <link name="child_link"/>
  <joint name="test_joint" type="revolute">
    <parent link="base_link"/>
    <child link="child_link"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57" effort="10" velocity="1"/>
    <dynamics/>
  </joint>
</robot>
)";

    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_string);
    EXPECT_TRUE(model == nullptr);  // Should fail to parse
}

// Test joint with invalid dynamics - invalid damping value
TEST_F(URDFJointDynamicsTest, test_parse_joint_dynamics_invalid_damping)
{
    std::string urdf_string = R"(
<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link"/>
  <link name="child_link"/>
  <joint name="test_joint" type="revolute">
    <parent link="base_link"/>
    <child link="child_link"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57" effort="10" velocity="1"/>
    <dynamics damping="not_a_number"/>
  </joint>
</robot>
)";

    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_string);
    EXPECT_TRUE(model == nullptr);  // Should fail to parse
}

// Test joint with invalid dynamics - invalid friction value
TEST_F(URDFJointDynamicsTest, test_parse_joint_dynamics_invalid_friction)
{
    std::string urdf_string = R"(
<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link"/>
  <link name="child_link"/>
  <joint name="test_joint" type="revolute">
    <parent link="base_link"/>
    <child link="child_link"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57" effort="10" velocity="1"/>
    <dynamics friction="invalid_value"/>
  </joint>
</robot>
)";

    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_string);
    EXPECT_TRUE(model == nullptr);  // Should fail to parse
}

// Test joint dynamics with zero values
TEST_F(URDFJointDynamicsTest, test_parse_joint_dynamics_zero_values)
{
    std::string urdf_string = R"(
<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link"/>
  <link name="child_link"/>
  <joint name="test_joint" type="revolute">
    <parent link="base_link"/>
    <child link="child_link"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57" effort="10" velocity="1"/>
    <dynamics damping="0.0" friction="0.0"/>
  </joint>
</robot>
)";

    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_string);
    ASSERT_TRUE(model != nullptr);

    auto joint = model->getJoint("test_joint");
    ASSERT_TRUE(joint != nullptr);

    ASSERT_TRUE(joint->dynamics != nullptr);
    EXPECT_DOUBLE_EQ(joint->dynamics->damping, 0.0);
    EXPECT_DOUBLE_EQ(joint->dynamics->friction, 0.0);
}

// Test joint dynamics with negative values (should be valid)
TEST_F(URDFJointDynamicsTest, test_parse_joint_dynamics_negative_values)
{
    std::string urdf_string = R"(
<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link"/>
  <link name="child_link"/>
  <joint name="test_joint" type="revolute">
    <parent link="base_link"/>
    <child link="child_link"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57" effort="10" velocity="1"/>
    <dynamics damping="-0.1" friction="-0.2"/>
  </joint>
</robot>
)";

    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_string);
    ASSERT_TRUE(model != nullptr);

    auto joint = model->getJoint("test_joint");
    ASSERT_TRUE(joint != nullptr);

    ASSERT_TRUE(joint->dynamics != nullptr);
    EXPECT_DOUBLE_EQ(joint->dynamics->damping, -0.1);
    EXPECT_DOUBLE_EQ(joint->dynamics->friction, -0.2);
}

// Test joint dynamics with scientific notation
TEST_F(URDFJointDynamicsTest, test_parse_joint_dynamics_scientific_notation)
{
    std::string urdf_string = R"(
<?xml version="1.0"?>
<robot name="test_robot">
  <link name="base_link"/>
  <link name="child_link"/>
  <joint name="test_joint" type="revolute">
    <parent link="base_link"/>
    <child link="child_link"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57" effort="10" velocity="1"/>
    <dynamics damping="1.5e-3" friction="2.1E+2"/>
  </joint>
</robot>
)";

    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_string);
    ASSERT_TRUE(model != nullptr);

    auto joint = model->getJoint("test_joint");
    ASSERT_TRUE(joint != nullptr);

    ASSERT_TRUE(joint->dynamics != nullptr);
    EXPECT_DOUBLE_EQ(joint->dynamics->damping, 0.0015);
    EXPECT_DOUBLE_EQ(joint->dynamics->friction, 210.0);
}