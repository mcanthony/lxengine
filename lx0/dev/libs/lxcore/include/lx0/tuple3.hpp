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
#include <Ogre/OgreVector3.h>

namespace lx0 { namespace core {


    namespace detail
    {
        /*!
            Base class used to consolidate common code in
            3-tuple classes.
         */
        class base_tuple3
        {
        public:
            base_tuple3() : x(0.0f), y(0.0f), z(0.0f) {}
            base_tuple3(float x0, float y0, float z0) : x(x0), y(y0), z(z0) {}

            inline float&   operator[] (int i)          { return elem[i]; }
            inline float    operator[] (int i) const    { return elem[i]; }
        
            union
            {
                struct 
                {
                    float x, y, z;
                };
                
                float elem[3];
                
                // The anonymous struct allows a class with a non-trivial ctor
                // to be used inside the union.  A compiler with full C++x0 
                // support should not require this.
                struct 
                { 
                    Ogre::Vector3 ogreVec; 
                };
            };    
        };

        class base_tuple4
        {
        public:
            inline float&   operator[] (int i)          { return elem[i]; }
            inline float    operator[] (int i) const    { return elem[i]; }

            union 
            {
                struct
                {
                    float x, y, z, w;
                };
                float elem[4];
            };
        };
    }

    inline void     set         (detail::base_tuple3& t, float x, float y, float z)             { t.x = x; t.y = y; t.z = z; }
    inline void     set         (detail::base_tuple4& t, float x, float y, float z, float w)    { t.x = x; t.y = y; t.z = z; t.w = w; }
    
    //=======================================================================//
    //!
    /*!
     */
    class tuple3 
        : public detail::base_tuple3
    {
    public: 
    };

    namespace detail
    {
        template <> struct cast_is_safe<tuple3&,        Ogre::Vector3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const tuple3&,  Ogre::Vector3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const tuple3&,  const Ogre::Vector3>   { enum { value = 1 }; }; 
    }
    
    inline tuple3   add             (const tuple3& a, const tuple3& b)      { return cast<tuple3&>(a.ogreVec + b.ogreVec); }
    inline tuple3   sub             (const tuple3& a, const tuple3& b)      { return cast<tuple3&>(a.ogreVec - b.ogreVec); }
    
    //=======================================================================//
    //!
    /*!
     */
    class tuple4
        : public detail::base_tuple4
    {
    public: 
    };

    


    class point3;
    class vector3;

    namespace detail
    {
        template <> struct cast_is_safe<tuple3&,        point3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const tuple3&,  point3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const tuple3&,  const point3>   { enum { value = 1 }; }; 
        template <> struct cast_is_safe<tuple3&,        vector3>        { enum { value = 1 }; };
        template <> struct cast_is_safe<const tuple3&,  vector3>        { enum { value = 1 }; };
        template <> struct cast_is_safe<const tuple3&,  const vector3>  { enum { value = 1 }; };
        
        template <> struct cast_is_safe<point3&,        tuple3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const point3&,  tuple3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const point3&,  const tuple3>   { enum { value = 1 }; };
        template <> struct cast_is_safe<point3&,        vector3>        { enum { value = 1 }; };
        template <> struct cast_is_safe<const point3&,  vector3>        { enum { value = 1 }; };
        template <> struct cast_is_safe<const point3&,  const vector3>  { enum { value = 1 }; };
        
        template <> struct cast_is_safe<vector3&,       tuple3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const vector3&, tuple3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const vector3&, const tuple3>   { enum { value = 1 }; };
        template <> struct cast_is_safe<vector3&,       point3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const vector3&, point3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const vector3&, const point3>   { enum { value = 1 }; };      
    }

}};