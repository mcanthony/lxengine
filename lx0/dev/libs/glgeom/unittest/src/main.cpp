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
#include "glgeom/prototype/camera.hpp"

using namespace glgeom;


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

class TestRun;

typedef std::function<void (TestRun&)> TestFunction;

struct Test
{
    std::string  mName;
    TestFunction mFunc;
};

struct TestSet
{
    void push (std::string name, TestFunction f)
    {
        Test t;
        t.mName = name;
        t.mFunc = f;
        mTests.push_back(t);
    }
    void push (TestFunction f)
    {
        std::stringstream ss;
        ss << mTests.size();
        
        push(ss.str(), f);
    };

    std::string       mName;
    std::vector<Test> mTests;
};

struct TestGroup
{
    std::string             mName;
    std::vector<TestSet>    mSets;
};

struct TestModule
{
    std::string             mName;
    std::vector<TestGroup>  mGroups;
};

struct Results
{
};

class TestRun
{
public:
    TestRun()
    {
    }

    void run (TestModule& module);

    void check (bool b, int line, std::string expression) 
    {
        if (!b)
        {
            mContext.checkFail++;
            Failure f;
            f.name = mContext.pTest->mName;
            f.line = line;
            f.expression = expression;
            f.detail = mDetail.str();
            mFailures.push_back(f);
        }
        else
            mContext.checkPass++;
        mContext.check ++;
    }

    void check_cmp (double a, double b, double t, int line, std::string expA, std::string expB) 
    {
        double d = abs(b - a);
        if (d > t)
        {
            std::stringstream ss;
            ss << d << " diff between '" << expA << "' and '" << expB << "' (" << t << " tol)";
            check(false, line, ss.str());
        }
        else
            check(true, line, "");
    }

    void check (bool b)
    {
        check(b, -1, "<unknown>");
    }

    bool    detailNeeded() const { return mContext.checkFail > 0; }
    std::stringstream& detail() { mDetail.str(""); return mDetail; }
    std::stringstream mDetail;

protected:
    struct
    {
        TestModule* pModule;
        TestGroup*  pGroup;
        TestSet*    pSet;
        Test*       pTest;
        int         check;
        int         checkPass;
        int         checkFail;
    } mContext;

    struct Count
    {
        Count() : total (0), pass(0), fail(0) {}
        int total;
        int pass;
        int fail;

        double percent () { return 100.0 * double(pass) / double(total); }
    };

    Count                                    mTest;
    Count                                    mCheck;

    struct Failure
    {
        std::string name;
        int line;
        std::string expression;
        std::string detail;
    };
    std::vector<Failure> mFailures;
};


void
TestRun::run (TestModule& module)
{
    mContext.pModule = &module;
    for (size_t i = 0; i < module.mGroups.size(); ++i)
    {
        mContext.pGroup = &module.mGroups[i];
        auto& sets = mContext.pGroup->mSets;
        for (size_t j = 0; j < sets.size(); ++j)
        {
            mContext.pSet = &sets[j];
            auto& tests = mContext.pSet->mTests;
            for (size_t k = 0; k < tests.size(); ++k)
            {
                mContext.pTest = &tests[k];
                mContext.check = 0;
                mContext.checkPass = 0;
                mContext.checkFail = 0;
                mDetail.clear();
                auto& test = *mContext.pTest;

                test.mFunc(*this);

                mTest.total ++;
                if (mContext.checkFail == 0)
                {
                    mTest.pass ++;
                    std::cout << "pass      " 
                        << mContext.pModule->mName << "/"
                        << mContext.pGroup->mName << "/"
                        << mContext.pSet->mName << "/"
                        << mContext.pTest->mName
                        << " (" << mContext.check << " checks)"
                        << std::endl;
                }
                else
                {
                    mTest.fail ++;

                    std::cout << "FAIL      " 
                        << mContext.pModule->mName << "/"
                        << mContext.pGroup->mName << "/"
                        << mContext.pSet->mName << "/"
                        << mContext.pTest->mName
                        << " (" << mContext.checkFail << "/" << mContext.check << " failed)"
                        << std::endl;
                }
                mCheck.total += mContext.check;
                mCheck.pass += mContext.checkPass;
                mCheck.fail += mContext.checkFail;
            }
        }
    }
    std::cout << std::endl;

    if (!mFailures.empty())
    {
        std::cout << "Failure Report:" << std::endl << std::endl;
        for (auto it = mFailures.begin(); it != mFailures.end(); ++it)
        {
            std::cout 
                << it->name
                << std::endl
                << "  " << it->line
                << ": " << it->expression
                << std::endl;
            if (!it->detail.empty())
                std::cout << std::endl << "Detail ---------\n" << it->detail << "----------------\n" << std::endl;
        }
    }
    std::cout << std::endl;
    std::cout << "Total: " << mTest.total << " tests (" << mTest.fail << " fail / " << mTest.pass << " pass) " << mTest.percent() << "%" << std::endl
              << "       " << mCheck.total << " checks (" << mCheck.fail << " fail / " << mCheck.pass << " pass) " << mCheck.percent() << "%" << std::endl;
}

