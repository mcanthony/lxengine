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

static void
generateNoiseCubeMap()
{
    using namespace glgeom;

    for (int f = 0; f < 6; ++f)
    {
        image3f img (256,256);
        float s = img.width();

        for (int y = 0; y < img.width(); ++y)
        {
            for (int x = 0; x < img.height(); ++x)
            {
                float dx = 2.0f * (float(x) / float(img.width() - 1) - .5f);
                float dy = 2.0f * (float(y) / float(img.height() - 1) - .5f);

                vector3f v;

                switch (f)
                {
                case 0: v = vector3f( 1, -dy, -dx); break;
                case 2: v = vector3f(dx, 1, dy); break;
                case 4: v = vector3f(dx, -dy,  1); break;

                case 1: v = vector3f(-1, -dy, dx); break;
                case 3: v = vector3f(dx, -1, -dy); break;
                case 5: v = vector3f(-dx, -dy, -1); break;
                }
                v = normalize(v);
                glm::vec3 u;

                glm::vec3 large(500000, 500000, 500000);

                u.x = lx0::noise3d_perlin(v.vec * (0.3f + lx0::noise3d_perlin(v.vec * 2.5f)));
                u.y = lx0::noise3d_perlin(v.vec * 1.4f);
                glm::vec3 a = glgeom::pattern_spot_dimmed( glm::vec3(1, 1, 1), glm::vec3(.2f, .2f, .02f), .49f, glm::vec2(u.x, u.y) );

                u.x = lx0::noise3d_perlin(v.vec * (12.0f + 12.0f * lx0::noise3d_perlin(v.vec * 2.5f)) + large);
                u.y = lx0::noise3d_perlin(42.0f * v.vec * u.x );
                glm::vec3 b = glgeom::pattern_spot_dimmed( glm::vec3(.5f, .5f, .3f), glm::vec3(.17f, .3f, .2f), .295f, glm::vec2(u.x, u.y) );
                           
                color3f c = color3f( glm::mix(a, b, .315f) );

                img.set(x, y, c);
            }
        }
        const char* filename[] = 
        {
            "xpos.png",
            "xneg.png",
            "ypos.png",
            "yneg.png",
            "zpos.png",
            "zneg.png",
        };

        std::string file = "media2/textures/cubemaps/noise000/";
        file += filename[f];
        lx0::save_png(img, file.c_str());
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
