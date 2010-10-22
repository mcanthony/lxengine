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
std::vector<std::string> mFailedTests;

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
                    for (size_t i = 0; i < mFailLines.size(); ++i)
                    {
                        std::cout << "\t* failure at line " << mFailLines[i] << std::endl;
                        if (!mFailedTests[i].empty())
                            std::cout << "\t  " << mFailedTests[i] << std::endl;
                    }
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

void check(int line, bool b, std::string source)
{
    if (b)
        mChecksOkay++;
    else
    {
        mFailLines.push_back(line);
        mFailedTests.push_back(source);
    }
    mChecksTotal++;
}

void check_exception (int line, bool bShouldThrow, std::string source, std::function<void()> f)
{
    try 
    { 
        f(); 
        check(line, !bShouldThrow, source); 
    } 
    catch (...) 
    { 
        check(line, bShouldThrow, source); 
    }
}

#define CHECK(b) check(__LINE__, b, "")
#define CHECK_EXCEPTION(Code) check_exception(__LINE__, true, #Code, [&]() Code )

int 
main (int argc, char** argv)
{
    try
    {
        F.add("lxvar tests", []() {
            
            lxvar v;
            CHECK(v.isUndefined());
            CHECK(!v.isDefined());
            CHECK_EXCEPTION({ v.size(); });
            CHECK_EXCEPTION({ v.at(0); });

            v = 1;
            CHECK(v.isInt());
            CHECK_EXCEPTION({ v.size(); });

            v = 1.1f;
            CHECK(v.isFloat());
            CHECK_EXCEPTION({ v.size(); });

            v.undefine();
            CHECK(v.isUndefined());
            CHECK_EXCEPTION({ v.size(); });

            v = lxvar::map();
            CHECK(v.isDefined());
            CHECK(v.isMap());
            CHECK(v.size() == 0);
            CHECK(v.find("a").isUndefined());
            CHECK_EXCEPTION({ v.at(0); });

            v = lxvar::array();
            CHECK(v.isDefined());
            CHECK(v.isArray());
            CHECK(v.size() == 0);
            CHECK_EXCEPTION({ v.at(0); });

            v.push(1.1f);
            CHECK( v.size() == 1 );
            CHECK( v.at(0).isFloat() );
            CHECK_EXCEPTION({ v.at(1); });
        });

        F.add("parse simple", [] () {
            JsonParser parser;
            lxvar v;

            v = parser.parse("{}");
            CHECK(v.isMap());
            CHECK(v.size() == 0);

            v = parser.parse(" { } ");
            CHECK(v.isMap());
            CHECK(v.size() == 0);

            v = parser.parse("{ \"alpha\" : 1, \"beta\" : \"two\" }");
            CHECK(v.find("alpha").asInt() == 1);
            CHECK(v.find("beta").asString() == "two");
            CHECK(v.size() == 2);

            // Trailing comma should be ok.
            v = parser.parse("{ \"alpha\" : 1, \"beta\" : \"two\", }");
            CHECK(v.find("alpha").asInt() == 1);
            CHECK(v.find("beta").asString() == "two");
            CHECK(v.size() == 2);

            // Single quotes should be ok.
            v = parser.parse("{ 'alpha' : 1, 'beta' : 'two', }");
            CHECK(v.find("alpha").asInt() == 1);
            CHECK(v.find("beta").asString() == "two");
            CHECK(v.size() == 2);

            v = parser.parse("{ 'al pha' : 1, \"beta \" :  ' two', }");
            CHECK(v.find("al pha").asInt() == 1);
            CHECK(v.find("beta ").asString() == " two");
            CHECK(v.size() == 2);

            // Unquoted associative array keys should be okay
            v = parser.parse("{ alpha : 1, beta:' two', }");
            CHECK(v.find("alpha").asInt() == 1);
            CHECK(v.find("beta").asString() == " two");
            CHECK(v.size() == 2);
            CHECK_EXCEPTION({ parser.parse("{ al pha : 1, beta:' two', }"); });
            CHECK_EXCEPTION({ parser.parse("{ alpha : 1, beta:two, }"); });
            
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
            try { v.size();  CHECK(true);  } catch (...) { CHECK(false); }
        });

        F.run();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return 0;
}
