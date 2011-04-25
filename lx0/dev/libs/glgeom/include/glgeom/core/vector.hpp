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

#ifndef GLGEOM_VECTOR_HPP
#define GLGEOM_VECTOR_HPP

#include <glm/glm.hpp>

namespace glgeom
{
    namespace detail
    {
        template <class Precision>
        class tvector2
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
        class tvector3
        {
        public:
            typedef Precision   type;

            tvector3 (void) { /* use glm default ctor */ }
            tvector3 (type _x, type _y, type _z) : vec(_x, _y, _z) {}

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
        class tvector4
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
        tvector3<T> operator+ (const tvector3<T>& a, const tvector3<T>& b)
        {
            return reinterpret_cast< tvector3<T>& >( a.vec + b.vec );
        }

        template <typename T>
        T dot (const tvector3<T>& a, const tvector3<T>& b)
        {
            return glm::dot(a.vec, b.vec);
        }
    }

    typedef detail::tvector2<float>     vector2f;
    typedef detail::tvector2<double>    vector2d;

    typedef detail::tvector3<float>     vector3f;
    typedef detail::tvector3<double>    vector3d;

    typedef detail::tvector4<float>     vector4f;
    typedef detail::tvector4<double>    vector4d;

    namespace detail
    {
        static_assert(sizeof(vector2f) == sizeof(float) * 2, "vector2f has unexpected size");
        static_assert(sizeof(vector2d) == sizeof(double) * 2, "vector2d has unexpected size");

        static_assert(sizeof(vector3f) == sizeof(float) * 3, "vector3f has unexpected size");
        static_assert(sizeof(vector3d) == sizeof(double) * 3, "vector3d has unexpected size");

        static_assert(sizeof(vector4f) == sizeof(float) * 4, "vector4f has unexpected size");
        static_assert(sizeof(vector4d) == sizeof(double) * 4, "vector4d has unexpected size");
    }
}

#endif
