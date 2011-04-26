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


#include <cstdio>
#include <iostream>
#include <sstream>
#include <functional>
#include <vector>
#include <map>
#include <string>

#include "glgeom/glgeom.hpp"
using namespace glgeom;

class Report
{
public:
    void benchmark  (std::string name) {}
    void value (std::string name, size_t value) {}
};

int 
main (int argc, char** argv)
{
    Report r;

    r.benchmark("Type sizes");
    r.value( "sizeof char",     sizeof(char) );
    r.value( "sizeof int",      sizeof(int) );
    r.value( "sizeof float",    sizeof(float) );
    r.value( "sizeof double",   sizeof(double) );
    r.value( "sizeof void*",    sizeof(void*) );
    r.value( "sizeof vector3f", sizeof(vector3f) );
    r.value( "sizeof point3f",  sizeof(point3f) );

    r.benchmark("Basic operations");

    return 0;
}
