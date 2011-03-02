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

#include <glm/glm.hpp>

namespace lx0 { namespace core { namespace math { namespace radians {

    //=======================================================================//
    //!
    /*!
        The strongly typed radian class is intended to allow the compiler to enforce 
        correct usage of degrees or radians depending on the context.
     */
    class radians
    {
    public:
                 radians (void)    : value (0.0f) {}
        explicit radians (float r) : value (r) {}

        float value;
    };

    inline radians     pi      (void)      { return radians(3.1415926535897932384626433832795f); }
    inline radians     twoPi   (void)      { return radians(6.283185307179586476925286766559f); }
    inline radians     halfPi  (void)      { return radians(1.5707963267948966192313216916398f); }

    inline float       degrees (radians r) { return glm::degrees(r.value); }



}}}}

namespace lx0 {
    using namespace core::math::radians;
};

