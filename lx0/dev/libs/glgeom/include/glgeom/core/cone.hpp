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

#ifndef GLGEOM_CONE_HPP
#define GLGEOM_CONE_HPP

#include <glm/glm.hpp>

#include <glgeom/core/point.hpp>
#include <glgeom/core/vector.hpp>

namespace glgeom
{
    namespace core
    {
        namespace cone
        {
            namespace detail
            {
                using namespace glgeom::core::point::detail;
                using namespace glgeom::core::vector::detail;

                //===========================================================================//
                //!
                /*!
                    Represents a unit sphere (radius = 1) centered at the origin.  

                    This class is largely designed as a helper class and for special cases
                    where a generalized cone can be put in terms of a unit cone at the
                    origin to keep the calculations simple.
                 */
                template <class P>
                class unit_cone3t
                {
                public:
                    typedef P           type;
                    typedef point3t<P>  point;
                    typedef vector3t<P> vector;
                };

                //===========================================================================//
                //!
                /*!
                 */
                template <class P>
                class cone3t
                {
                public:
                    typedef P           type;
                    typedef point3t<P>  point;
                    typedef vector3t<P> vector;

                    point3t             center;
                    vector3t            axis;
                    type                radius;
                };
            }

            typedef detail::cone3t<float>    cone3f;
            typedef detail::cone3t<double>   cone3d;
        }
    }
    
    using namespace glgeom::core::cone;
}

#endif
