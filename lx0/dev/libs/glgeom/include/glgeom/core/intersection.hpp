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

#ifndef GLGEOM_INTERSECTION_HPP
#define GLGEOM_INTERSECTION_HPP

#include <glm/glm.hpp>

#include <glgeom/core/ray.hpp>
#include <glgeom/core/plane.hpp>
#include <glgeom/core/sphere.hpp>

namespace glgeom
{
    namespace core
    {
        namespace intersection
        {
            namespace detail
            {
                using namespace glgeom::core::ray;
                using namespace glgeom::core::sphere;

                //===========================================================================//
                //!
                /*!
                 */
                template <typename P>
                class intersect3t
                {
                public:
                    typedef P           type;
                    typedef point3t<P>  point;
                    typedef vector3t<P> vector;

                    type        distance;
                    point       position;
                    vector      normal;
                };

                ///////////////////////////////////////////////////////////////////////////////
                //
                // R A Y   I N T E R S E C T I O N S
                //
                ///////////////////////////////////////////////////////////////////////////////



                //---------------------------------------------------------------------------//
                //!
                /*!
                    Dev Notes:
                    - How should "ray is on the plane" intersection be better handled?
                      Allow intersect3t to return more complex primitives than points?
                 */
                template <typename P>
                bool  intersect (const ray3t<P>& ray, const plane3t<P>& plane, intersect3t<P>& intersect)
                {
                    typedef P   type;
                    const auto& n = plane.normal;
                    const auto& d = plane.d;
                    const auto& o = reinterpret_cast<const vector3t<P>&>(ray.origin);
                    const auto& v = ray.direction;

                    const type cs = dot (n, v);
                    if (cs == type(0.0))
                    {
                        // The ray lies on the plane: the intersection is the entire ray.
                        return false;
                    }
                    else
                    {
                        const type dot_no = dot(n, o);
                        const type t = -(dot_no + d) / cs;
                        if (t >= type(0.0)) 
                        {
                            intersect.distance = t;
                            intersect.normal = n;
                            intersect.position = ray.origin + v * t;
                            return true;
                        }
                        else 
                        {
                            // The plane is behind the ray
                            return false;
                        }
                    }
                }


                //---------------------------------------------------------------------------//
                //!
                /*!
                    Dev Notes:
                    - Does this handle the ray originating from inside the sphere correctly?
                    - Should this return ALL intersections or just the first?
                 */
                template <typename P>
                bool intersect (const ray3t<P>& ray, const sphere3t<P>& sphere, intersect3t<P>& intersect)
                {
                    typedef P   type;
                    typedef vector3t<P> vector3;

                    // Construct a vector u from the ray's origin to the sphere center
	                const vector3   u(sphere.center - ray.origin);
	                const vector3   v(normalize(ray.direction));

                    // Measure the angle between the ray direction and L indirectly via
                    // the dot product of the two vectors.  
                    //
                    // Do a preliminary check to see if the angle is greater than 90 degrees,
                    // in which case an intersection could not be possible (assuming the 
                    // ray origin is outside the sphere!).
                    const type dotUV = dot(u, v);
	                if (dotUV < type(0.0)) 
                        return false;		         
	
                    // Next compute the distance from the ray to there center of the sphere.
                    // This is easy to compute since the shortest distance between a point
                    // and a line is via the perpendicular of that line: therefore, construct
                    // a right triangle.  u is one side.  dotUV = |u||v|cos(a), and cos(a) = 
                    // adjacent / hypotenuse, |u||v|cos(a) = |v| * adjacent. Next, via 
                    // a^2 + b^2 = c^2, this means b^2 = |u|^2 - dotUV^2/|v|^2, where b is 
                    // the perpendicular distance from the ray to the sphere center.
                    // If |v| is assumed to = 1, then this becomes |u|^2 - (u dot v)^2.
                    //
                    const type d_sqrd = length_squared(u) - dotUV * dotUV;

                    // This provides the shortest distance between the sphere center and
                    // the ray; if this shortest distance is greater than the sphere's 
                    // radius, the algorithm can immediately return that no intersection
                    // occurred.
                    //
                    const type r_sqrd = sphere.radius * sphere.radius;
                    if (d_sqrd > r_sqrd)
                        return false;

                    // Compute the perpendicular distance from the closest point to 
                    // the point on the sphere's surface intersected by the ray.
                    //
                    // Next, construct a triangle from the point closest to the center
                    // to the center, to a point on the sphere's edge along the ray.
                    // The distance from the center to the sphere's edge on the triangle
                    // must be the radius of the sphere (by definition of a sphere).
                    // The distance from the center to the closest point is already known
                    // to be "d" as computed above.  Also, since the closest point is
                    // necessarily perpendicular to the ray, the Pythagorian theorem can
                    // be used again to compute the distance from the closest point to
                    // point on the ray lying on the sphere's surface.
                    // 
                    // 
	                const type dp = sqrt(r_sqrd - d_sqrd);	

                    // (u dot v) also represents the projection of u onto v (or vice-versa).
                    // If v is assumed to be normalized, this means that (u dot v) represents
                    // the length of u when projected onto the direction of v.  In this particular 
                    // case where u is the vector from the ray origin to the sphere center, 
                    // (u dot v) is the length along the ray to the point perpendicular to the 
                    // sphere center.
                    //
                    // And since the distance from that point to the sphere's edge is known,
                    // the distance along the ray can be directly computed by combining these
                    // values.
                    //
                    type dr[2];
	                dr[0] = dotUV - dp;
	                dr[1] = dotUV + dp;
	
                    auto hit = [&] (type d) -> bool
                    {
                        intersect.distance = d;
                        intersect.position = ray.origin + d * ray.direction;
                        intersect.normal = normalize(intersect.position - sphere.center);
                        return true;
                    };

                    if (dr[0] < 0)
                        if (dr[1] < 0)
                            return false;
                        else
                            return hit(dr[1]);
                    else
                        if (dr[1] < 0)
                            return hit(dr[0]);
                        else
                            return hit(std::min(dr[0], dr[1]));
                }


                ///////////////////////////////////////////////////////////////////////////////
                //
                // L I N E   I N T E R S E C T I O N S
                //
                ///////////////////////////////////////////////////////////////////////////////
            }

            using   detail::intersect3t;
            typedef detail::intersect3t<float>  intersect3f;
            typedef detail::intersect3t<double> intersect3d;
        }
    }

    using namespace glgeom::core::intersection;

}

#endif
