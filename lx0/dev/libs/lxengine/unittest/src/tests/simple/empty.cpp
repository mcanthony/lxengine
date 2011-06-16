#include"main.hpp"

void
testset_empty(TestSet& set)
{
    set.push("empty test", [] (TestRun& r) {
        /* Do nothing test */
    });
    set.push("check true", [] (TestRun& r) {
        CHECK(r, true);
    });
    set.push("check zero", [] (TestRun& r) {
        CHECK_CMP(r, 0.0f, 0.0f, 1e-6f);
    });
}
