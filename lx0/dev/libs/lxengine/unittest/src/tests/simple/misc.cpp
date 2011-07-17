#include"main.hpp"
#include <lx0/lxengine.hpp>
#include <lx0/util/math/noise.hpp>

void
testset_misc(TestSet& set)
{
    set.push("glm", [] (TestRun& tr) {

        glm::mat4 m(1.0f);

        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                CHECK(tr, m[c][r] == ((r == c) ? 1.0f : 0.0f));

        // glm::mat4 is set up for row-vector multiplications: therefore,
        // translation should end up in row 4.
        m = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 4.0f, 6.0f));
        CHECK(tr, m[0][0] == 1.0f);
        CHECK(tr, m[0][1] == 0.0f);
        CHECK(tr, m[3][0] == 2.0f);
        CHECK(tr, m[3][1] == 4.0f);
        CHECK(tr, m[3][2] == 6.0f);
        CHECK(tr, m[3][3] == 1.0f);
    });


    set.push("noise", [] (TestRun& r) {

        double noiseMin = std::numeric_limits<double>::max();
        double noiseMax = std::numeric_limits<double>::min();
        double noiseAvg = 0.0;

        double avg = 0.0;
        srand(512);
        for (int i = 0; i < 1000 * 1000; ++i) 
        {
            glgeom::point3f p;
            for (int j = 0; j < 3; ++j)
                p[j] = ((rand() % 20000) - 10000) / 1000.0f;
            float v = lx0::noise3d_perlin(p.x, p.y, p.z);
            
            double d = double(v);
            noiseMin = std::min(noiseMin, d);
            noiseMax = std::max(noiseMax, d);
            noiseAvg += d;
        }
        noiseAvg /= 1000 * 1000;

        CHECK(r, noiseMin >= 0.0 && noiseMin <= .2);
        CHECK(r, noiseMax <= 1.0 && noiseMax >= .8);
        CHECK(r, abs(noiseAvg - .5) < 0.05);
    });
}
