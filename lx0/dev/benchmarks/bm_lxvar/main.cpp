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
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

// Standard headers
#include <vector>
#include <iostream>

// Lx0 headers
#include <lx0/core/lxvar/lxvar.hpp>
#include <lx0/util/misc/util.hpp>

using namespace lx0;

inline int generate(int i)
{
    return (2 * i - 1) % 117;
}

const int kOuter = 100;
const int kInner = 10000;

void
time_stdvector()
{
    for (int j = 0; j < kOuter; j++)
    {
        std::vector<int> v;
        for (int i = 0; i < kInner; i++)
        {
            v.push_back(generate(i));
        }
    }
}

void
time_lxvar()
{
    for (int j = 0; j < kOuter; j++)
    {
        lxvar v;
        for (int i = 0; i < kInner; i++)
        {
            v.push(generate(i));
        }
    }
}

int 
main (int argc, char** argv)
{
    {
        auto start = lx_milliseconds();
        time_stdvector();
        auto end = lx_milliseconds();
        std::cout << "std::vector = " << (end - start) << std::endl;
    }
    {
        auto start = lx_milliseconds();
        time_lxvar();
        auto end = lx_milliseconds();
        std::cout << "      lxvar = " << (end - start) << std::endl;
    }
    return 0;
}
