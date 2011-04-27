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
#include <lx0/core/math/matrix4.hpp>
#include <lx0/core/core.hpp>

class UnitTest
{
public:
    UnitTest()
        : mFailed (0)
        , mTotal (0)
    {
    }

    int mFailed;
    int mTotal;

    void add_group (std::string s, std::function<void()> f)
    {
        std::cout << "Running group '" << s << "'..." << std::endl;
        f();
        std::cout << "Done." << std::endl;
    }
    

    void check (bool b, std::function<void()> fail)
    {
        mTotal++;

        std::cout << "\t";
        if (b)
        {
            std::cout << "PASS";
        }
        else
        {
            mFailed++;
            std::cout << "FAIL ";
            fail();
        }
        std::cout << std::endl;
    }

    void check (bool b)
    {
        check(b, [](){});
    }

    void compare(float value, float expected)
    {
        float d = abs(value - expected);
        check(d < 0.00001f, [&]() { std::cout << value << " != " << expected; });
    }
};


using namespace lx0::core;


int 
main (int argc, char** argv)
{
    std::cout << "lx::vector sandbox" << std::endl;
    
    UnitTest test;
    
    /*test.add_group("type conversions", [test]() 
    {
        tuple3 a;
        point3 b;
        glgeom::vector3f c;
        
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
        c = cast<glgeom::vector3f&>(a);
        c = cast<glgeom::vector3f&>(b);
    });
    
    test.add_group("constructors", [&test]() {

        {
            tuple2 t;
            test.compare(t.x, 0.0f);
            test.compare(t.y, 0.0f);
        }
        {
            tuple3 t;
            test.compare(t.x, 0.0f);
            test.compare(t.y, 0.0f);
            test.compare(t.z, 0.0f);
        }
        {
            tuple4 t;
            test.compare(t.x, 0.0f);
            test.compare(t.y, 0.0f);
            test.compare(t.z, 0.0f);
            test.compare(t.w, 0.0f);
        }

        tuple3 a;
        test.check(a.x == 0.0f);
        test.check(a.y == 0.0f);
        test.check(a.z == 0.0f);
        
        point3 b;
        test.check(b.x == 0.0f);
        test.check(b.y == 0.0f);
        test.check(b.z == 0.0f);
        
        glgeom::vector3f c;
        test.check(c.x == 0.0f);
        test.check(c.y == 0.0f);
        test.check(c.z == 0.0f);

        {
            point3 c(1, 2, 3);
            test.compare(c.x, 1.0f);
            test.compare(c.y, 2.0f);
            test.compare(c.z, 3.0f);
        }

        {
            vector3 c(1, 2, 3);
            test.compare(c.x, 1.0f);
            test.compare(c.y, 2.0f);
            test.compare(c.z, 3.0f);
        }

        test.check( is_orthogonal(vector3(1, 0, 0), vector3(0, 1, 0)) == true );
        test.check( is_orthogonal(vector3(0, 1, 0), vector3(0, 1, 0)) == false );
        test.check( is_orthogonal(vector3(0, 0, 1), vector3(0, 1, 0)) == true );
    });
    */
    test.add_group("radians", [&test]() {
        lx0::radians r;

        test.compare(lx0::degrees(r), 0.0f);
        test.compare(lx0::degrees(lx0::halfPi()), 90.0f);
        test.compare(lx0::degrees(lx0::pi()), 180.0f);
        test.compare(lx0::degrees(lx0::twoPi()), 360.0f);
    });

    test.add_group("matrices", [&test]() {

        matrix4 m;
        set_identity(m);

        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                test.check(m(r,c) == ((r == c) ? 1.0f : 0.0f));

        // matrix4 is set up for row-vector multiplications: therefore,
        // translation should end up in row 4.
        set_translation(m, 2.0f, 4.0f, 6.0f);
        test.check(m(0,0) == 1.0f);
        test.check(m(1,0) == 0.0f);
        test.check(m(3,0) == 2.0f);
        test.check(m(3,1) == 4.0f);
        test.check(m(3,2) == 6.0f, [&]() { std::cout << m(3,2); });
        test.compare(m(3,3), 1.0f);

    });

    test.add_group("noise", [&test]() {

        double noiseMin = std::numeric_limits<double>::max();
        double noiseMax = std::numeric_limits<double>::min();
        double noiseAvg = 0.0;

        double avg = 0.0;
        srand(512);
        for (int i = 0; i < 1000 * 1000; ++i) 
        {
            glgeom::point3f p;
            for (int j = 0; j < 3; ++j)
                p[j] = ((rand() % 20000) - 10000) / 1000.0f;
            float v = noise3d_perlin(p.x, p.y, p.z);
            
            double d = double(v);
            noiseMin = std::min(noiseMin, d);
            noiseMax = std::max(noiseMax, d);
            noiseAvg += d;
        }
        noiseAvg /= 1000 * 1000;

        test.check(noiseMin >= 0.0 && noiseMin <= .2, [&]() { std::cout << "noiseMin = " << noiseMin; } );
        test.check(noiseMax <= 1.0 && noiseMax >= .8, [&]() { std::cout << "noiseMax = " << noiseMax; } );
        test.check( abs(noiseAvg - .5) < 0.05, [&]() { std::cout << "noiseAvg = " << noiseAvg; } );
    });

    test.add_group("bugcheck", [&test]() {

        // BUG: The anonymous union in vector3 calls *all* constructors defined in the union.
        // Placement of the GLM vec3 in the union after the x,y,z floats in the union caused
        // the default GLM vec3 constructor to zero out the fields after x, y, and z had been
        // set.  This is a complicated way of checking that the ctor initialization is correct.
        //
        {
            float x = 8.321321f;
            float y = -7.532123f;
            float z = -6.432111f;

            const int cellX = (int)floor(x);
            const int cellY = (int)floor(y);
            const int cellZ = (int)floor(z);

            glgeom::vector3f uvw = glgeom::vector3f(x, y, z);
            uvw.x = x - cellX;
            uvw.y -= cellY;
            uvw.z -= cellZ;

            test.compare(uvw.x, .321321f);
            test.compare(uvw.y, 1.0f - .532123f);
            test.compare(uvw.z, 1.0f - .432111f);
        }
    });
    
    std::cout << std::endl;
    std::cout << "=========================================================================" << std::endl;
    std::cout << std::endl;

    if (test.mFailed > 0)
    {
        std::cout << "FAILURE!" << std::endl;
        std::cout << "Failed " << test.mFailed << " of " << test.mTotal << " tests." << std::endl;
    }
    else
        std::cout << "SUCCESS" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

	return 0;
}