#define CHECK(R,EXP) R.check(EXP, __LINE__, #EXP);

#define CHECK_CMP(R,A,B,T) R.check_cmp(A, B, T, __LINE__, #A, #B);

inline bool cmp(float a, float b, float d) { return fabs(b - a) <= d; }


template <typename T, int PREC>
class SphereRandomTest
{
public:
    static void run(TestRun& r)
    {
        typedef T           type;
        typedef point3t<T>  point3x;
        typedef vector3t<T> vector3x;

        int seed = 0;
        auto f = [&]() { return random_set[(seed++)%256]; };

        for (int i = 0; i < 256; i++)
        {
            point3x     expected(f(), f(), f());

            type        radius = f() / 32.0f + T(0.1);
            vector3x    normal(normalize(vector3x(f(), f(), f())));
            sphere3t<T> sphere(expected - radius * normal, radius);
                    
            vector3x    direction (normalize(vector3x(f(), f(), f())));
            if (dot(direction, normal) > 0.0f)
                direction = -direction;

            ray3t<T>    ray (expected - direction * (T(.1) + f()), direction);

            auto& detail = r.detail();

            detail 
                << "ray o:(" << ray.origin.x << ", " << ray.origin.y << ", " << ray.origin.z << ") d:(" << ray.direction.x << ", " << ray.direction.y << ", " << ray.direction.z << ")" << std::endl
                << "sphere c:(" << sphere.center.x << ", " << sphere.center.y << ", " << sphere.center.z << ") r:(" << sphere.radius << ")" << std::endl
                << "expected: (" << expected.x << ", " << expected.y << ", " << expected.z << ")" << std::endl;

            const auto tol = distance(ray.origin, sphere.center) / T(PREC);

            // Preliminary checks on the generated data
            CHECK(r, point_on_surface(expected, sphere, tol));
            CHECK(r, point_on_curve(expected, ray, tol));
            CHECK(r, distance(ray.origin, sphere.center) > sphere.radius);

            intersect3t<T> isect;
            const bool b = intersect(ray, sphere, isect);

            detail
                << "isect: (" << isect.position.x << ", " << isect.position.y << ", " << isect.position.z << ")" << std::endl
                << "distance: " << distance(ray.origin, sphere.center) << std::endl;

            CHECK(r, b == true);
            CHECK_CMP(r, distance(isect.position, sphere), T(0.0), tol);
            CHECK(r, point_on_curve(isect.position, ray, tol));
            CHECK_CMP(r, distance(expected, isect.position), T(0.0), tol);
            CHECK_CMP(r, degrees(angle_between(normal, isect.normal)).value, T(0.0), T(5.0));
        }
    }
};

void
testset_vector(TestSet& set)
{
    set.mName = "Basic";

    set.push("vector2f constructor", [] (TestRun& r) {
        vector2f v;
        CHECK(r, sizeof(v) == sizeof(float) * 2);
        CHECK(r, v.x == 0.0f);
        CHECK(r, v.y == 0.0f);
    });
    set.push("vector3f constructor", [] (TestRun& r) {
        vector3f v;
        CHECK(r, sizeof(v) == sizeof(float) * 3);
        CHECK(r, v.x == 0.0f);
        CHECK(r, v.y == 0.0f);
        CHECK(r, v.z == 0.0f);
    });
    set.push("vector4f constructor", [] (TestRun& r) {
        vector4f v;
        CHECK(r, sizeof(v) == sizeof(float) * 4);
        CHECK(r, v.x == 0.0f);
        CHECK(r, v.y == 0.0f);
        CHECK(r, v.z == 0.0f);
        CHECK(r, v.w == 0.0f);
    });

    set.push("vector2d constructor", [] (TestRun& r) {
        vector2d v;
        CHECK(r, sizeof(v) == sizeof(double) * 2);
        CHECK(r, v.x == 0.0);
        CHECK(r, v.y == 0.0);
    });
    set.push("vector3d constructor", [] (TestRun& r) {
        vector3d v;
        CHECK(r, sizeof(v) == sizeof(double) * 3);
        CHECK(r, v.x == 0.0);
        CHECK(r, v.y == 0.0);
        CHECK(r, v.z == 0.0);
    });
    set.push("vector4d constructor", [] (TestRun& r) {
        vector4d v;
        CHECK(r, sizeof(v) == sizeof(double) * 4);
        CHECK(r, v.x == 0.0);
        CHECK(r, v.y == 0.0);
        CHECK(r, v.z == 0.0);
        CHECK(r, v.w == 0.0);
    });

    set.push("vector3f dot product", [] (TestRun& r) {
        vector3f v (0, 1, 2);
        vector3f u (3, 6, 9);
                
        float w = dot(v, u);
        float h = dot(u, v);
                
        CHECK(r,  w == h );
        CHECK(r,  w == 24.0f);

        CHECK(r, v.x == 0.0f);
        CHECK(r, v.y == 1.0f);
        CHECK(r, v.z == 2.0f);

        CHECK(r, u.x == 3.0f);
        CHECK(r, u.y == 6.0f);
        CHECK(r, u.z == 9.0f);
    });
}

