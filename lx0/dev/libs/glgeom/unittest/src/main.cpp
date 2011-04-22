#include <cstdio>
#include <iostream>
#include <functional>
#include <vector>
#include <map>

typedef std::function<void (void)> TestFunction;

struct Test
{
    std::string  mName;
    TestFunction mFunc;
};

struct TestSet
{
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

struct TestRun
{
    void run (TestModule& module);
};


void
TestRun::run (TestModule& module)
{
    for (auto it = module.mGroups.begin(); it != module.mGroups.end(); ++it)
    {
        for (auto jt = it->mSets.begin(); jt != it->mSets.end(); ++jt)
        {
            for (auto kt = jt->mTests.begin(); kt != jt->mTests.end(); ++kt)
            {
                kt->mFunc();
            }
        }
    }
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
            set.mName = "Basics";
            {
                Test test;
                test.mName = "0";
                test.mFunc = [] () {
                    printf("Hello World!\n");
                };
                set.mTests.push_back(test);
            }
            group.mSets.push_back(set);
        }
        module.mGroups.push_back(group);
    }

    TestRun run;
    run.run(module);

    return 0;
}
