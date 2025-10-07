/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2008, Willow Garage, Inc.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

#include <gtest/gtest.h>
#include <urdf_model/model.h>
#include <urdf_parser/urdf_parser.h>
#include <string>
#include <memory>

class URDFPointerCastTest : public ::testing::Test
{
protected:
    std::string urdf_with_geometry;

    virtual void SetUp()
    {
        // URDF with different geometry types to test polymorphism
        urdf_with_geometry = R"(
        <?xml version="1.0"?>
        <robot name="test_robot">
          <link name="base_link"/>

          <link name="link_with_box">
            <visual>
              <geometry>
                <box size="1 2 3"/>
              </geometry>
            </visual>
          </link>

          <link name="link_with_sphere">
            <visual>
              <geometry>
                <sphere radius="0.5"/>
              </geometry>
            </visual>
          </link>

          <link name="link_with_cylinder">
            <visual>
              <geometry>
                <cylinder radius="0.3" length="1.0"/>
              </geometry>
            </visual>
          </link>

          <link name="link_with_mesh">
            <visual>
              <geometry>
                <mesh filename="test.dae"/>
              </geometry>
            </visual>
          </link>

          <joint name="joint1" type="fixed">
            <parent link="base_link"/>
            <child link="link_with_box"/>
          </joint>

          <joint name="joint2" type="fixed">
            <parent link="base_link"/>
            <child link="link_with_sphere"/>
          </joint>

          <joint name="joint3" type="fixed">
            <parent link="base_link"/>
            <child link="link_with_cylinder"/>
          </joint>

          <joint name="joint4" type="fixed">
            <parent link="base_link"/>
            <child link="link_with_mesh"/>
          </joint>
        </robot>
        )";
    }
};

// Test dynamic_pointer_cast for Box geometry
TEST_F(URDFPointerCastTest, test_dynamic_cast_box)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_with_geometry);
    ASSERT_NE(model, nullptr);

    urdf::LinkConstSharedPtr link = model->getLink("link_with_box");
    ASSERT_NE(link, nullptr);
    ASSERT_TRUE(link->visual_array.size() > 0);

    urdf::VisualSharedPtr visual = link->visual_array[0];
    ASSERT_NE(visual, nullptr);
    ASSERT_NE(visual->geometry, nullptr);

    // Test dynamic cast to Box
    urdf::BoxSharedPtr box = urdf::dynamic_pointer_cast<urdf::Box>(visual->geometry);
    ASSERT_NE(box, nullptr);
    EXPECT_EQ(box->dim.x, 1.0);
    EXPECT_EQ(box->dim.y, 2.0);
    EXPECT_EQ(box->dim.z, 3.0);

    // Test dynamic cast to wrong type should return nullptr
    urdf::SphereSharedPtr sphere = urdf::dynamic_pointer_cast<urdf::Sphere>(visual->geometry);
    EXPECT_EQ(sphere, nullptr);
}

// Test dynamic_pointer_cast for Sphere geometry
TEST_F(URDFPointerCastTest, test_dynamic_cast_sphere)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_with_geometry);
    ASSERT_NE(model, nullptr);

    urdf::LinkConstSharedPtr link = model->getLink("link_with_sphere");
    ASSERT_NE(link, nullptr);
    ASSERT_TRUE(link->visual_array.size() > 0);

    urdf::VisualSharedPtr visual = link->visual_array[0];
    ASSERT_NE(visual, nullptr);
    ASSERT_NE(visual->geometry, nullptr);

    // Test dynamic cast to Sphere
    urdf::SphereSharedPtr sphere = urdf::dynamic_pointer_cast<urdf::Sphere>(visual->geometry);
    ASSERT_NE(sphere, nullptr);
    EXPECT_EQ(sphere->radius, 0.5);

    // Test dynamic cast to wrong type should return nullptr
    urdf::BoxSharedPtr box = urdf::dynamic_pointer_cast<urdf::Box>(visual->geometry);
    EXPECT_EQ(box, nullptr);
}

// Test dynamic_pointer_cast for Cylinder geometry
TEST_F(URDFPointerCastTest, test_dynamic_cast_cylinder)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_with_geometry);
    ASSERT_NE(model, nullptr);

    urdf::LinkConstSharedPtr link = model->getLink("link_with_cylinder");
    ASSERT_NE(link, nullptr);
    ASSERT_TRUE(link->visual_array.size() > 0);

    urdf::VisualSharedPtr visual = link->visual_array[0];
    ASSERT_NE(visual, nullptr);
    ASSERT_NE(visual->geometry, nullptr);

    // Test dynamic cast to Cylinder
    urdf::CylinderSharedPtr cylinder = urdf::dynamic_pointer_cast<urdf::Cylinder>(visual->geometry);
    ASSERT_NE(cylinder, nullptr);
    EXPECT_EQ(cylinder->radius, 0.3);
    EXPECT_EQ(cylinder->length, 1.0);

    // Test dynamic cast to wrong type should return nullptr
    urdf::MeshSharedPtr mesh = urdf::dynamic_pointer_cast<urdf::Mesh>(visual->geometry);
    EXPECT_EQ(mesh, nullptr);
}

// Test dynamic_pointer_cast for Mesh geometry
TEST_F(URDFPointerCastTest, test_dynamic_cast_mesh)
{
    urdf::ModelInterfaceSharedPtr model = urdf::parseURDF(urdf_with_geometry);
    ASSERT_NE(model, nullptr);

    urdf::LinkConstSharedPtr link = model->getLink("link_with_mesh");
    ASSERT_NE(link, nullptr);
    ASSERT_TRUE(link->visual_array.size() > 0);

    urdf::VisualSharedPtr visual = link->visual_array[0];
    ASSERT_NE(visual, nullptr);
    ASSERT_NE(visual->geometry, nullptr);

    // Test dynamic cast to Mesh
    urdf::MeshSharedPtr mesh = urdf::dynamic_pointer_cast<urdf::Mesh>(visual->geometry);
    ASSERT_NE(mesh, nullptr);
    EXPECT_EQ(mesh->filename, "test.dae");

    // Test dynamic cast to wrong type should return nullptr
    urdf::CylinderSharedPtr cylinder = urdf::dynamic_pointer_cast<urdf::Cylinder>(visual->geometry);
    EXPECT_EQ(cylinder, nullptr);
}