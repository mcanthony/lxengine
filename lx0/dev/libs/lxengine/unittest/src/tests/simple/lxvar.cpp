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
        CHECK(r, v.asFloat() == 1.0f);
        
        q = v;

        CHECK(r, v.isSharedType() == false);
        CHECK(r, v.isShared() == false);
        CHECK(r, v.asFloat() == 1.0f);
        CHECK(r, q.isSharedType() == false);
        CHECK(r, q.isShared() == false);
        CHECK(r, q.asFloat() == 1.0f);

        v = 2.0f;
        CHECK(r, v.asFloat() == 2.0f);
        CHECK(r, q.asFloat() == 1.0f);

        v = "alpha";
        q = v;
        CHECK(r, v.isSharedType() == false);
        CHECK(r, v.isShared() == false);
        CHECK(r, v.asString() == "alpha");
        CHECK(r, q.isSharedType() == false);
        CHECK(r, q.isShared() == false);
        CHECK(r, q.asString() == "alpha");

        v = "beta";
        CHECK(r, v.isSharedType() == false);
        CHECK(r, v.isShared() == false);
        CHECK(r, v.asString() == "beta");
        CHECK(r, q.isSharedType() == false);
        CHECK(r, q.isShared() == false);
        CHECK(r, q.asString() == "alpha");
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



        struct lxpoint3f : public lxvalue_ref<point3f, lxpoint3f>
        {
            lxpoint3f (const Data* p) : Base(p) {}
            virtual lxvar at(int i)  
            {
                return (*mpData)[i];
            }
        };

        struct lxcamera 
            : public lx0::core::lxvar_ns::detail::lxvalue
            , public Camera
        {
            virtual lxvalue* clone() const { return nullptr; }

            virtual lxvar find(const char* _key) const
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
        CHECK_CMP(r, v.find("position").at(0).asFloat(), 5.0f, 1e-4f);
        CHECK(r, v.find("direction").size() == 3);
        CHECK(r, v("direction").size() == 3);
        CHECK(r, v("direction")[0].asFloat() < 0.0);

    });
}
