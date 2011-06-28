#include"main.hpp"
#include <lx0/lxengine.hpp>


using lx0::lxvar;
using namespace glgeom;


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

        CHECK(r, v.isDefined() == false);
        CHECK(r, v.isUndefined() == true);
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

    set.push("ref", [&test_maps] (TestRun& r) {
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

    });

    set.push("custom object", [] (TestRun& r) {
        
        struct Camera
        {
            Camera()
                : position (5, 4, 3)
                , target   (0, 1, 2)
            {
            }
            point3f position;
            point3f target;
        };



        /*struct lxpoint3f : public lxvalue_ref<point3f, lxpoint3f>
        {
            lxpoint3f (const Data* p) : Base(p) {}
            virtual lxvar* at(int i)  
            {
                return &(*mpData)[i];
            }
        };*/

        /*struct lxcamera 
            : public lx0::core::lxvar_ns::detail::lxvalue
            , public Camera
        {
            virtual lxvalue* clone() const { return nullptr; }

            virtual lxvar* find(const char* _key) const
            {
                std::string key(_key);
                     if (key == "position")     return new lxpoint3f(&position);
                else if (key == "target")       return new lxpoint3f(&target);
                else if (key == "direction")    return lxvar_from(normalize(target - position));
                else                            return _invalid(), lxvar::undefined();
            }

        };


        lxvar v(new lxcamera);
        v.find("position");
        CHECK_CMP(r, v.find("position").at(0).as<float>(), 5.0f, 1e-4f);
        CHECK(r, v.find("direction").size() == 3);
        CHECK(r, v["direction"].size() == 3);
        CHECK(r, v["direction"][0].as<float>() < 0.0);*/

    });
}
