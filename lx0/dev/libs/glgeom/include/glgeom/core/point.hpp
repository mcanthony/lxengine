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

#include <glgeom/core/vector.hpp>

namespace glgeom
{
    namespace core
    {
        namespace point
        {
            namespace detail
            {
                using namespace glgeom::core::vector;
                using namespace glgeom::core::vector::detail;

                //===========================================================================//
                //!
                /*!
                 */
                template <typename P>
                class point2t
                {
                public:
                    typedef P               type;
                    typedef point2t<P>      point2;
                    typedef vector2t<P>     vector2;

                    inline type&   operator[] (int i)          { return vec[i]; }
                    inline type    operator[] (int i) const    { return vec[i]; }

                    void operator+=  (const vector2& v) { vec += v.vec; }
                    void operator-=  (const vector2& v) { vec += v.vec; }

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
                template <typename P>
                class point3t
                {
                public:
                    typedef P               type;
                    typedef point3t<P>      point3;
                    typedef vector3t<P>     vector3;

                                point3t (void) { /* use glm default ctor */ }
                                point3t (type _x, type _y, type _z) : vec(_x, _y, _z) {}
                    explicit    point3t (const glm::detail::tvec3<P>& that) : vec(that) {}

                    inline type&   operator[] (int i)          { return vec[i]; }
                    inline type    operator[] (int i) const    { return vec[i]; }

                    void operator+=  (const vector3& v) { vec += v.vec; }
                    void operator-=  (const vector3& v) { vec += v.vec; }

                    point3t operator- (void) const { return point3t(-vec); }

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
                point3t<T> operator+ (const point3t<T>& a, const vector3t<T>& b)
                {
                    return reinterpret_cast< point3t<T>& >( a.vec + b.vec );
                }

                template <typename T>
                point3t<T> operator- (const point3t<T>& a, const vector3t<T>& b)
                {
                    return reinterpret_cast< point3t<T>& >( a.vec - b.vec );
                }

                template <typename T>
                vector3t<T> operator- (const point3t<T>& a, const point3t<T>& b)
                {
                    return reinterpret_cast< vector3t<T>& >( a.vec - b.vec );
                }


                /*inline point3   add             (const point3& p, const vector3& v) { return cast<point3&>(p.ogreVec + v.ogreVec); }
                inline point3   sub             (const point3& p, const vector3& v) { return cast<point3&>(p.ogreVec - v.ogreVec); }
                inline vector3  sub             (const point3& p, const point3& q)  { return cast<vector3&>(p.ogreVec - q.ogreVec); }
                inline bool     equal           (const point3& p, const point3& q)  { return p.x == q.x && p.y == q.y && p.z == q.z; }
                inline bool     equiv           (const point3& p, const point3& q, float e) { return (abs(p.x - q.x) < e) && (abs(p.y - q.y) < e) && (abs(p.z - q.z) < e); }  */

                template <typename T>
                inline T    distance                    (const point3t<T>& a, const point3t<T>& b)  
                { 
                    return glm::distance(a.vec, b.vec); 
                }
        
                template <typename T>
                inline T    distance_squared            (const point3t<T>& a, const point3t<T>& b)  
                { 
                    return glm::dot(a.vec, b.vec); 
                }
        
                template <typename T>
                inline T    distance_to_origin_squared  (const point3t<T>& p)
                { 
                    return p.x * p.x + p.y * p.y + p.z * p.z; 
                }
        
                template <typename T>
                inline T    distance_to_origin          (const point3t<T>& p)                   
                { 
                    return sqrtf( distance_to_origin_squared(p) ); 
                }

                template <typename T>
                inline point3t<T>   mid_point       (const point3t<T>& a, const point3t<T>& b)  
                { 
                    return point3t<T>( (b.vec - a.vec) / 2 );
                }


                //===========================================================================//
                //!
                /*!
                 */
                template <typename P>
                class point4t
                {
                public:
                    typedef P               type;
                    typedef point4t<P>      point4;
                    typedef vector4t<P>     vector4;

                    inline type&   operator[] (int i)          { return vec[i]; }
                    inline type    operator[] (int i) const    { return vec[i]; }

                    void operator+=  (const vector4& v) { vec += v.vec; }
                    void operator-=  (const vector4& v) { vec += v.vec; }

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

            using detail::point2t;
            using detail::point3t;
            using detail::point4t;

            typedef detail::point2t<float>     point2f;
            typedef detail::point2t<double>    point2d;

            typedef detail::point3t<float>     point3f;
            typedef detail::point3t<double>    point3d;

            typedef detail::point4t<float>     point4f;
            typedef detail::point4t<double>    point4d;

            using detail::operator+;
            using detail::operator-;

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
    }

    using namespace glgeom::core::point;
}

#endif
