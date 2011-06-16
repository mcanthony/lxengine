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

#include "unittest.hpp"


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

            std::cout << "= Set '" << mContext.pSet->mName << "'" << std::endl;

            auto& tests = mContext.pSet->mTests;
            for (size_t k = 0; k < tests.size(); ++k)
            {
                mContext.pTest = &tests[k];
                mContext.check = 0;
                mContext.checkPass = 0;
                mContext.checkFail = 0;
                mContext.checkLastFailed = false;
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
                        << " [" << mContext.check << " checks]"
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
    std::cout << "Total: " << mCheck.total << " checks (" << mCheck.fail << " fail / " << mCheck.pass << " pass) " << mCheck.percent() << "%" << std::endl
              << "       " << mTest.total << " tests (" << mTest.fail << " fail / " << mTest.pass << " pass) " << mTest.percent() << "%" << std::endl;
}

