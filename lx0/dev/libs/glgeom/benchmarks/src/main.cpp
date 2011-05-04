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
    void value (std::string name, size_t value) { Result r = { name, value }; mResults.push_back(r); }
    void value (std::string name, double value) { Result r = { name, value }; mResults.push_back(r); }
    void report()
    {
        for (auto it = mResults.begin(); it != mResults.end(); ++it)
        {
            std::cout << it->name << ": " << it->value << std::endl;
        }
    }

    struct Result
    {
        std::string name;
        double      value;
    };
    std::vector<Result> mResults;
};

unsigned char random_set[] = 
{
    28, 250, 125, 212, 202, 155, 122, 27, 233, 176, 161, 69, 116, 124, 181, 129, 
    249, 0, 148, 147, 21, 139, 72, 195, 57, 243, 255, 253, 160, 103, 92, 214, 165,
    19, 174, 123, 169, 239, 68, 146, 31, 55, 83, 96, 40, 90, 52, 210, 61, 130, 67,
    56, 32, 84, 63, 119, 113, 204, 42, 118, 162, 30, 45, 12, 227, 222, 182, 186,
    106, 82, 238, 220, 170, 85, 185, 241, 251, 70, 200, 247, 177, 33, 163, 88,
    121, 196, 98, 213, 93, 230, 48, 97, 206, 143, 13, 24, 46, 7, 224, 235, 226,
    221, 79, 128, 60, 71, 54, 107, 194, 175, 2, 152, 105, 187, 131, 144, 11, 142,
    208, 109, 203, 198, 22, 49, 4, 10, 35, 53, 215, 248, 26, 62, 158, 217, 246,
    94, 242, 34, 101, 244, 50, 126, 38, 211, 134, 240, 80, 236, 136, 111, 225,
    234, 153, 89, 3, 36, 51, 39, 20, 232, 75, 254, 14, 166, 58, 228, 29, 64, 138,
    91, 199, 252, 87, 173, 150, 218, 15, 112, 59, 18, 151, 86, 8, 164, 192, 66,
    207, 16, 104, 102, 178, 114, 9, 179, 44, 1, 190, 189, 223, 157, 135, 127, 77,
    95, 216, 120, 37, 78, 205, 43, 180, 133, 154, 65, 17, 149, 141, 201, 47, 168,
    23, 167, 110, 219, 197, 132, 115, 108, 6, 156, 237, 76, 5, 81, 191, 183, 117,
    231, 73, 184, 25, 41, 172, 245, 188, 171, 137, 100, 140, 229, 99, 145, 193, 
    159, 209, 74
};

template <typename T>
class SphereRandomTest
{
public:
    static T run(int seed, int inc, T dist)
    {
        typedef T           type;

        auto f = [&]() -> unsigned char { auto r = random_set[seed%256]; seed += inc; return r; };

        vector3t<T> normal( glm::normalize(glm::detail::tvec3<T>(f() - 127, f() - 127, f() - 127)) );
        point3t<T>  expected( T(.5) * normal.vec  );
        sphere3t<T> sphere(point3t<T>(0,0,0), T(0.5));

        ray3t<T>    ray (expected + normal * T(dist), -normal);

        intersect3t<T> isect;
        const bool b = intersect(ray, sphere, isect);
        type       d = distance(expected, isect.position);
        return d;
    }
};

template <typename T>
class SphereEdgeTest
{
public:
    static bool run(double dist, double dev)
    {
        const T offset(1e3);
        const T base(0.25);
        sphere3t<T> sphere(point3t<T>(offset,0,0), T(0.5));
        ray3t<T>    ray (point3t<T>(offset + base, dist, 0), vector3t<T>(0, -1, 0));

        intersect3t<T> isect;
        bool hit_ok = intersect(ray, sphere, isect);
        if (!hit_ok)
            return false;
        
        bool pos_ok = isect.position.x > T(offset + base - dev) 
            && isect.position.x < T(offset + base + dev);
        if (!pos_ok)
            return false;

        return true;
    }

};

double sphere_edge_test (double dev)
{
    bool failure = false;
    double p;
    for (p = 0.0; p < 128.0; p += 0.01)
    {
        bool b = SphereEdgeTest<float>::run( pow(10.0, p), dev );
        if (!b)
        {
            failure = true;
            break;
        }
    }
    if (!failure)
        p = std::numeric_limits<double>::infinity();
    else
        p = pow(10.0, p);

    return p;
}

class Stat
{
public:
    Stat() 
        : mMin (std::numeric_limits<double>::max())
        , mMax (std::numeric_limits<double>::min())
        , mTotal (0.0)
        , mCount (0)
    {
    }

    void tally (double d)
    {
        mCount ++;
        mTotal += d;
        mMin = std::min(d, mMin);
        mMax = std::max(d, mMax);
    }

    double min()     { return mMin; }
    double max()     { return mMax; }
    double average() { return mTotal / double(mCount); }

protected:
    double  mMin;
    double  mMax;
    double  mTotal;
    int     mCount;
};

int 
main (int argc, char** argv)
{
    Report r;

    r.benchmark("Type sizes");
    r.value( "sizeof char",             sizeof(char) );
    r.value( "sizeof int",              sizeof(int) );
    r.value( "sizeof float",            sizeof(float) );
    r.value( "sizeof double",           sizeof(double) );
    r.value( "sizeof long double",      sizeof(long double) );
    r.value( "sizeof void*",            sizeof(void*) );
    r.value( "sizeof unsigned char*",   sizeof(unsigned char*) );
    r.value( "sizeof vector3f",         sizeof(vector3f) );
    r.value( "sizeof point3f",          sizeof(point3f) );

    r.benchmark("Basic operations");


    /*
        Test precision limits on a unit sphere located at the origin
        position limit
     */
    {
        for (int p = 0; p < 10; p ++)
        {
            const auto dist = pow(10.0f, p);

            Stat stat;
            for (int i = 0; i < 200; ++i)
            {
                const int step = (i % 2) ? 1 : 17;

                auto f = SphereRandomTest<float>::run(i, step, dist);
                auto d = SphereRandomTest<double>::run(i, step, dist);
                
                double dev = abs(d - double(f));
                stat.tally(dev);
            }

            std::stringstream ss;
            ss << "deviation at dist " << dist;
            ss << " (" << stat.max() << " max) ";
            r.value(ss.str(), stat.average());
        }
    }
    {
        for (int i = 1; i <= 12; i++)
        {
            double dev = pow(.1, i);
            std::stringstream ss;
            ss << "Sphere edge intersect failure " << dev << " (float)";
            r.value(ss.str(), sphere_edge_test(dev));
        }
    }

    r.report();

    char c;
    std::cin >> c;
    return 0;
}
