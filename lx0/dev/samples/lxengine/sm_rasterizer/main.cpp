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
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

#include <iostream>
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/javascript.hpp>
#include "renderer.hpp"

using namespace lx0;

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

#include <glgeom/prototype/image.hpp>
#include <lx0/prototype/misc.hpp>
#include <glgeom/ext/patterns.hpp>
#include <glgeom/ext/mappers.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

static 
void
generateNoiseCubeMap ()
{
    boost::mt19937 generator;         
    boost::uniform_int<> range(0,1023);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die(generator, range);

    glm::vec3 offset(223.33f, 3125.32138f, 2734.234221f);

    std::vector<glgeom::image3f> imageSet;
    glgeom::generate_cube_map(imageSet, 256, 256, [&] (const glm::vec3& v) -> glm::vec3 {

        glm::vec3 water;
        if (1) {
            glm::vec3 sum;
            for (int i = 1; i <= 8; ++i)
            {
                glm::vec2 uv;
                uv = glgeom::mapper_spherical(v, glm::vec2(4, 4));
                uv.x += i * 4.0f * lx0::noise3d_perlin(v * 1.2f + glm::vec3(234, i * 122, 621));
                uv.y += i * 4.0f * lx0::noise3d_perlin(v * 1.2f + glm::vec3(i * 532, i * 7732, 23462));

                glgeom::color3f c;
                sum += glgeom::pattern_spot_dimmed(
                        glm::vec3(13/255.0f, 128/255.0f, 255/255.0f),
                        glm::vec3(237/255.0f, 241/255.0f, 244/255.0f),
                        .45f,
                        uv
                    );
            }

            water = sum / 8.0f;
        }

        glm::vec3 grass;
        {
            glm::vec3 sum;
            for (int i = 1; i <= 3; ++i)
            {
                glm::vec2 uv;
                uv = glgeom::mapper_spherical(v, glm::vec2(4, 4));
                uv.x += i * 4.0f * lx0::noise3d_perlin(v * 5.2f + glm::vec3(234, i * 122, 621));
                uv.y += i * 4.0f * lx0::noise3d_perlin(v * 4.2f + glm::vec3(i * 532, i * 7732, 23462));

                glgeom::color3f c;
                sum += glgeom::pattern_checker(
                        glm::vec3(174/255.0f, 165/255.0f, 117/255.0f),
                        glm::vec3(90/255.0f, 47/255.0f, 12/255.0f),
                        uv
                    );
            }

            grass = sum / 8.0f;
        }

        float b =  lx0::noise3d_perlin(v * 3.2f + glm::vec3(234, 3112, 621));
        b += lx0::noise3d_perlin(v * 0.62f + glm::vec3(234, 3112, 621));
        b /= 2.0f;
        b = pow(.45f + b, 3);
        b = glm::clamp(b, 0.0f, 1.0f);
        if (b < .25f) b = powf(b, 1.25f);
        b = .55f + b * .45f;

        glm::vec3 sum;
        sum = glm::mix(water, grass, b);

        return sum;
    });

    const char* filename[] = 
    {
        "xpos.png",
        "xneg.png",
        "ypos.png",
        "yneg.png",
        "zpos.png",
        "zneg.png",
    };

    for (int i = 0; i < 6; ++i)
    {
        std::string file = "media2/textures/cubemaps/noise000/";
        file += filename[i];
        lx0::save_png(imageSet[i], file.c_str());
    }
}

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        EnginePtr spEngine = Engine::acquire();
        
        spEngine->globals().add("file",      lx0::eAcceptsString,  lx0::validate_filename());
        spEngine->globals().add("output",    lx0::eAcceptsString,  lx0::validate_string());
        spEngine->globals().add("width",     lx0::eAcceptsInt,     lx0::validate_int_range(32, 4096), 512);
        spEngine->globals().add("height",    lx0::eAcceptsInt,     lx0::validate_int_range(32, 4096), 512);

        if (spEngine->parseCommandLine(argc, argv, "file"))
        {
            // Temp
            {
                generateNoiseCubeMap();
            }
            //


            spEngine->attachComponent(lx0::createJavascriptSubsystem());
            spEngine->attachComponent(createScriptHandler());
        
            DocumentPtr spDocument = spEngine->loadDocument(spEngine->globals().find("file"));
            spDocument->addController( create_controller(spDocument) );

            ViewPtr spView = spDocument->createView("Canvas", "view", create_renderer() );
            spView->addUIBinding( create_uibinding() );

            lxvar options;
            options.insert("title",  "LxEngine Rasterizer Sample");
            options.insert("width",  spEngine->globals().find("width"));
            options.insert("height", spEngine->globals().find("height"));
            spView->show(options);

            lxvar outputVar = spEngine->globals().find("output");
            if (outputVar.is_defined())
            {
                spView->sendEvent("screenshot", outputVar);
                spEngine->sendEvent("quit");
            }

            exitCode = spEngine->run();
        }
        spEngine->shutdown();
    }
    catch (lx0::error_exception& e)
    {
        std::cerr << "Error: " << e.details().c_str() << std::endl
                    << "Code: " << e.type() << std::endl
                    << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Fatal: unhandled std::exception.\n"
            << "Exception: " << e.what();
    }

    return exitCode;
}
