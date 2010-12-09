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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

// Standard headers
#include <iostream>
#include <string>
#include <sstream>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

// Lx0 headers
#include <lx0/core.hpp>
#include <lx0/lxvar.hpp>
#include <lx0/util.hpp>

using namespace lx0::core;

class fixed8
{
public:
                fixed8() {}
    explicit    fixed8(int value);
    explicit    fixed8(float v);
    explicit    fixed8(double v);

    operator float () { return float(mValue) / kScale; }
    operator double() { return double(mValue) / kScale; }

    friend fixed8 operator+ (fixed8, fixed8);
    friend fixed8 operator- (fixed8, fixed8);

protected:
    enum { 
        kShift = 8,
        kScale = (1 << kShift),
    };

    static fixed8   _raw    (int value)    { fixed8 t; t.mValue = value; return t; }

    int mValue;
};



fixed8::fixed8 (int value)
    : mValue (value * kScale)
{
}

fixed8::fixed8 (float v)
    : mValue (int(v * kScale))
{
}

fixed8::fixed8 (double v)
    : mValue (int(v * kScale))
{
}


inline fixed8 operator+ (fixed8 a, fixed8 b) { return fixed8::_raw(a.mValue + b.mValue); }
inline fixed8 operator- (fixed8 a, fixed8 b) { return fixed8::_raw(a.mValue - b.mValue); }

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    fixed8 a (5);
    fixed8 b (4.3f);

    fixed8 c = a + b;
    fixed8 d = a + b;

    std::cout << "Value = " << float(c) << std::endl;

    return 0;
}
