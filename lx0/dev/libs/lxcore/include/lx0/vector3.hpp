//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010 athile@athile.net (http://www.athile.net)

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

#pragma once

#include <limits>

#include <lx0/cast.hpp>
#include <lx0/tuple3.hpp>
#include <Ogre/OgreVector3.h>

namespace lx0 { namespace core {

    //=======================================================================//
    //! A single-precision, 3-space vector class
    /*!
     */
    class vector3 : public detail::base_tuple3
    {
    public:
        vector3() {}
        vector3 (float x_, float y_, float z_) : base_tuple3(x_, y_, z_) {}
    
        vector3     operator-   (void) const    { return vector3(-x, -y, -z); }
    };

    namespace detail
    {
        template <> struct cast_is_safe<vector3&,       Ogre::Vector3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const vector3&, Ogre::Vector3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const vector3&, const Ogre::Vector3>   { enum { value = 1 }; }; 
    }

    inline vector3  abs_                (const vector3& v)                   { return vector3(fabs(v.x), fabs(v.y), fabs(v.z)); }

    inline float    dot                 (const vector3& a, const vector3& b) { return a.ogreVec.dotProduct(b.ogreVec); }
    inline float    abs_dot             (const vector3& a, const vector3& b) { return abs(dot(a, b)); }
    inline vector3  cross               (const vector3& a, const vector3& b) { return cast<vector3&>(a.ogreVec.crossProduct(b.ogreVec)); }
    inline vector3  normalize           (const vector3& a)                   { vector3 t = a; t.ogreVec.normalise(); return t; }
    inline float    length              (const vector3& a)                   { return a.ogreVec.length(); }
    inline float    length_squared      (const vector3& v)                   { return v.x * v.x + v.y * v.y + v.z * v.z; }
    inline bool     is_zero_length      (const vector3& v)                   { return fabs( length_squared(v) ) <= 10.0f * std::numeric_limits<float>::epsilon(); }
    inline bool     is_unit_length      (const vector3& v)                   { return fabs( length_squared(v)  - 1.0f ) <= 10.0f * std::numeric_limits<float>::epsilon(); }
    inline bool     is_orthogonal       (const vector3& u, const vector3& v) { return fabs( dot(u, v) ) <= 10.0f * std::numeric_limits<float>::epsilon(); }
           bool     is_codirectional    (const vector3& u, const vector3& v); 

    inline float    angle_between   (const vector3& a, const vector3& b) 
    {
        // Work around a bug in OGRE 1.7.1: angleBetween should be const
        // See http://ogre3d.org/forums/viewtopic.php?f=3&t=56750&p=402888#p402888
        return const_cast<Ogre::Vector3&>(a.ogreVec).angleBetween(b.ogreVec).valueRadians() ; 
    }

    inline vector3  operator*       (float s, const vector3& v)         { return vector3(s*v.x, s*v.y, s*v.z); }
    inline vector3  operator*       (const vector3& v, float s)         { return s * v; }


    

}};
