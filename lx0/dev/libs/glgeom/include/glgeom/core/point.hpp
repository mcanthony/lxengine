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

#ifndef GLGEOM_POINT_HPP
#define GLGEOM_POINT_HPP

#include <glm/glm.hpp>

namespace glgeom
{
    namespace detail
    {
        //===========================================================================//
        //!
        /*!
         */
        template <class P>
        class point2t
        {
        public:
            union
            {
                struct
                {
                    float x, y;
                };
                struct
                {
                    glm::detail::tvec2<P> vec;
                };
            };

        protected:
        };

        //===========================================================================//
        //!
        /*!
         */
        template <class P>
        class point3t
        {
        public:
            typedef P       type;

            point3t (void) { /* use glm default ctor */ }
            point3t (type _x, type _y, type _z) : vec(_x, _y, _z) {}

            union
            {
                struct
                {
                    float x, y, z;
                };
                struct
                {
                    glm::detail::tvec3<P> vec;
                };
            };

        protected:
        };

        //===========================================================================//
        //!
        /*!
         */
        template <class P>
        class point4t
        {
        public:
            union
            {
                struct
                {
                    float x, y, z, w;
                };
                struct
                {
                    glm::detail::tvec4<P> vec;
                };
            };

        protected:
        };
    }

    typedef detail::point2t<float>     point2f;
    typedef detail::point2t<double>    point2d;

    typedef detail::point3t<float>     point3f;
    typedef detail::point3t<double>    point3d;

    typedef detail::point4t<float>     point4f;
    typedef detail::point4t<double>    point4d;

    namespace detail
    {
        static_assert(sizeof(point2f) == sizeof(float) * 2, "point2f has unexpected structure size");
        static_assert(sizeof(point2d) == sizeof(double) * 2, "point2d has unexpected structure size");

        static_assert(sizeof(point3f) == sizeof(float) * 3, "point3f has unexpected structure size");
        static_assert(sizeof(point3d) == sizeof(double) * 3, "point3d has unexpected structure size");

        static_assert(sizeof(point4f) == sizeof(float) * 4, "point4f has unexpected structure size");
        static_assert(sizeof(point4d) == sizeof(double) * 4, "point4d has unexpected structure size");
    }
}

#endif
