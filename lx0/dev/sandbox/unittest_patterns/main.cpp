//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

// Standard headers
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

// Lx0 headers
#include <lx0/lxengine.hpp>
#include <lx0/prototype/misc.hpp>
#include <glgeom/ext/patterns.hpp>
#include <glgeom/prototype/image.hpp>

using namespace lx0;


//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

static void runTest (lxvar& results, std::string testName, std::function<glm::vec3 (const glm::vec2&)> func)
{
    glgeom::image3f img(128 * 3, 128 * 3);

    for (int y = 0; y < img.height(); ++y)
    {
        for (int x = 0; x <img.width(); ++x)
        {
            glm::vec3 sum (0, 0, 0);
            for (int sy = -1; sy <= 1; ++sy)
            {
                for (int sx = -1; sx <= 1; ++sx)
                {
                    glm::vec2 uv;
                    uv.x = (float(x) + 0.5f + float(sx) / 3.0f) / img.width();
                    uv.y = (float(y) + 0.5f + float(sy) / 3.0f) / img.height();
                    uv *= 3;
                    uv += glm::vec2(-1, -1);
                   
                    sum += func(uv);
                }
            }
            sum /= 9.0f;

            // Dim outside the main 0 to 1 uv range and draw boundary lines
            if (x == 127 || x == 2 * 128)
            {
                sum = glm::vec3(1, 0, 0);
            }
            else if (y == 127 || y == 2 * 128)
            {
                sum = glm::vec3(0, 1, 0);
            }
            else if ((x < 128 || x >= 2 * 128 || y < 128 || y >= 2 * 128))
            {
                if ((x + y) % 2 == 0)
                {
                    sum += glm::vec3(.5f, .5f, .5f);
                    sum /= 2.0f;
                }
                else
                    sum += glm::vec3(.15f, .15f, .15f);
            }

            img.set(x, y, glgeom::color3f(sum));
        }
    }

    std::string imageName = (boost::format("unittest_results\\glgeom\\patterns\\basic\\%1%.png") % testName).str();
    lx0::save_png(img, imageName.c_str());

    lxvar test;
    test["type"]     = "image";
    test["name"]     = testName;
    test["image"]    = imageName;
    results.push(test);
}

static void runTestSet(lxvar& results)
{
    runTest(results, "checker", [](const glm::vec2& uv) {
        return glgeom::pattern_checker( glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), uv );
    });

    runTest(results, "spot_r0.02", [](const glm::vec2& uv) {
        return glgeom::pattern_spot( glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), .02f, uv );
    });
    runTest(results, "spot_r0.10", [](const glm::vec2& uv) {
        return glgeom::pattern_spot( glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), .10f, uv );
    });
    runTest(results, "spot_r0.25", [](const glm::vec2& uv) {
        return glgeom::pattern_spot( glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), .25f, uv );
    });
    runTest(results, "spot_r0.50", [](const glm::vec2& uv) {
        return glgeom::pattern_spot( glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), .50f, uv );
    });
    runTest(results, "spot_r0.65", [](const glm::vec2& uv) {
        return glgeom::pattern_spot( glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), .65f, uv );
    });



    runTest(results, "wave_w0.10", [](const glm::vec2& uv) {
        return glgeom::pattern_wave( glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), .1f, uv );
    });
    runTest(results, "wave_w0.25", [](const glm::vec2& uv) {
        return glgeom::pattern_wave( glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), .25f, uv );
    });
    runTest(results, "wave_w0.50", [](const glm::vec2& uv) {
        return glgeom::pattern_wave( glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), .5f, uv );
    });

}

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        EnginePtr spEngine = Engine::acquire();
        {
            system("mkdir unittest_results");
            system("mkdir unittest_results\\glgeom");
            system("mkdir unittest_results\\glgeom\\patterns");
            system("mkdir unittest_results\\glgeom\\patterns\\basic");

            lxvar results;
            results["date"] = lx0::lx_ctime();
            runTestSet(results["tests"]);

            std::ofstream file;
            file.open("unittest_results\\glgeom\\patterns\\basic\\results.json");
            file << lx0::format_json(results) << std::endl;
            file.close();
        }
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
