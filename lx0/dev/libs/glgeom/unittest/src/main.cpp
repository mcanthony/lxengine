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
    void run (TestModule& module);

    void check (bool b, int line, std::string expression) 
    {
        if (!b)
        {
            mContext.checkFail++;
            mFailures.push_back(std::make_pair(line, expression));
        }
        else
            mContext.checkPass++;
        mContext.check ++;
    }

    void check (bool b)
    {
        check(b, -1, "<unknown>");
    }

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

    std::vector<std::pair<int, std::string>> mFailures;
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
                auto& test = *mContext.pTest;

                test.mFunc(*this);

                if (mContext.checkFail == 0)
                {
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
                    std::cout << "FAIL      " 
                        << mContext.pModule->mName << "/"
                        << mContext.pGroup->mName << "/"
                        << mContext.pSet->mName << "/"
                        << mContext.pTest->mName
                        << std::endl;
                }
            }
        }
    }
    std::cout << std::endl;


    if (!mFailures.empty())
    {
        std::cout << "Failure Report:" << std::endl;
        for (auto it = mFailures.begin(); it != mFailures.end(); ++it)
        {
            std::cout 
                << "  " << it->first
                << ": " << it->second
                << std::endl;
        }
    }
}

#define CHECK(R,EXP) R.check(EXP, __LINE__, #EXP);

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
            set.mName = "Basics";
            
            set.push("Empty test", [] (TestRun& r) {
                /* Do nothing test */
            });

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


            //! \todo Does adding two vectors make sense?  Adding two tuples, yes...but vectors?
            set.push("tuple3f addition", [] (TestRun& r) {
                tuple3f v, u, w;

                v = tuple3f(0, 1, 2);
                u = tuple3f(3, 6, 9);
                w = v + u;
                
                CHECK(r, w.x == 3.0f);
                CHECK(r, w.y == 7.0f);
                CHECK(r, w.z == 11.0f);

                CHECK(r, v.x == 0.0f);
                CHECK(r, v.y == 1.0f);
                CHECK(r, v.z == 2.0f);

                CHECK(r, u.x == 3.0f);
                CHECK(r, u.y == 6.0f);
                CHECK(r, u.z == 9.0f);
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

            group.mSets.push_back(set);
        }
        module.mGroups.push_back(group);
    }

    TestRun run;
    run.run(module);

    return 0;
}
