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

#include <iostream>
#include <functional>
#include <string>
#include <lx0/tuple3.hpp>
#include <lx0/point3.hpp>
#include <lx0/vector3.hpp>

class UnitTest
{
public:
    void add_group (std::string s, std::function<void()> f)
    {
        std::cout << "Running group '" << s << "'..." << std::endl;
        f();
        std::cout << "Done." << std::endl;
    }
    
    void check (bool b)
    {
        std::cout << "\t";
        if (b)
            std::cout << "PASS";
        else
            std::cout << "FAIL";
        std::cout << std::endl;
    }
};


using namespace lx0::core;


int 
main (int argc, char** argv)
{
    std::cout << "lx::vector sandbox" << std::endl;
    
    UnitTest test;
    
    test.add_group("type conversions", [test]() 
    {
        tuple3 a;
        point3 b;
        vector3 c;
        
        //
        // Implicit conversions are illegal
        //
        //a = b;
        //a = c;
        //b = a;
        //b = c;
        //c = a;
        //c = b;
        
        //
        // Explicit casts are legal
        //
        a = cast<tuple3&>(b);
        a = cast<tuple3&>(c);
        b = cast<point3&>(a);
        b = cast<point3&>(c);
        c = cast<vector3&>(a);
        c = cast<vector3&>(b);
    });
    
    test.add_group("constructors", [&test]() {
        tuple3 a;
        test.check(a.x == 0.0f);
        test.check(a.y == 0.0f);
        test.check(a.z == 0.0f);
        
        point3 b;
        test.check(b.x == 0.0f);
        test.check(b.y == 0.0f);
        test.check(b.z == 0.0f);
        
        vector3 c;
        test.check(c.x == 0.0f);
        test.check(c.y == 0.0f);
        test.check(c.z == 0.0f);


        test.check( is_orthogonal(vector3(1, 0, 0), vector3(0, 1, 0)) == true );
        test.check( is_orthogonal(vector3(0, 1, 0), vector3(0, 1, 0)) == false );
        test.check( is_orthogonal(vector3(0, 0, 1), vector3(0, 1, 0)) == true );
    });
    
    
	return 0;
}