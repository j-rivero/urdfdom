#include <gtest/gtest.h>
#include <urdf_model/model.h>
#include <urdf_parser/urdf_parser.h>
#include <urdf_model/pose.h>
#include <cmath>

class URDFPoseAdvancedTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}

    // Helper to compare doubles with tolerance
    bool isNear(double a, double b, double tolerance = 1e-9) {
        return std::abs(a - b) < tolerance;
    }
};

// Test Vector3 operator+ functionality
TEST_F(URDFPoseAdvancedTest, test_vector3_addition_operator)
{
    urdf::Vector3 vec1(1.0, 2.0, 3.0);
    urdf::Vector3 vec2(4.0, 5.0, 6.0);

    urdf::Vector3 result = vec1 + vec2;

    EXPECT_DOUBLE_EQ(result.x, 5.0);
    EXPECT_DOUBLE_EQ(result.y, 7.0);
    EXPECT_DOUBLE_EQ(result.z, 9.0);
}

// Test Vector3 addition with zero vector
TEST_F(URDFPoseAdvancedTest, test_vector3_addition_with_zero)
{
    urdf::Vector3 vec1(1.5, -2.5, 3.7);
    urdf::Vector3 zero_vec(0.0, 0.0, 0.0);

    urdf::Vector3 result = vec1 + zero_vec;

    EXPECT_DOUBLE_EQ(result.x, 1.5);
    EXPECT_DOUBLE_EQ(result.y, -2.5);
    EXPECT_DOUBLE_EQ(result.z, 3.7);
}

// Test Vector3 addition with negative values
TEST_F(URDFPoseAdvancedTest, test_vector3_addition_negative_values)
{
    urdf::Vector3 vec1(-1.0, -2.0, -3.0);
    urdf::Vector3 vec2(1.0, 2.0, 3.0);

    urdf::Vector3 result = vec1 + vec2;

    EXPECT_DOUBLE_EQ(result.x, 0.0);
    EXPECT_DOUBLE_EQ(result.y, 0.0);
    EXPECT_DOUBLE_EQ(result.z, 0.0);
}

// Test Rotation setFromQuaternion functionality
TEST_F(URDFPoseAdvancedTest, test_rotation_set_from_quaternion)
{
    urdf::Rotation rot;

    // Set from unit quaternion (no rotation)
    rot.setFromQuaternion(0.0, 0.0, 0.0, 1.0);

    double roll, pitch, yaw;
    rot.getRPY(roll, pitch, yaw);

    EXPECT_TRUE(isNear(roll, 0.0, 1e-6));
    EXPECT_TRUE(isNear(pitch, 0.0, 1e-6));
    EXPECT_TRUE(isNear(yaw, 0.0, 1e-6));
}

// Test Rotation setFromQuaternion with 90 degree rotation around Z
TEST_F(URDFPoseAdvancedTest, test_rotation_set_from_quaternion_90z)
{
    urdf::Rotation rot;

    // 90 degree rotation around Z axis: quat = (0, 0, sin(pi/4), cos(pi/4))
    double angle = M_PI / 2.0;
    rot.setFromQuaternion(0.0, 0.0, sin(angle/2.0), cos(angle/2.0));

    double roll, pitch, yaw;
    rot.getRPY(roll, pitch, yaw);

    EXPECT_TRUE(isNear(roll, 0.0, 1e-6));
    EXPECT_TRUE(isNear(pitch, 0.0, 1e-6));
    EXPECT_TRUE(isNear(yaw, M_PI/2.0, 1e-6));
}

// Test Rotation normalize functionality with zero quaternion
TEST_F(URDFPoseAdvancedTest, test_rotation_normalize_zero_quaternion)
{
    urdf::Rotation rot;
    rot.x = 0.0;
    rot.y = 0.0;
    rot.z = 0.0;
    rot.w = 0.0;

    rot.normalize();

    // Should default to identity quaternion
    EXPECT_DOUBLE_EQ(rot.x, 0.0);
    EXPECT_DOUBLE_EQ(rot.y, 0.0);
    EXPECT_DOUBLE_EQ(rot.z, 0.0);
    EXPECT_DOUBLE_EQ(rot.w, 1.0);
}

// Test Rotation normalize with unnormalized quaternion
TEST_F(URDFPoseAdvancedTest, test_rotation_normalize_unnormalized)
{
    urdf::Rotation rot;
    rot.x = 2.0;
    rot.y = 0.0;
    rot.z = 0.0;
    rot.w = 2.0;

    rot.normalize();

    // Should be normalized to unit length
    double norm = sqrt(rot.x*rot.x + rot.y*rot.y + rot.z*rot.z + rot.w*rot.w);
    EXPECT_TRUE(isNear(norm, 1.0, 1e-9));

    // Check specific normalized values
    double expected = 1.0 / sqrt(2.0);  // 2.0 / sqrt(8.0)
    EXPECT_TRUE(isNear(rot.x, expected, 1e-9));
    EXPECT_DOUBLE_EQ(rot.y, 0.0);
    EXPECT_DOUBLE_EQ(rot.z, 0.0);
    EXPECT_TRUE(isNear(rot.w, expected, 1e-9));
}

