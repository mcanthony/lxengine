//===========================================================================//
/*
                                   GLGeom

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

#ifndef UNITTEST_HPP
#define UNITTEST_HPP

#include <cstdio>
#include <iostream>
#include <sstream>
#include <functional>
#include <vector>
#include <map>
#include <string>
#include <complex>

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
        mContext.checkLastFailed = !b;

        if (!b)
        {
            mContext.checkFail++;
            Failure f;
            f.name = mContext.pTest->mName;
            f.line = line;
            f.expression = expression;
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

    bool    failed() const { return mContext.checkLastFailed; }
    void    set_detail (std::stringstream& ss) { ss << std::endl; mFailures.back().detail = ss.str(); }

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
        bool        checkLastFailed;
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


#define ADD_TESTSET(GROUP, NAME) \
    { \
        TestSet set; \
        set.mName = #NAME; \
        void testset_ ## NAME (TestSet&); \
        testset_ ##  NAME (set); \
        GROUP.mSets.push_back(set); \
    }

#define CHECK(R,EXP) R.check(EXP, __LINE__, #EXP);
#define CHECK_CMP(R,A,B,T) R.check_cmp(A, B, T, __LINE__, #A, #B);

inline bool cmp(float a, float b, float d) { return fabs(b - a) <= d; }


#endif
