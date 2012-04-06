#include"main.hpp"
#include <lx0/lxengine.hpp>

using namespace lx0;

static 
void element_flags (TestRun& r)
{
    struct NoUpdateComp : public Element::Component
    {
        virtual lx0::uint32 flags               (void) const { return eSkipUpdate; }
    };

    struct UpdateComp : public Element::Component
    {
        virtual lx0::uint32 flags               (void) const { return eCallUpdate; }
        virtual void onUpdate (ElementPtr spElem)
        {
        }
    };

    EnginePtr spEngine = Engine::acquire();
    {
        auto spDoc = spEngine->createDocument();
        auto spElem = spDoc->createElement("Test");
        spDoc->root()->append(spElem);

        CHECK(r, spElem->flagNeedsUpdate() == false);
        spDoc->update();
        CHECK(r, spElem->flagNeedsUpdate() == false);
        spDoc->update();
        CHECK(r, spElem->flagNeedsUpdate() == false);

        spElem->attachComponent(new NoUpdateComp);
        CHECK(r, spElem->flagNeedsUpdate() == false);
        spDoc->update();
        CHECK(r, spElem->flagNeedsUpdate() == false);
        spDoc->update();
        CHECK(r, spElem->flagNeedsUpdate() == false);

        spElem->attachComponent(new UpdateComp);
        CHECK(r, spElem->flagNeedsUpdate() == true);
        spDoc->update();
        CHECK(r, spElem->flagNeedsUpdate() == true);
        spDoc->update();
        CHECK(r, spElem->flagNeedsUpdate() == true);

    }
    spEngine->shutdown();
}

void
testset_engine(TestSet& set)
{
    set.push("acquire", [] (TestRun& r) {
        EnginePtr spEngine = Engine::acquire();
        CHECK(r, spEngine.get() != nullptr);
        CHECK(r, spEngine.get() == Engine::acquire().get());

        spEngine->shutdown();
        CHECK(r, spEngine.get() != nullptr);

        spEngine.reset();
        CHECK(r, spEngine.get() == nullptr);
    });

    set.push("version check", [] (TestRun& r) {
        EnginePtr spEngine = Engine::acquire();

        CHECK(r, spEngine->versionMajor() == LXENGINE_VERSION_MAJOR);
        CHECK(r, spEngine->versionMinor() == LXENGINE_VERSION_MINOR);
        CHECK(r, spEngine->versionRevision() == LXENGINE_VERSION_REVISION);

        spEngine->shutdown();
    });

    set.push("Document", [] (TestRun& r) {
        EnginePtr spEngine = Engine::acquire();
        {
            auto spDoc1 = spEngine->createDocument();
            auto spDoc2 = spEngine->createDocument();
            CHECK(r, spEngine->documents().size() == 2);
            CHECK(r, spEngine->objectCount("Document").current() == 2);
            CHECK(r, spDoc1.use_count() == 2);
            CHECK(r, spDoc2.use_count() == 2);

            spEngine->closeDocument(spDoc2);
            CHECK(r, spEngine->documents().size() == 1);
            CHECK(r, spEngine->objectCount("Document").current() == 1);
            CHECK(r, spDoc2.get() == nullptr);

            // Orphan a document so the Engine has the only reference...
            CHECK(r, spEngine->documents().size() == 1);

            auto spDoc3 = spEngine->createDocument();
            CHECK(r, spEngine->documents().size() == 2);
            CHECK(r, spEngine->objectCount("Document").current() == 2);

            spDoc3.reset();
            CHECK(r, spEngine->documents().size() == 2);
            CHECK(r, spEngine->objectCount("Document").current() == 2);
        }
        spEngine->shutdown();
    });

    set.push("Element flags", element_flags);
}
