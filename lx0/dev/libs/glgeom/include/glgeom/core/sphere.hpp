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

#ifndef GLGEOM_SPHERE_HPP
#define GLGEOM_SPHERE_HPP

#include <glm/glm.hpp>

#include <glgeom/core/point.hpp>
#include <glgeom/core/vector.hpp>

namespace glgeom
{
    namespace detail
    {

        //===========================================================================//
        //!
        /*!
            Represents a unit sphere (radius = 1) centered at the origin.  

            This class is largely designed as a helper class and for special cases
            where a generalized sphere can be put in terms of a unit sphere at the
            origin to keep the calculations simple.
         */
        template <class P>
        class unit_sphere3t
        {
        public:
            typedef P           type;
            typedef point3t<P>  point;
            typedef vector3t<P> vector;
        };

        //===========================================================================//
        //!
        /*!
            Dev Notes:
            - Should a sphere always be centered at 0,0,0?

            A sphere class could be defined as always unit and centered at 0,0,0
            and have a scale and translation handled as Decoration patterns on the 
            class.
         */
        template <class P>
        class sphere3t
        {
        public:
            typedef P           type;
            typedef point3t<P>  point;
            typedef vector3t<P> vector;

            point3t             center;
            type                radius;
        };
    }

    typedef detail::plane3t<float>    plane3f;
    typedef detail::plane3t<double>   plane3d;
}

#endif
