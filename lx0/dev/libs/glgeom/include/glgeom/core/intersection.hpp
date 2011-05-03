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
                //! Simple binary intersection test: do the primitives intersect or not?
                /*!
                    Tests if the two primitives intersect as fast as can be computed.  If there
                    is an intersection, the intersection point data is not computed.

                    Dev Notes:
                    - This requires some sort of tolerance factor for how near the rays must
                      be to considered to intersect.
                 */
                template <typename P>
                bool intersect (const detail::ray3t<P>& rayA, const detail::ray3t<P>& rayB);


                //---------------------------------------------------------------------------//
                //!
                /*!
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
                        const type t = -(dot(n, o) + d) / cs;
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
                //! Simple binary intersection test: do the primitives intersect or not?
                /*!
                    Tests if the two primitives intersect as fast as can be computed.  If there
                    is an intersection, the intersection point data is not computed.
                 */
                template <typename P>
                bool intersect (const detail::ray3t<P>& ray, const detail::sphere3t<P>& sphere);

                //---------------------------------------------------------------------------//
                //!
                /*!
                 */
                template <typename P>
                void intersect (const detail::ray3t<P>& ray, const detail::sphere3t<P>& sphere, detail::intersect3t<P>& intersect);


                ///////////////////////////////////////////////////////////////////////////////
                //
                // L I N E   I N T E R S E C T I O N S
                //
                ///////////////////////////////////////////////////////////////////////////////
            }

            using   detail::intersect;
            typedef detail::intersect3t<float>  intersect3f;
            typedef detail::intersect3t<double> intersect3d;
        }
    }

    using namespace glgeom::core::intersection;

}

#endif
