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
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

#include <lx0/core.hpp>
#include <lx0/jsonio.hpp>

using namespace lx0::core;
using namespace lx0::serial;


int mChecksOkay;
int mChecksTotal;
std::vector<int> mFailLines;

class Framework
{
public:
    struct Test
    {
        std::string name;
        std::function<void ()> test;
    };

    void add(std::string name, std::function<void ()> test)
    {
        Test t = { name, test };
        mTests.push_back(t);
    }

    void run ()
    {
        for (auto it = mTests.begin(); it != mTests.end(); ++it)
        {
            mChecksOkay = 0;
            mChecksTotal = 0;
            mFailLines.clear();
        
            std::string nameCol = it->name;
            while (nameCol.size() < 40) nameCol += " ";

            try
            {
                it->test();
                bool bOk = (mChecksOkay == mChecksTotal);

                std::cout << (bOk ? "    " : "FAIL") << "  " << nameCol << " (" << mChecksOkay << "/" << mChecksTotal << ")" << std::endl;
                if (!mFailLines.empty())
                {
                    for (auto it = mFailLines.begin(); it != mFailLines.end(); ++it)
                        std::cout << "\t* failure at line " << *it << std::endl;
                }
            }
            catch (std::exception& e)
            {
                std::cout << "EXCP" << "  " << nameCol << " Unhandled exception!" << std::endl;
                std::cout << "\t" << e.what() << std::endl;
            }
        }
    }

    std::vector<Test> mTests;
};

Framework F;

void check(int line, bool b)
{
    if (b)
        mChecksOkay++;
    else
        mFailLines.push_back(line);
    mChecksTotal++;
}

#define CHECK(b) check(__LINE__, b)

int 
main (int argc, char** argv)
{
    try
    {

        F.add("parse simple", [] () {
            JsonParser parser;
            lxvar v;

            v = parser.parse("{}");
            CHECK(v.isMap());
            CHECK(v.count() == 0);

            v = parser.parse(" { } ");
            CHECK(v.isMap());
            CHECK(v.count() == 0);

            v = parser.parse("{ \"alpha\" : 1, \"beta\" : \"two\" }");
            CHECK(v.find("alpha").asInt() == 1);
            CHECK(v.find("beta").asString() == "two");
            CHECK(v.count() == 2);

            // Trailing comma should be ok.
            v = parser.parse("{ \"alpha\" : 1, \"beta\" : \"two\", }");
            CHECK(v.find("alpha").asInt() == 1);
            CHECK(v.find("beta").asString() == "two");
            CHECK(v.count() == 2);

            // Single quotes should be ok.
            v = parser.parse("{ 'alpha' : 1, 'beta' : 'two', }");
            CHECK(v.find("alpha").asInt() == 1);
            CHECK(v.find("beta").asString() == "two");
            CHECK(v.count() == 2);

            v = parser.parse("{ 'al pha' : 1, \"beta \" :  ' two', }");
            CHECK(v.find("al pha").asInt() == 1);
            CHECK(v.find("beta ").asString() == " two");
            CHECK(v.count() == 2);
            
        });
        F.add("parse non-standard", [] () {
            JsonParser parser;
            lxvar v;

            v = parser.parse("0");
            CHECK(v.isInt() && v.asInt() == 0);

            v = parser.parse("123");
            CHECK(v.isInt() && v.asInt() == 123);

            v = parser.parse("1.1");
            CHECK(v.isFloat() && v.asFloat() == 1.1f);

            v = parser.parse("\"This is a string.\"");
            CHECK(v.isString());
            CHECK(v.asString() == "This is a string.");
        });

        F.add("invalid ops", []() {
            JsonParser parser;
            lxvar v = parser.parse("{}");

            try { v.push(3); CHECK(false); } catch (...) { CHECK(true); }
            try { v.size(); CHECK(false);  } catch (...) { CHECK(true); }
        });

        F.run();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return 0;
}
