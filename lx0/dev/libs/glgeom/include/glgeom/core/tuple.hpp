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

#ifndef GLGEOM_TUPLE_HPP
#define GLGEOM_TUPLE_HPP

#include <glm/glm.hpp>

namespace glgeom
{
    namespace detail
    {
        template <class Precision>
        class tuple2t
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
                    glm::detail::tvec2<Precision> vec;
                };
            };

        protected:
        };

        template <class Precision>
        class tuple3t
        {
        public:
            typedef Precision   type;

            tuple3t (void) { /* use glm default ctor */ }
            tuple3t (type _x, type _y, type _z) : vec(_x, _y, _z) {}

            union
            {
                struct
                {
                    float x, y, z;
                };
                struct
                {
                    glm::detail::tvec3<Precision> vec;
                };
            };

        protected:
        };

        template <class Precision>
        class tuple4t
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
                    glm::detail::tvec4<Precision> vec;
                };
            };

        protected:
        };

        template <typename T>
        tuple3t<T> operator+ (const tuple3t<T>& a, const tuple3t<T>& b)
        {
            return reinterpret_cast< tuple3t<T>& >( a.vec + b.vec );
        }
    }

    typedef detail::tuple2t<float>     tuple2f;
    typedef detail::tuple2t<double>    tuple2d;

    typedef detail::tuple3t<float>     tuple3f;
    typedef detail::tuple3t<double>    tuple3d;

    typedef detail::tuple4t<float>     tuple4f;
    typedef detail::tuple4t<double>    tuple4d;

    namespace detail
    {
        static_assert(sizeof(tuple2f) == sizeof(float) * 2, "tuple2f has unexpected size");
        static_assert(sizeof(tuple2d) == sizeof(double) * 2, "tuple2d has unexpected size");

        static_assert(sizeof(tuple3f) == sizeof(float) * 3, "tuple3f has unexpected size");
        static_assert(sizeof(tuple3d) == sizeof(double) * 3, "tuple3d has unexpected size");

        static_assert(sizeof(tuple4f) == sizeof(float) * 4, "tuple4f has unexpected size");
        static_assert(sizeof(tuple4d) == sizeof(double) * 4, "tuple4d has unexpected size");
    }

}

#endif

