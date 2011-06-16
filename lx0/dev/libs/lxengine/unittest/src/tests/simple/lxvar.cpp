#include"main.hpp"
#include <lx0/lxengine.hpp>

using lx0::lxvar;
using namespace glgeom;

void
testset_lxvar(TestSet& set)
{
    lx0::lx_init();

    set.push("ctor", [] (TestRun& r) {
        lxvar v;

        CHECK(r, v.isDefined() == false);
        CHECK(r, v.isUndefined() == true);
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

        struct lxcamera 
            : public lx0::core::lxvar_ns::detail::lxvalue
            , public Camera
        {
            virtual lxvalue* clone() const { return nullptr; }

            virtual lxvar find(const char* _key) const
            {
                std::string key(_key);
                if (key == "position")
                    return lxvar(5, 4, 3);
                else
                    _invalid();
            }

        };


        lxvar v(new lxcamera);
        v.find("position");
        CHECK_CMP(r, v.find("position").at(0).asFloat(), 5.0f, 1e-4f);

    });
}
