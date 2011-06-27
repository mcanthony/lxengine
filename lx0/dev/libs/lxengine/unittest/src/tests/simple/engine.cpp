#include"main.hpp"
#include <lx0/lxengine.hpp>

using namespace lx0;

void
testset_engine(TestSet& set)
{
    set.push("acquire", [] (TestRun& r) {
        EnginePtr spEngine = Engine::acquire();
        CHECK(r, spEngine.get() != nullptr);

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
        
        auto spDoc1 = spEngine->createDocument();
        auto spDoc2 = spEngine->createDocument();
        CHECK(r, spEngine->documents().size() == 2);
        CHECK(r, spEngine->objectCount("Document").current() == 2);
        CHECK(r, spDoc1.use_count() == 2);
        CHECK(r, spDoc2.use_count() == 2);

        spEngine->closeDocument(spDoc2);
        CHECK(r, spEngine->documents().size() == 1);
        CHECK(r, spEngine->objectCount("Document").current() == 2);

        spDoc2.reset();
        CHECK(r, spEngine->objectCount("Document").current() == 1);

        // Orphan a document so the Engine has the only reference...
        CHECK(r, spEngine->documents().size() == 1);

        auto spDoc3 = spEngine->createDocument();
        CHECK(r, spEngine->documents().size() == 2);
        CHECK(r, spEngine->objectCount("Document").current() == 2);

        spDoc3.reset();
        CHECK(r, spEngine->documents().size() == 2);
        CHECK(r, spEngine->objectCount("Document").current() == 2);

        spEngine->shutdown();
    });
}
