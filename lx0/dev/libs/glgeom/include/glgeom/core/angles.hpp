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

#ifndef GLGEOM_ANGLES_HPP
#define GLGEOM_ANGLES_HPP

#include <glm/glm.hpp>

namespace glgeom
{
    namespace core
    {
        namespace radians
        {
            class radians;
            class degrees;

            namespace detail
            {

            }

            //=======================================================================//
            //! Strongly typed radian class
            /*!
                The strongly typed radian class is intended to allow the compiler to 
                enforce correct usage of degrees or radians depending on the context. 
                */
            class radians
            {
            public:
                            radians (void)    : value (0.0f) {}
                            radians (const radians& r) : value (r.value) {}
                explicit    radians (const degrees& d);
                explicit    radians (float r) : value (r) {}


                radians     operator*   (float s) const { return radians(value * s); }
                radians     operator/   (float s) const { return radians(value / s); }

                void        operator+=  (radians r)         { value += r.value; }
                void        operator-=  (radians r)         { value -= r.value; }

                bool        operator!=  (radians r) const   { return value != r.value; }
                bool        operator==  (radians r) const   { return value == r.value; }
                bool        operator<   (radians r) const   { return value < r.value; }
                bool        operator>   (radians r) const   { return value > r.value; }

                float value;
            };

            //=======================================================================//
            //! Strongly typed radian class
            /*!
                The strongly typed radian class is intended to allow the compiler to 
                enforce correct usage of degrees or radians depending on the context. 
                */
            class degrees
            {
            public:
                            degrees (void)    : value (0.0f) {}
                            degrees (const degrees& t) : value (t.value) {}
                explicit degrees (const radians& r);
                explicit degrees (float d) : value (d) {}


                degrees     operator*   (float s) const { return degrees(value * s); }
                degrees     operator/   (float s) const { return degrees(value / s); }

                void        operator+=  (degrees d)     { value += d.value; }

                float value;
            };

            inline radians     pi       (void)      { return radians(3.1415926535897932384626433832795f); }
            inline radians     two_pi   (void)      { return radians(6.283185307179586476925286766559f); }
            inline radians     half_pi  (void)      { return radians(1.5707963267948966192313216916398f); }

            using ::cos;
            using ::sin;
            inline float        cos     (radians r) { return cosf(r.value); }
            inline float        sin     (radians r) { return sinf(r.value); }

            //---------------------------------------------------------------------------//
                
            inline radians::radians (const degrees& d)
                : value ( glm::radians(d.value) )
            {
            }

            //---------------------------------------------------------------------------//
                
            inline degrees::degrees (const radians& r)
                : value ( glm::degrees(r.value) )
            {
            }
            
        }
    }

    using namespace glgeom::core::radians;
}


#endif

