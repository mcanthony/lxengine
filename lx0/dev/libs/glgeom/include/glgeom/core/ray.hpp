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

#ifndef GLGEOM_RAY_HPP
#define GLGEOM_RAY_HPP

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
            Dev Notes:
            - Should direction be normalized?  Should that be a condition of validity
              of the class?  Or does unnormalized allow more flexibility?
         */
        template <class P>
        class ray3t
        {
        public:
            typedef P           type;
            typedef point3t<P>  point;
            typedef vector3t<P> vector;

            ray3t () {}
            ray3t (const point& origin_, const vector& direction_) : origin(origin_), direction(direction_) {}
        
            point   origin;
            vector  direction;
        };

    }

    typedef detail::ray3t<float>    ray3f;
    typedef detail::ray3t<double>   ray3d;
}

#endif
