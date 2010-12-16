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

#include <lx0/cast.hpp>
#include <lx0/tuple3.hpp>
#include <lx0/vector3.hpp>
#include <Ogre/OgreVector3.h>

namespace lx0 { namespace core {

    //=======================================================================//
    //! A single-precision, 3-space point class
    /*!
     */
    class point3 : public detail::base_tuple3
    {
    public:
        point3() {}
        point3 (float x_, float y_, float z_) : base_tuple3(x_, y_, z_) {}
    
        point3      operator-   (void) const    { return point3(-x, -y, -z); }
    };

    namespace detail
    {
        template <> struct cast_is_safe<point3&,       Ogre::Vector3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const point3&, Ogre::Vector3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const point3&, const Ogre::Vector3>   { enum { value = 1 }; }; 
    }

    inline point3   add             (const point3& p, const vector3& v) { return cast<point3&>(p.ogreVec + v.ogreVec); }
    inline point3   sub             (const point3& p, const vector3& v) { return cast<point3&>(p.ogreVec - v.ogreVec); }
    inline vector3  sub             (const point3& p, const point3& q)  { return cast<vector3&>(p.ogreVec - q.ogreVec); }
    inline bool     equal           (const point3& p, const point3& q)  { return p.x == q.x && p.y == q.y && p.z == q.z; }
    inline bool     equiv           (const point3& p, const point3& q, float e) { return (abs(p.x - q.x) < e) && (abs(p.y - q.y) < e) && (abs(p.z - q.z) < e); }  

    inline float    distance                    (const point3& a, const point3& b)  { return a.ogreVec.distance(b.ogreVec); }
    inline float    distance_squared            (const point3& a, const point3& b)  { return a.ogreVec.squaredDistance(b.ogreVec); }
    inline float    distance_to_origin_squared  (const point3& p)                   { return p.x * p.x + p.y * p.y + p.z * p.z; }
    inline float    distance_to_origin          (const point3& p)                   { return sqrtf( distance_to_origin_squared(p) ); }

    inline point3   mid_point       (const point3& a, const point3& b)  { return cast<point3&>(a.ogreVec.midPoint(b.ogreVec)); }

    inline vector3  operator-       (const point3& p, const point3& q)  { return sub(p, q); }
    
}};
