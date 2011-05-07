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

#ifndef GLGEOM_TRANSFORM_SRT_HPP
#define GLGEOM_TRANSFORM_SRT_HPP

#include <glm/glm.hpp>

#include <glgeom/core/point.hpp>
#include <glgeom/core/vector.hpp>

//
// Top-level GLGeom namespace
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
        namespace transform_srt_ns
        {
            namespace detail
            {
                using namespace glgeom::core::point::detail;
                using namespace glgeom::core::vector::detail;

                //===========================================================================//
                //! Scale, rotate, translate transformation
                /*!
                    Transformation that explicitly separates the scale, rotation, and
                    translation components rather than storing them within a single matrix.
                 */
                template <typename T>
                class transform_srt_3t
                {
                public:
                    transform_srt_3t() : scale (1, 1, 1) {}

                    vector3t<T> scale;
                    tquat<T>    rotate;
                    vector3t<T> translate;
                };

                template <typename T>
                point3t<T>
                transform (const point3t<T>& point, const transform_srt_3t<T>& transformation)
                {
                    //
                    // GLM Note:
                    //
                    // Rotating a point (or a vector) via a quaternion is often described as
                    //
                    // v' = q * v * conjugate(q)
                    //
                    // GLM supports both ther operator* and a conjugate() function ***BUT***
                    // the operator* for a glm::vec3 and glm::tquat is overloaded to do the
                    // point transformation completely.  Therefore (q * p) is all that is
                    // needed if the types are correct.
                    //
                    const glm::detail::tvec3<T>&    p = point.vec;
                    const glm::detail::tquat<T>&    q = transformation.rotate;
                    const glm::detail::tvec3<T>&    s = transformation.scale.vec;
                    const glm::detail::tvec3<T>&    t = transformation.translate.vec;

                    return point3t<T>(q * (p * s) + t);
                }
            }

            //
            // Explicitly import only the symbols that belong in the public namespace
            //
            using detail::transform_srt_3t;
            
            typedef transform_srt_3t<float>     transform_srt_3f;
            typedef transform_srt_3t<double>    transform_srt_3d;

            using detail::transform;
        }
    }
    
    using namespace glgeom::prototype::transform_srt_ns;
}

#endif
