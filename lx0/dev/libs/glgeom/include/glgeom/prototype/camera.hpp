//===========================================================================//
/*
                                   GLGeom

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

    Permission is hereby granted, free of charge, to any person obtaining a 
    copy of this software and associated documentation files (the "Software"), 
    to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, and to permit persons to whom the 
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
    IN THE SOFTWARE.
*/
//===========================================================================//

#ifndef GLGEOM_CAMERA_HPP
#define GLGEOM_CAMERA_HPP

#include <glm/glm.hpp>

#include <glgeom/core/point.hpp>
#include <glgeom/core/vector.hpp>

//
// Main GLGeom namespace
//
namespace glgeom
{
    //
    // Keep this in the prototype namespace since it is work in progress code
    //
    namespace prototype
    {
        //
        // Hide all the implementation details in a unique namespace.  Only
        // explicitly exported symbols should end up in the glgeom::prototype
        // namespace
        //
        namespace camera_ns
        {
            namespace detail
            {
                using namespace glgeom::core::point::detail;
                using namespace glgeom::core::vector::detail;

                //===========================================================================//
                //!
                /*!
                    What is a camera?  
                    
                    A camera is description, in world coordinates, of a view frustum within that world.
                    The view frustum is composed of two similar rectangles - set on the near and far 
                    planes respectively - that are centered about a primary view axis originating at
                    the camera eye point and extending outward.

                    The camera defines it's own space / coordinate system in terms of three vectors:
                    - The view axis 
                    - The vector running along the horizontal width of the frustum plane
                    - The vector running along the vertical width of the frustum plane

                    This space will always be composed of an orthogonormal basis; but, the mapping
                    of axes relies on convention.  By convention, the x and y axes map to the 
                    width and height vectors along the frustum, respectively and the z axis maps to
                    the view axis.  The convention, however, is not universal in the orientation of
                    those mappings as the frusta applies to representation of a physical screen.
                    In most every system, +X maps to moving "right".  However, +Y may indicate moving
                    "up" or "down" the screen and +Z may indicate moving "towards" or "away" from the
                    screen plane.   The choice is arbitrary as the math is the same until the
                    physical mapping to the screen device must occur.

                    GLGeom chooses to follow the OpenGL convention of +X representing right, 
                    +Y representing up, and -Z representing *increasing* depth.  In other words, 
                    the view axis is mapped to the -Z axis.   This is known as a right-handed
                    coordinate system.
                    
                    Dev Notes:
                    - The current setup of public members is lightweight and convenient, but allows for
                      invalid camera setups to be generated without detection.
                 */
                template <typename T>
                class camera3t
                {
                public:
                    typedef T                       type;
                    typedef glm::detail::tquat<T>   quat;
                    typedef point3t<T>              point3;

                    camera3t() : near_plane(.1), far_plane(1000.0), field_of_view(degrees(60)), aspect_ratio(1) {}

                    point3      position;           //! Camera origin / eye-point
                    quat        orientation;        //! Orientation of the center of the line of sight
                    type        near_plane;         //! Distance to the near plane of the view frustum
                    type        far_plane;          //! Distance to the far plane of the view frustum
                    radians     field_of_view;      //! Horizontal field of view in radians
                    type        aspect_ratio;       //! Ratio of the horizontal over the vertical spans of the view rectangle
                };

                //----------------------------------------------------------------------------//
                //! Determine the camera orientation quaternion via a position, target, and "world" up vector
                /*!
                    Computes the quaternion that represents the orientation (rotation) for camera
                    placed at "position" looking at "target" with a given "world_up" vector.

                    The computed orientation assumes a right-handed coordinate system where 
                    the position-to-target vector is mapped to the -Z direction.  The up 
                    direction is mapped to the +Y axis.

                    To the swap handedness to left-handed, such that the position-to-target is
                    mapped to the +Z direction, simply swap the position and target arguments.
                 */
                template <typename T>
                typename glm::detail::tquat<T>
                orientation_from_to_up (const point3t<T>& position, const point3t<T>& target, const vector3t<T>& world_up)
                {
                    // Compute the camera basis in terms of three axes in world coordinates
                    const auto view_axis = normalize(target - position);
                    const auto right = normalize( cross(view_axis, world_up) );
                    const auto up = normalize( cross(right, view_axis) );

                    // Create the new basis.  This creates a right-handed coordinate system where
                    // -z is into the screen / increasing depth.
                    //
                    // A basis transformation from one coordinate system to another via a matrix
                    // is done by setting each column of the matrix to the vectors representing
                    // the new coordinate system (in terms of the old coordinate system).
                    //
                    // The final step is simply to convert that basis transformation matrix into
                    // quaternion form and return the value.
                    //
                    glm::detail::tmat3x3<T> mat(right.vec, up.vec, -view_axis.vec);
                    
                    return glm::quat_cast(mat);
                }

                template <typename T>
                typename glm::detail::tquat<T>
                orientation_dir_up (const vector3t<T>& v, bool right_handed, const vector3t<T>& reference_up)
                {
                    const auto right = normalize( cross(v, reference_up) );
                    const auto up = normalize( cross(right, v) );

                    const T sign = right_handed ? -1 : 1;
                    glm::detail::tmat3x3<T> mat(right.vec, up.vec, sign * v.vec);
                    
                    return glm::quat_cast(mat);
                }

                //! Create a quaternion from an axis-angle representation
                /*
                 */
                template <typename T>
                glm::detail::tquat<T>  
                orientation (const vector3t<T>& axis, glgeom::radians r)
                {
                    const T sin_half_r ( sin(r / 2) );
                    glm::fquat q;
                    q.x = axis.x * sin_half_r;
                    q.y = axis.y * sin_half_r;
                    q.z = axis.z * sin_half_r;
                    q.w = cos(r / 2);
                    return q;
                }

                template <typename T>
                vector3t<T>
                camera_forward_vector (const camera3t<T>& camera)
                {
                    // camera3t is a right-handed coordinate system, so -Z points "forward"
                    //
                    // Also since the transformation is from camera-space to world-space, and the 
                    // orientation represents the transformation from world-to-camera, the inverse
                    // is used: which is hidden in the order of operands in glm's operator* for
                    // quaternions.  (i.e. q * pt == pt * inverse(q)).
                    //
                    return vector3t<T>( glm::normalize( camera.orientation * vector3t<T>(0, 0, -1).vec ) );
                }

                template <typename T>
                vector3t<T>
                camera_right_vector (const camera3t<T>& camera)
                {
                    return vector3t<T>( glm::normalize( camera.orientation * vector3t<T>(1, 0, 0).vec ) );
                }

                template <typename T>
                vector3t<T>
                camera_up_vector (const camera3t<T>& camera)
                {
                    return vector3t<T>( glm::normalize( camera.orientation * vector3t<T>(0, 1, 0).vec ) );
                }

            }

            //
            // Explicitly import only the symbols that belong in the public namespace
            //
            using detail::camera3t;
            
            typedef camera3t<float>     camera3f;
            typedef camera3t<double>    camera3d;

            using detail::orientation_from_to_up;
            using detail::orientation_dir_up;
            using detail::orientation;

            using detail::camera_forward_vector;
            using detail::camera_right_vector;
            using detail::camera_up_vector;
        }
    }
    
    using namespace glgeom::prototype::camera_ns;
}

#endif