void
testset_camera(TestSet& set)
{
    set.push("orientation_from_to_up (5,5,5)", [] (TestRun& r) {

        glgeom::camera3f camera;
        camera.position = point3f(5, 5, 5);
        camera.orientation = orientation_from_to_up(camera.position, point3f(0, 0, 0), vector3f(0, 0, 1));
        auto u = (glm::vec3(0, 0, 0) - camera.position.vec) * camera.orientation;

        CHECK_CMP(r, u.x, 0.0f, 1e-5f);
        CHECK_CMP(r, u.y, 0.0f, 1e-5f);
        CHECK_CMP(r, u.z, -8.66025404f, 1e-3f);
    });
    
    set.push("orientation_from_to_up (axes)", [] (TestRun& r) {   
        for (int i = 0; i < 6; ++i)
        {
            glgeom::camera3f camera;
            camera.position = point3f(0, 0, 0);
            camera.position[i%3] = ((i/3 == 0) ? 1 : -1) * 5;
            
            camera.orientation = orientation_from_to_up(camera.position, point3f(0, 0, 0), vector3f(0, 0, 1));
            auto u = (glm::vec3(0, 0, 0) - camera.position.vec) * camera.orientation;

            CHECK_CMP(r, u.x, 0.0f, 1e-5f);
            CHECK_CMP(r, u.y, 0.0f, 1e-5f);
            CHECK_CMP(r, u.z, -5.0f, 1e-3f);
        }

    });
}

