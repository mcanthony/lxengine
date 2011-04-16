//===========================================================================//
/*
                                   LxEngine

    LICENSE

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

#pragma once

#include <cstddef>
#include <glm/glm.hpp>

namespace lx0 { namespace core {

    //=======================================================================//
    //!
    /*!
        Specialization of tuple4 intended for RGB colors.
     */
    class color3
    {
    public: 

        inline float&   operator[] (int i)          { return elem[i]; }
        inline float    operator[] (int i) const    { return elem[i]; }

        union 
        {
            struct
            {
                float r, g, b;
            };
            float elem[3];
            struct
            {
                glm::vec3 vec3;
            };
        };
    };


    //=======================================================================//
    //!
    /*!
        Specialization of tuple4 intended for RGBA colors.
     */
    class color4
    {
    public: 
        color4() {}
        color4(float r, float g, float b, float a) : vec4(r, g, b, a) {}

        inline float&   operator[] (int i)          { return elem[i]; }
        inline float    operator[] (int i) const    { return elem[i]; }

        union 
        {
            struct
            {
                float r, g, b, a;
            };
            float elem[4];
            struct
            {
                glm::vec4 vec4;
            };
        };
    };

}};