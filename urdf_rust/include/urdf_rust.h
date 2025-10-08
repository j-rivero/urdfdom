
#ifndef URDF_RUST_H
#define URDF_RUST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// ====================
// POSE DATA STRUCTURES
// ====================

// Vector3 struct - represents a 3D vector with x, y, z components
typedef struct {
    double x;
    double y;
    double z;
} Vector3;

// Rotation struct - represents rotation using quaternion (x, y, z, w)
typedef struct {
    double x;
    double y;
    double z;
    double w;
} Rotation;

// Pose struct - combines position (Vector3) and rotation (Rotation)
typedef struct {
    Vector3 position;
    Rotation rotation;
} Pose;

// ====================
// VECTOR3 FUNCTIONS
// ====================

// Create a new Vector3 with specified components
Vector3 urdf_vector3_new(double x, double y, double z);

// Create a zero Vector3 (0, 0, 0)
Vector3 urdf_vector3_zero(void);

// Clear/reset a Vector3 to zero
void urdf_vector3_clear(Vector3* vec);

// Initialize Vector3 from string "x y z"
// Returns 0 on success, negative on error
int urdf_vector3_init(Vector3* vec, const char* vector_str);

// Add two Vector3 instances
Vector3 urdf_vector3_add(Vector3 a, Vector3 b);

// ====================
// ROTATION FUNCTIONS
// ====================

// Create a new Rotation with specified quaternion components
Rotation urdf_rotation_new(double x, double y, double z, double w);

// Create an identity Rotation (no rotation)
Rotation urdf_rotation_identity(void);

// Clear/reset a Rotation to identity
void urdf_rotation_clear(Rotation* rot);

// Get quaternion components from Rotation
void urdf_rotation_get_quaternion(const Rotation* rot, double* x, double* y, double* z, double* w);

// Get Roll-Pitch-Yaw angles from Rotation (in radians)
void urdf_rotation_get_rpy(const Rotation* rot, double* roll, double* pitch, double* yaw);

// Set rotation from quaternion components
void urdf_rotation_set_from_quaternion(Rotation* rot, double x, double y, double z, double w);

// Set rotation from Roll-Pitch-Yaw angles (in radians)
void urdf_rotation_set_from_rpy(Rotation* rot, double roll, double pitch, double yaw);

// Initialize Rotation from string "roll pitch yaw"
// Returns 0 on success, negative on error
int urdf_rotation_init(Rotation* rot, const char* rotation_str);

// Normalize quaternion to unit length
void urdf_rotation_normalize(Rotation* rot);

// Get inverse rotation
Rotation urdf_rotation_get_inverse(const Rotation* rot);

// Multiply two rotations (quaternion multiplication)
Rotation urdf_rotation_multiply(Rotation r1, Rotation r2);

// Rotate a Vector3 by this rotation
Vector3 urdf_rotation_multiply_vector3(Rotation rot, Vector3 vec);

// ====================
// POSE FUNCTIONS
// ====================

// Create a new Pose with specified position and rotation
Pose urdf_pose_new(Vector3 position, Rotation rotation);

// Create an identity Pose (zero position, identity rotation)
Pose urdf_pose_identity(void);

// Clear/reset a Pose to identity
void urdf_pose_clear(Pose* pose);

// Transform a Vector3 by this pose (rotate then translate)
Vector3 urdf_pose_transform_vector(const Pose* pose, Vector3 vec);

// Get inverse pose
Pose urdf_pose_get_inverse(const Pose* pose);

// Multiply two poses (combine transformations)
Pose urdf_pose_multiply(Pose p1, Pose p2);

// ====================
// LEGACY FUNCTIONS
// ====================

// Test function - will be replaced by actual URDF functions
int urdf_rust_test_function(void);

// Placeholder for future URDF parsing functions
typedef struct UrdfModel UrdfModel;

#ifdef __cplusplus
}
#endif

#endif // URDF_RUST_H
