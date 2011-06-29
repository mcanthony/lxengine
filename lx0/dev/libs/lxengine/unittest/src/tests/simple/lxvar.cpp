#include"main.hpp"
#include <lx0/lxengine.hpp>


using lx0::lxvar;
using namespace glgeom;

using lx0::core::lxvar_ns::detail::lxvalue;
using lx0::core::lxvar_ns::detail::lxvalue_iterator;

//===========================================================================//

template <typename T, typename D> 
struct lxvalue_ref : public lx0::core::lxvar_ns::detail::lxvalue
{
    typedef lxvalue_ref<T,D>    Base;
    typedef T                   Data;

    lxvalue_ref(const T* pData) : mpData(pData) {}
    virtual lxvalue* clone() const { return new D(mpData); }
    const T* mpData;
};

lxvar lxvar_from (const vector3f& v)
{
    return lxvar(v.x, v.y, v.z);
}

void
testset_lxvar(TestSet& set)
{
    lx0::lx_init();

    set.push("example usage", [] (TestRun& r) {
        lxvar v;
        //v = true;
        v = 1;
        CHECK(r, v.as<int>() == 1);

        try
        {
            v.as<std::string>();
            CHECK(r, false);
        }
        catch (std::exception&)
        {
            CHECK(r, true);
        }
        
        v = 1.0f;
        //v = 1.0;
        v = "one";
    });

    set.push("ctor", [] (TestRun& r) {
        lxvar v;

        CHECK(r, v.is_defined() == false);
        CHECK(r, v.is_undefined() == true);
    });

    set.push("basics", [] (TestRun& r) {
    
        lxvar v(1.0f);
        lxvar q;

        CHECK(r, v.isSharedType() == false);
        CHECK(r, v.isShared() == false);
        CHECK(r, v.as<float>() == 1.0f);
        
        q = v;

        CHECK(r, v.isSharedType() == false);
        CHECK(r, v.isShared() == false);
        CHECK(r, v.as<float>() == 1.0f);
        CHECK(r, q.isSharedType() == false);
        CHECK(r, q.isShared() == false);
        CHECK(r, q.as<float>() == 1.0f);

        v = 2.0f;
        CHECK(r, v.as<float>() == 2.0f);
        CHECK(r, q.as<float>() == 1.0f);

        v = "alpha";
        q = v;
        CHECK(r, v.isSharedType() == false);
        CHECK(r, v.isShared() == false);
        CHECK(r, v.as<std::string>() == "alpha");
        CHECK(r, q.isSharedType() == false);
        CHECK(r, q.isShared() == false);
        CHECK(r, q.as<std::string>() == "alpha");

        v = "beta";
        CHECK(r, v.isSharedType() == false);
        CHECK(r, v.isShared() == false);
        CHECK(r, v.as<std::string>() == "beta");
        CHECK(r, q.isSharedType() == false);
        CHECK(r, q.isShared() == false);
        CHECK(r, q.as<std::string>() == "alpha");
    });

    set.push("ref", [] (TestRun& r) {
        lxvar v;
        v["size"]["a"] = 7;

        lxvar u = v["red"];
        u = 6;

        CHECK(r, v["size"]["a"] == 7);
        CHECK(r, v["red"] == lxvar::undefined());

        lxvar info = lxvar::ordered_map();
        info["sizes"] = lxvar::ordered_map();
        info["sizes"]["char"] =            (int)sizeof(char);
        info["sizes"]["short"] =           (int)sizeof(short);
        info["sizes"]["int"] =             (int)sizeof(int);
        info["sizes"]["long"] =            (int)sizeof(long);
        info["sizes"]["float"] =           (int)sizeof(long);
        info["sizes"]["double"] =          (int)sizeof(long);
        info["sizes"]["pointer"] =         (int)sizeof(void*);

        auto it = info["sizes"].begin();
        CHECK(r, it.key() == "char");
        CHECK(r, *it == (int)sizeof(char));
        ++it;
        CHECK(r, it.key() == "short");
        CHECK(r, *it == (int)sizeof(short));

        {
            lxvar a = 9;
            lxvar b = a;
            a = 7;

            CHECK(r, a.as<int>() == 7 );
            CHECK(r, b.as<int>() == 9 );

            a = lxvar::parse("[ 0 ] ");
            b = a;
            b.at(0, 1);
            CHECK(r, a.at(0).as<int>() == 1);
            CHECK(r, b.at(0).as<int>() == 1);
        }

    });

    auto test_maps = [] (TestRun& r, lxvar& v) {
        
        CHECK(r, v.isSharedType() == true);

        v.insert("test", 7);
        CHECK(r, v.find("test").as<int>() == 7);
        CHECK(r, v.size() == 1);

        v.insert("test2", 8);
        CHECK(r, v.size() == 2);
        CHECK(r, v.find("test").as<int>() == 7);
        CHECK(r, v.find("test2").as<int>() == 8);

        v.insert("test", 9);
        CHECK(r, v.size() == 2);
        CHECK(r, v.find("test").as<int>() == 9);
        CHECK(r, v.find("test2").as<int>() == 8);

        v.insert("test3", "alpha");
        CHECK(r, v.size() == 3);
        CHECK(r, v.find("test").as<int>() == 9);
        CHECK(r, v.find("test2").as<int>() == 8);
        CHECK(r, v.find("test3").as<std::string>() == "alpha");

        v.insert("test4", lxvar::undefined());
        CHECK(r, v.size() == 4);
        CHECK(r, v.has("test4") == true);
        CHECK(r, v.find("test4") == lxvar::undefined());
    };

    set.push("map", [&test_maps] (TestRun& r) {
        test_maps(r, lxvar::map());
    });

    set.push("ordered_map", [&test_maps] (TestRun& r) {
        test_maps(r, lxvar::ordered_map());

        lxvar m = lxvar::ordered_map();
        m.insert("b", "one");
        m.insert("a", "two");
        m.insert("0", "three");

        auto it = m.begin();
        CHECK(r, (*it).as<std::string>() == "one");
        ++it;
        CHECK(r, (*it).as<std::string>() == "two");
        ++it;
        CHECK(r, (*it).as<std::string>() == "three");
        ++it;
    });

    set.push("decorated map", [&test_maps] (TestRun& r) {
        
        {
            lxvar v = lxvar::decorated_map();
            test_maps(r, v);
        }
        {
            lxvar v = lxvar::decorated_map();

            v.add("percent", 0, [] (lxvar v, lxvar& out) -> bool {
                if (v.is_int())
                {
                    auto i = v.as<int>();
                    if (i < 0)
                        out = 0;
                    else if (i > 100)
                        out = 100;
                    else
                        out = v;
                    return true;
                }
                else
                    return false;
            });

            v.insert("percent", 42);
            CHECK(r, v["percent"] == 42);

            v.insert("percent", 199);
            CHECK(r, v["percent"] == 100);

            v.insert("percent", "test");
            CHECK(r, v["percent"] == 100);

            v.add("percent", 0, [] (lxvar v, lxvar& out) -> bool {
                if (v.is_int())
                {
                    auto i = v.as<int>();
                    if (i >= 0 && i <= 100)
                    {
                        out = v;
                        return true;
                    }
                }
                return false;
            });


            v.insert("percent", 42);
            CHECK(r, v["percent"] == 42);

            v.insert("percent", 199);
            CHECK(r, v["percent"] == 42);

            v.insert("percent", "test");
            CHECK(r, v["percent"] == 42);
        }
    });

    set.push("parse simple", [] (TestRun& r) {
        lxvar v;

        v = lxvar::parse("0.8");
        CHECK(r, v.is_float());

        v = lxvar::parse(" 0.8");
        CHECK(r, v.is_float());

        v = lxvar::parse("{}");
        CHECK(r, v.is_map());
        CHECK(r, v.size() == 0);

        v = lxvar::parse(" { } ");
        CHECK(r, v.is_map());
        CHECK(r, v.size() == 0);

        v = lxvar::parse("{ \"alpha\" : 1, \"beta\" : \"two\" }");
        CHECK(r, v.find("alpha").as<int>() == 1);
        CHECK(r, v.find("beta").as<std::string>() == "two");
        CHECK(r, v.size() == 2);

        // Trailing comma should be ok.
        v = lxvar::parse("{ \"alpha\" : 1, \"beta\" : \"two\", }");
        CHECK(r, v.find("alpha").as<int>() == 1);
        CHECK(r, v.find("beta").as<std::string>() == "two");
        CHECK(r, v.size() == 2);

        // Single quotes should be ok.
        v = lxvar::parse("{ 'alpha' : 1, 'beta' : 'two', }");
        CHECK(r, v.find("alpha").as<int>() == 1);
        CHECK(r, v.find("beta").as<std::string>() == "two");
        CHECK(r, v.size() == 2);

        v = lxvar::parse("{ 'al pha' : 1, \"beta \" :  ' two', }");
        CHECK(r, v.find("al pha").as<int>() == 1);
        CHECK(r, v.find("beta ").as<std::string>() == " two");
        CHECK(r, v.size() == 2);

        // Unquoted associative array keys should be okay
        v = lxvar::parse("{ alpha : 1, beta:' two', }");
        CHECK(r, v.find("alpha").as<int>() == 1);
        CHECK(r, v.find("beta").as<std::string>() == " two");
        CHECK(r, v.size() == 2);
        //CHECK_EXCEPTION({ lxvar::parse("{ al pha : 1, beta:' two', }"); });
        //CHECK_EXCEPTION({ lxvar::parse("{ alpha : 1, beta:two, }"); });
    });

    set.push("parse non-standard", [] (TestRun& r) {
        lxvar v;

        v = lxvar::parse("0");
        CHECK(r, v.is_int() && v.as<int>() == 0);

        v = lxvar::parse("123");
        CHECK(r, v.is_int() && v.as<int>() == 123);

        v = lxvar::parse("1.1");
        CHECK(r, v.is_float() && v.as<float>() == 1.1f);

        v = lxvar::parse("\"This is a string.\"");
        CHECK(r, v.is_string());
        CHECK(r, v.as<std::string>() == "This is a string.");
    });

    set.push("invalid ops", [](TestRun& r) {
        lxvar v = lxvar::parse("{}");

        try { v.push(3); CHECK(r, false); } catch (...) { CHECK(r, true); }
        try { v.size();  CHECK(r, true);  } catch (...) { CHECK(r, false); }
    });

    set.push("iterators", [](TestRun& r) {
        lxvar v = lxvar::parse("[ 0, 1, 2, 3, 4, 5 ]");

        try
        {
            int i = 0;
            auto et = v.end();
            for (auto it = v.begin(); it != v.end(); ++it)
            {
                bool b = ((*it).as<int>() == i);
                CHECK(r, b);
                i++;
            }
            CHECK(r, i == v.size()); 
            CHECK(r, true);
        } 
        catch (...)
        {
            CHECK(r, false);
        }


        v = lxvar::parse("{ a:0, b:1, c:2, d:3, e:4, f:5 }");

        try
        {
            int i = 0;
            auto et = v.end();
            for (auto it = v.begin(); it != v.end(); ++it)
            {
                bool b = ((*it).as<int>() == i);
                CHECK(r, b);
                i++;
            }
            CHECK(r, i == v.size()); 
            CHECK(r, true);
        } 
        catch (...)
        {
            CHECK(r, false);
        }

    });

}
