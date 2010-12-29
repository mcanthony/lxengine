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
#include <limits>
#include <algorithm>
#include <string>
#include <lx0/core/math/tuple3.hpp>
#include <lx0/core/math/point3.hpp>
#include <lx0/core/math/vector3.hpp>
#include <lx0/core/core.hpp>

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

    test.add_group("noise", [&test]() {

        double noiseMin = std::numeric_limits<double>::max();
        double noiseMax = std::numeric_limits<double>::min();
        double noiseAvg = 0.0;

        double avg = 0.0;
        srand(512);
        for (int i = 0; i < 1000 * 1000; ++i) 
        {
            point3 p;
            for (int j = 0; j < 3; ++j)
                p[j] = ((rand() % 20000) - 10000) / 1000.0f;
            float v = noise3d_perlin(p.x, p.y, p.z);
            
            double d = double(v);
            noiseMin = std::min(noiseMin, d);
            noiseMax = std::max(noiseMax, d);
            noiseAvg += d;
        }
        noiseAvg /= 1000 * 1000;

        test.check(noiseMin >= -1.0 && noiseMin <= -.6);
        test.check(noiseMax <= 1.0 && noiseMax >= .6);
        test.check( abs(noiseAvg) < 0.1 );
    });
    
    
	return 0;
}