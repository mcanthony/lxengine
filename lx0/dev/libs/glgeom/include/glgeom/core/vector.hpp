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

#include <OGRE/OgreQuaternion.h> //!\todo Remove me!

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
            typedef P       type;

            union
            {
                struct
                {
                    type x, y;
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
            explicit    vector3t (const glm::detail::tvec3<P>& that) : vec(that) {}


            inline type&   operator[] (int i)          { return vec[i]; }
            inline type    operator[] (int i) const    { return vec[i]; }

            vector3t    operator- (void) const { return vector3t( -vec ); }

            union
            {
                struct
                {
                    type x, y, z;
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
        vector3t<T>  operator*       (typename vector3t<T>::type s, const vector3t<T>& v)         
        { 
            return vector3f(s*v.x, s*v.y, s*v.z); 
        }
        
        template <typename T> 
        vector3t<T>  operator*       (const vector3t<T>& v, typename vector3t<T>::type s)         
        { 
            return s * v; 
        }

        template <typename T>
        T dot (const vector3t<T>& a, const vector3t<T>& b)
        {
            return glm::dot(a.vec, b.vec);
        }



        template <typename T> 
        vector3t<T>  abs_                (const vector3t<T>& v)                   { return vector3t<T>(fabs(v.x), fabs(v.y), fabs(v.z)); }
        
        template <typename T> 
        typename vector3t<T>::type     abs_dot             (const vector3t<T>& a, const vector3t<T>& b) { return abs(dot(a, b)); }
        
        template <typename T> 
        vector3t<T>  cross (const vector3t<T>& a, const vector3t<T>& b) 
        { 
            return vector3t<T>( glm::cross(a.vec, b.vec) ); 
        }
        
        template <typename T> 
        vector3t<T>  normalize (const vector3t<T>& a)
        {
            return vector3t<T>( glm::normalize(a.vec) );
        }
        
        template <typename T> 
        typename vector3t<T>::type length (const vector3t<T>& a)
        { 
            return glm::length(a.vec); 
        }
        
        template <typename T> 
        typename vector3t<T>::type     length_squared      (const vector3t<T>& v)                   { return v.x * v.x + v.y * v.y + v.z * v.z; }
        
        template <typename T> 
        bool     is_zero_length      (const vector3t<T>& v)                   { return fabs( length_squared(v) ) <= 10.0f * std::numeric_limits<float>::epsilon(); }
        
        template <typename T> 
        bool     is_unit_length      (const vector3t<T>& v)                   { return fabs( length_squared(v)  - 1.0f ) <= 10.0f * std::numeric_limits<float>::epsilon(); }
        
        template <typename T> 
        bool     is_orthogonal       (const vector3t<T>& u, const vector3t<T>& v) { return fabs( dot(u, v) ) <= 10.0f * std::numeric_limits<float>::epsilon(); }


        template <typename T>
        typename vector3t<T>::type angle_between   (const vector3t<T>& a, const vector3t<T>& b) 
        {
            // Work around a bug in OGRE 1.7.1: angleBetween should be const
            // See http://ogre3d.org/forums/viewtopic.php?f=3&t=56750&p=402888#p402888
            return const_cast<Ogre::Vector3&>(a.ogreVec).angleBetween(b.ogreVec).valueRadians() ; 
        }


        /*!
            Returns true if the vectors point in the same direction or opposite directions (180 degrees apart).
         */
        template <typename T>
        bool is_codirectional (const vector3t<T>& u, const vector3t<T>& v) 
        {
            vector3t<T> un = normalize(u);
            vector3t<T> vn = normalize(v);
            float cosA = dot(u, v);
            return fabs(cosA - 1.0f) <= 10.0f * std::numeric_limits<float>::epsilon(); 
        }

        template <typename T>
        vector3t<T>  
        rotate (const vector3t<T>& v, const vector3t<T>& axis, typename vector3t<T>::type r)
        {
            Ogre::Quaternion q(Ogre::Radian(r), *reinterpret_cast<const Ogre::Vector3*>(&axis));
            Ogre::Vector3 u = q * (*reinterpret_cast<const Ogre::Vector3*>(&v));
            return *reinterpret_cast<vector3t<T>*>(&u);
        }


        //===========================================================================//
        //!
        /*!
         */
        template <class P>
        class vector4t
        {
        public:
            typedef P       type;

            inline type&   operator[] (int i)          { return vec[i]; }
            inline type    operator[] (int i) const    { return vec[i]; }

            union
            {
                struct
                {
                    type x, y, z, w;
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

    using namespace glgeom::detail;
}

#endif