// Test Rotation getQuaternion functionality
TEST_F(URDFPoseAdvancedTest, test_rotation_get_quaternion)
{
    urdf::Rotation rot;
    rot.setFromRPY(0.1, 0.2, 0.3);  // Set known RPY values

    double qx, qy, qz, qw;
    rot.getQuaternion(qx, qy, qz, qw);

    // Verify quaternion is normalized
    double norm = sqrt(qx*qx + qy*qy + qz*qz + qw*qw);
    EXPECT_TRUE(isNear(norm, 1.0, 1e-9));

    // Verify quaternion matches internal representation
    EXPECT_DOUBLE_EQ(qx, rot.x);
    EXPECT_DOUBLE_EQ(qy, rot.y);
    EXPECT_DOUBLE_EQ(qz, rot.z);
    EXPECT_DOUBLE_EQ(qw, rot.w);
}

// Test edge case: extreme RPY values near gimbal lock
TEST_F(URDFPoseAdvancedTest, test_rotation_gimbal_lock_positive_90)
{
    urdf::Rotation rot;
    rot.setFromRPY(0.1, M_PI/2.0, 0.3);  // Pitch = 90 degrees

    double roll, pitch, yaw;
    rot.getRPY(roll, pitch, yaw);

    // At gimbal lock, pitch should be preserved
    EXPECT_TRUE(isNear(pitch, M_PI/2.0, 1e-6));
    // Roll should be zero due to gimbal lock
    EXPECT_TRUE(isNear(roll, 0.0, 1e-6));
}

// Test edge case: extreme RPY values near negative gimbal lock
TEST_F(URDFPoseAdvancedTest, test_rotation_gimbal_lock_negative_90)
{
    urdf::Rotation rot;
    rot.setFromRPY(0.1, -M_PI/2.0, 0.3);  // Pitch = -90 degrees

    double roll, pitch, yaw;
    rot.getRPY(roll, pitch, yaw);

    // At gimbal lock, pitch should be preserved
    EXPECT_TRUE(isNear(pitch, -M_PI/2.0, 1e-6));
    // Roll should be zero due to gimbal lock
    EXPECT_TRUE(isNear(roll, 0.0, 1e-6));
}

// Test Vector3 constructor with parameters
TEST_F(URDFPoseAdvancedTest, test_vector3_constructor_with_params)
{
    urdf::Vector3 vec(1.5, -2.7, 3.14);

    EXPECT_DOUBLE_EQ(vec.x, 1.5);
    EXPECT_DOUBLE_EQ(vec.y, -2.7);
    EXPECT_DOUBLE_EQ(vec.z, 3.14);
}

// Test Rotation constructor with parameters
TEST_F(URDFPoseAdvancedTest, test_rotation_constructor_with_params)
{
    urdf::Rotation rot(0.1, 0.2, 0.3, 0.4);

    EXPECT_DOUBLE_EQ(rot.x, 0.1);
    EXPECT_DOUBLE_EQ(rot.y, 0.2);
    EXPECT_DOUBLE_EQ(rot.z, 0.3);
    EXPECT_DOUBLE_EQ(rot.w, 0.4);
}

// Test Vector3 clear function - covers uncovered inline function
TEST_F(URDFPoseAdvancedTest, test_vector3_clear_function)
{
    urdf::Vector3 vec(1.0, 2.0, 3.0);

    // Test that values are set
    EXPECT_DOUBLE_EQ(vec.x, 1.0);
    EXPECT_DOUBLE_EQ(vec.y, 2.0);
    EXPECT_DOUBLE_EQ(vec.z, 3.0);

    // Test clear function
    vec.clear();
    EXPECT_DOUBLE_EQ(vec.x, 0.0);
    EXPECT_DOUBLE_EQ(vec.y, 0.0);
    EXPECT_DOUBLE_EQ(vec.z, 0.0);
}

// Test Rotation clear function - covers uncovered inline function
TEST_F(URDFPoseAdvancedTest, test_rotation_clear_function)
{
    urdf::Rotation rot(0.1, 0.2, 0.3, 0.4);

    // Test that values are set
    EXPECT_DOUBLE_EQ(rot.x, 0.1);
    EXPECT_DOUBLE_EQ(rot.y, 0.2);
    EXPECT_DOUBLE_EQ(rot.z, 0.3);
    EXPECT_DOUBLE_EQ(rot.w, 0.4);

    // Test clear function - should reset to identity quaternion
    rot.clear();
    EXPECT_DOUBLE_EQ(rot.x, 0.0);
    EXPECT_DOUBLE_EQ(rot.y, 0.0);
    EXPECT_DOUBLE_EQ(rot.z, 0.0);
    EXPECT_DOUBLE_EQ(rot.w, 1.0);
}

// Test Pose clear function - covers uncovered inline function
TEST_F(URDFPoseAdvancedTest, test_pose_clear_function)
{
    urdf::Pose pose;
    pose.position = urdf::Vector3(1.0, 2.0, 3.0);
    pose.rotation = urdf::Rotation(0.1, 0.2, 0.3, 0.4);

    // Test that values are set
    EXPECT_DOUBLE_EQ(pose.position.x, 1.0);
    EXPECT_DOUBLE_EQ(pose.rotation.x, 0.1);

    // Test clear function - should reset both position and rotation
    pose.clear();
    EXPECT_DOUBLE_EQ(pose.position.x, 0.0);
    EXPECT_DOUBLE_EQ(pose.position.y, 0.0);
    EXPECT_DOUBLE_EQ(pose.position.z, 0.0);
    EXPECT_DOUBLE_EQ(pose.rotation.x, 0.0);
    EXPECT_DOUBLE_EQ(pose.rotation.y, 0.0);
    EXPECT_DOUBLE_EQ(pose.rotation.z, 0.0);
    EXPECT_DOUBLE_EQ(pose.rotation.w, 1.0);
}