int 
main (int argc, char** argv)
{
    TestModule module;
    module.mName = "GlGeom";
    {
        TestGroup group;
        group.mName = "Simple";
        {
            TestSet set;
            set.mName = "Empty";
            
            set.push("Empty test", [] (TestRun& r) {
                /* Do nothing test */
            });

            group.mSets.push_back(set);
        }
        {
            TestSet set;
            testset_vector(set);
            group.mSets.push_back(set);
        }
        {
            TestSet set;
            set.mName = "Intersections";

            set.push("ray-plane intersection base planes", [] (TestRun& r) {
                
                // XY, XZ, YZ planes - should intersect
                {
                    plane3f plane ( point3f(0, 0, 0), vector3f(0, 0,  1) );
                    ray3f   ray   ( point3f(0, 0, 1), vector3f(0, 0, -1) );
                    intersect3f isect;
                    bool b = intersect(ray, plane, isect);
                    CHECK(r, b == true);
                    CHECK(r, cmp(isect.distance, 1.0f, 0.00001f));
                }
                {
                    plane3f plane ( point3f(0, 0, 0), vector3f(0, 1,  0) );
                    ray3f   ray   ( point3f(0, 1, 0), vector3f(0, -1, 0) );
                    intersect3f isect;
                    bool b = intersect(ray, plane, isect);
                    CHECK(r, b == true);
                    CHECK(r, cmp(isect.distance, 1.0f, 0.00001f));
                }
                {
                    plane3f plane ( point3f(0, 0, 0), vector3f(1, 0,  0) );
                    ray3f   ray   ( point3f(1, 0, 0), vector3f(-1, 0, 0) );
                    intersect3f isect;
                    bool b = intersect(ray, plane, isect);
                    CHECK(r, b == true);
                    CHECK(r, cmp(isect.distance, 1.0f, 0.00001f));
                }

                // XY, XZ, YZ planes - should NOT intersect
                {
                    plane3f plane ( point3f(0, 0, 0), vector3f(0, 0,  1) );
                    ray3f   ray   ( point3f(0, 0, 1), vector3f(0, 0,  1) );
                    intersect3f isect;
                    bool b = intersect(ray, plane, isect);
                    CHECK(r, b == false);
                }
                {
                    plane3f plane ( point3f(0, 0, 0), vector3f(0, 1, 0) );
                    ray3f   ray   ( point3f(0, 1, 0), vector3f(0, 1, 0) );
                    intersect3f isect;
                    bool b = intersect(ray, plane, isect);
                    CHECK(r, b == false);
                }
                {
                    plane3f plane ( point3f(0, 0, 0), vector3f(1, 0, 0) );
                    ray3f   ray   ( point3f(1, 0, 0), vector3f(1, 0, 0) );
                    intersect3f isect;
                    bool b = intersect(ray, plane, isect);
                    CHECK(r, b == false);
                }
            });
            set.push("ray-plane intersection xy circle at origin", [] (TestRun& r) {

                plane3f plane ( point3f(0, 0, 0), vector3f(0, 0,  1) );

                for (float h = 1.0f; h <= powf(2, 10); h *= 2.0f)
                {
                    for (radians a(0.0f); a < two_pi(); a += pi() / 12.0f)
                    {
                        ray3f ray;  
                        ray.origin.x = cos(a);
                        ray.origin.y = sin(a);
                        ray.origin.z = h;
                        ray.direction = glm::normalize(-ray.origin.vec);

                        intersect3f isect;
                        bool b = intersect(ray, plane, isect);
                        CHECK(r, b == true);
                        CHECK(r, cmp(isect.distance, glm::length(ray.origin.vec), 0.0001f));
                    }
                }
            });

            set.push("ray-plane intersection random points", [] (TestRun& r) {

                int seed = 0;
                auto f = [&]() { return random_set[(seed++)%256]; };

                for (int i = 0; i < 256; i++)
                {
                    point3f     expected(f(), f(), f());
                    plane3f     plane   (expected, normalize(vector3f(f(), f(), f())) );
                    vector3f    direction (normalize(vector3f(f(), f(), f())));
                    ray3f       ray (expected - direction * f(), direction);

                    intersect3f isect;
                    const bool b = intersect(ray, plane, isect);
                    CHECK(r, b == true);
                    CHECK_CMP(r, distance(expected, isect.position), 0.0f, 3e-5f);
                    CHECK_CMP(r, angle_between(plane.normal, isect.normal).value, 0.0f, 0.0f);
                }

                seed = 0;
                for (int i = 0; i < 256; i++)
                {
                    point3d     expected(f(), f(), f());
                    plane3d     plane   (expected, normalize(vector3d(f(), f(), f())) );
                    vector3d    direction (normalize(vector3d(f(), f(), f())));
                    ray3d       ray (expected - direction * f(), direction);

                    intersect3d isect;
                    const bool b = intersect(ray, plane, isect);
                    CHECK(r, b == true);
                    CHECK_CMP(r, distance(expected, isect.position), 0.0, 2e-13);
                    CHECK_CMP(r, angle_between(plane.normal, isect.normal).value, 0.0f, 0.0f);
                }
            });


            set.push("ray-sphere intersection basic", [](TestRun& r) {
                
                sphere3f sphere(point3f(0,0,0), 1.0f);

                intersect3f isect;
                bool b;
                
                b = intersect(ray3f(point3f(2, 0, 0), vector3f(-1, 0, 0)), sphere, isect);
                CHECK(r, b == true);
                CHECK_CMP(r, distance(point3f(1, 0, 0), isect.position), 0.0f, 0.0f);

                b = intersect(ray3f(point3f(-2, 0, 0), vector3f(1, 0, 0)), sphere, isect);
                CHECK(r, b == true);
                CHECK_CMP(r, distance(point3f(-1, 0, 0), isect.position), 0.0f, 0.0f);
            });


            set.push("ray-sphere intersection random points (float)",   SphereRandomTest<float, 1000>::run);
            set.push("ray-sphere intersection random points (double)",  SphereRandomTest<double, 10000000>::run);

            group.mSets.push_back(set);
        }
        {
            TestSet set;
            testset_camera(set);
            group.mSets.push_back(set);
        }
        module.mGroups.push_back(group);
    }

    TestRun run;
    run.run(module);

    return 0;
}
