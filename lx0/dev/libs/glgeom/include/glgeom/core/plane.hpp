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

#ifndef GLGEOM_PLANE_HPP
#define GLGEOM_PLANE_HPP

#include <glm/glm.hpp>

#include <glgeom/core/point.hpp>
#include <glgeom/core/vector.hpp>

namespace glgeom
{
    namespace core
    {
        namespace plane
        {
            using namespace glgeom::core::vector::detail;
            using namespace glgeom::core::point::detail;

            namespace detail
            {
                //===========================================================================//
                //!
                /*!
                 */
                template <class P>
                class plane3t
                {
                public:
                    typedef P           type;
                    typedef point3t<P>  point;
                    typedef vector3t<P> vector;

                    plane3t () : a(0), b(0), c(0), d(0) {}
                    plane3t (type a_, type b_, type c_, type d_) : a(a_), b(b_), c(c_), d(d_) {}
                    plane3t (point p, vector n) : normal(n), d( -dot(vector(p), n) ) {}
        
                    union
                    {
                        struct
                        {
                            type a, b, c;
                        };
                        struct
                        {
                            vector  normal;
                        };
                    };
                    type d;
                };

                template <typename T>
                bool    
                valid (const plane3t<T>& plane)
                {
                    return (length(plane.normal) > 0);
                }

            }

            using detail::plane3t;
            typedef detail::plane3t<float>    plane3f;
            typedef detail::plane3t<double>   plane3d;
        }
    }
    
    using namespace glgeom::core::plane;
}

#endif
