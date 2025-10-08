
#ifndef URDF_RUST_H
#define URDF_RUST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

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

// ====================
// ROTATION FUNCTIONS
// ====================

// Create a new Rotation with specified quaternion components
Rotation urdf_rotation_new(double x, double y, double z, double w);

// Create an identity Rotation (0, 0, 0, 1)
Rotation urdf_rotation_identity(void);

// Clear/reset a Rotation to identity
void urdf_rotation_clear(Rotation* rot);

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
// GEOMETRY DATA STRUCTURES
// ====================

// Geometry type enumeration - matches C++ urdf::Geometry enum
typedef enum {
    URDF_GEOMETRY_SPHERE = 0,
    URDF_GEOMETRY_BOX = 1,
    URDF_GEOMETRY_CYLINDER = 2,
    URDF_GEOMETRY_MESH = 3
} UrdfGeometryType;

// Opaque pointer to Rust Geometry - managed internally
typedef void* UrdfGeometry;

// ====================
// GEOMETRY FUNCTIONS
// ====================

// Create a box geometry with dimensions (x, y, z)
UrdfGeometry urdf_geometry_create_box(double x, double y, double z);

// Create a sphere geometry with radius
UrdfGeometry urdf_geometry_create_sphere(double radius);

// Create a cylinder geometry with radius and length
UrdfGeometry urdf_geometry_create_cylinder(double radius, double length);

// Create a mesh geometry with filename and optional scale
UrdfGeometry urdf_geometry_create_mesh(const char* filename, const double scale[3]);

// Destroy/free a geometry object
void urdf_geometry_destroy(UrdfGeometry geometry);

// Get the type of a geometry object
int urdf_geometry_get_type(UrdfGeometry geometry);

// Box-specific functions
int urdf_geometry_box_get_dimensions(UrdfGeometry geometry, double dimensions[3]);

// Sphere-specific functions
double urdf_geometry_sphere_get_radius(UrdfGeometry geometry);

// Cylinder-specific functions
double urdf_geometry_cylinder_get_radius(UrdfGeometry geometry);
double urdf_geometry_cylinder_get_length(UrdfGeometry geometry);

// Mesh-specific functions
int urdf_geometry_mesh_get_filename(UrdfGeometry geometry, char* buffer, size_t buffer_size);
int urdf_geometry_mesh_get_scale(UrdfGeometry geometry, double scale[3]);

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
