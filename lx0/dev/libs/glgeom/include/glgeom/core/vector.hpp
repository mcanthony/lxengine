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
#include <glm/gtc/quaternion.hpp>

namespace glgeom
{
    namespace core
    {
        namespace point
        {
            namespace detail
            {
                template <typename T> class point3t;
            }
        }

        namespace vector
        {
            namespace detail
            {
                using glgeom::core::point::detail::point3t;

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

                                    vector3t    (void) { /* use glm default ctor */ }
                                    vector3t    (type _x, type _y, type _z) : vec(_x, _y, _z) {}
                    explicit        vector3t    (const glm::detail::tvec3<P>& that) : vec(that) {}
                    explicit        vector3t    (const point3t<P>& p);
                    

                    void            operator=   (const glm::detail::tvec3<P>& that) { vec = that; }

                    inline type&    operator[]  (int i)          { return vec[i]; }
                    inline type     operator[]  (int i) const    { return vec[i]; }

                    vector3t        operator-   (void) const { return vector3t( -vec ); }

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


                /*!
                    Add two vectors together.

                    Dev Notes:
                    - Does this operation actually make sense for a vector when treated as a 
                        directed length?  
                    */
                template <typename T>
                vector3t<T> operator+ (const vector3t<T>& a, const vector3t<T>& b)
                {
                    return reinterpret_cast< vector3t<T>& >( a.vec + b.vec );
                }

                /*!
                    Scale a vector.
                    */
                template <typename T>
                vector3t<T>  operator*       (typename vector3t<T>::type s, const vector3t<T>& v)         
                { 
                    return vector3t<T>(s*v.x, s*v.y, s*v.z); 
                }
        
                /*!
                    Scale a vector.
                    */
                template <typename T> 
                vector3t<T>  operator*       (const vector3t<T>& v, typename vector3t<T>::type s)         
                { 
                    return s * v; 
                }

                //! Return the dot product of two vectors.
                /*!
                    a dot b = |a||b|cos 0
                    */
                template <typename T>
                T dot (const vector3t<T>& a, const vector3t<T>& b)
                {
                    return glm::dot(a.vec, b.vec);
                }



                template <typename T> 
                vector3t<T>  abs_ (const vector3t<T>& v)
                { 
                    return vector3t<T>(fabs(v.x), fabs(v.y), fabs(v.z)); 
                }
        
                //! Returns the absolute value of the dot product
                /*!
                    Dev Notes:
                    - Is this method actually needed?  Is abs_dot(a, b) really better than abs(dot(a, b))
                    */
                template <typename T> 
                typename vector3t<T>::type 
                abs_dot (const vector3t<T>& a, const vector3t<T>& b) 
                { 
                    return abs(dot(a, b)); 
                }
        
                //! Returns the cross product of two vectors
                /*!
                    */
                template <typename T> 
                vector3t<T>  cross (const vector3t<T>& a, const vector3t<T>& b) 
                { 
                    return vector3t<T>( glm::cross(a.vec, b.vec) ); 
                }
        
                //! Returns a normalized equivalent of the input vector
                /*!
                    */
                template <typename T> 
                vector3t<T>  normalize (const vector3t<T>& a)
                {
                    return vector3t<T>( glm::normalize(a.vec) );
                }
        
                //! Returns the length of the vector
                /*!
                    */
                template <typename T> 
                typename vector3t<T>::type length (const vector3t<T>& a)
                { 
                    return glm::length(a.vec); 
                }
        
                //! Returns the length squared of the vector
                /*!
                    Equivalent to dot(v, v)
                    */
                template <typename T> 
                typename vector3t<T>::type length_squared (const vector3t<T>& v)
                { 
                    return v.x * v.x + v.y * v.y + v.z * v.z; 
                }
        
                //! Returns true of the vector is zero length
                /*!
                    Dev Notes:
                    - Some consideration of exactness versus what epsilon to use is necessary
                    */
                template <typename T> 
                bool     is_zero_length (const vector3t<T>& v)
                { 
                    return fabs( length_squared(v) ) <= 10.0f * std::numeric_limits<float>::epsilon(); 
                }
        
                template <typename T> 
                bool     is_unit_length (const vector3t<T>& v)
                { 
                    return fabs( length_squared(v)  - 1.0f ) <= 10.0f * std::numeric_limits<float>::epsilon(); 
                }
        
                template <typename T> 
                bool     is_orthogonal (const vector3t<T>& u, const vector3t<T>& v) 
                { 
                    return fabs( dot(u, v) ) <= 10.0f * std::numeric_limits<float>::epsilon(); 
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

                //! Rotate a vector about an axis
                /*
                    Rotate the vector v around the axis by r radians
                 */
                template <typename T>
                vector3t<T>  
                rotate (const vector3t<T>& v, const vector3t<T>& axis, typename vector3t<T>::type r)
                {
                    const T sin_half_r ( sin(r / 2) );
                    glm::fquat q;
                    q.x = axis.x * sin_half_r;
                    q.y = axis.y * sin_half_r;
                    q.z = axis.z * sin_half_r;
                    q.w = cos(r / 2);

                    auto w = q * v.vec;
                    return vector3t<T>( w );
                }

                template <typename T>
                glgeom::radians
                angle_between (const vector3t<T>& a, const vector3t<T>& b)
                {
                    // a dot b = |a||b|cos(t)
                    return glgeom::radians( acosf( dot(a, b) / (length(a) * length(b)) ) );
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

            using detail::vector2t;
            using detail::vector3t;
            using detail::vector4t;

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
    }

    using namespace glgeom::core::vector;
}   

#endif
