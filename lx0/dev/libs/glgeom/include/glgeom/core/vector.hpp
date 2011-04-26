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
        //===========================================================================//
        //!
        /*!
         */
        template <class P>
        class vector2t
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
        class vector3t
        {
        public:
            typedef P       type;

            vector3t (void) { /* use glm default ctor */ }
            vector3t (type _x, type _y, type _z) : vec(_x, _y, _z) {}

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


        template <typename T>
        vector3t<T> operator+ (const vector3t<T>& a, const vector3t<T>& b)
        {
            return reinterpret_cast< vector3t<T>& >( a.vec + b.vec );
        }

        template <typename T>
        T dot (const vector3t<T>& a, const vector3t<T>& b)
        {
            return glm::dot(a.vec, b.vec);
        }

        //===========================================================================//
        //!
        /*!
         */
        template <class P>
        class vector4t
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

    typedef detail::vector2t<float>     vector2f;
    typedef detail::vector2t<double>    vector2d;

    typedef detail::vector3t<float>     vector3f;
    typedef detail::vector3t<double>    vector3d;

    typedef detail::vector4t<float>     vector4f;
    typedef detail::vector4t<double>    vector4d;

    namespace detail
    {
        static_assert(sizeof(vector2f) == sizeof(float) * 2, "vector2f has unexpected structure size");
        static_assert(sizeof(vector2d) == sizeof(double) * 2, "vector2d has unexpected structure size");

        static_assert(sizeof(vector3f) == sizeof(float) * 3, "vector3f has unexpected structure size");
        static_assert(sizeof(vector3d) == sizeof(double) * 3, "vector3d has unexpected structure size");

        static_assert(sizeof(vector4f) == sizeof(float) * 4, "vector4f has unexpected structure size");
        static_assert(sizeof(vector4d) == sizeof(double) * 4, "vector4d has unexpected structure size");
    }
}

#endif
