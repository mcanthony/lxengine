//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)

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
#include <ctime>
#include <boost/format.hpp>
#include <glgeom/prototype/image.hpp>
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/javascript.hpp>
#include <lx0/prototype/misc.hpp>
#include <lx0/prototype/control_structures.hpp>
#include <lx0/util/misc.hpp>

#include <windows.h>
#include <gl/gl.h>

using namespace lx0;

//===========================================================================//

glgeom::image3f img;

//===========================================================================//

class UIBindingImp : public lx0::UIBinding
{
public:

    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard)
    {
        if (keyboard.bDown[KC_ESCAPE])
            Engine::acquire()->sendEvent("quit");
 
        timed_gate_block(50, { 
            spView->sendEvent("redraw", lxvar());
        });
    }

    virtual     void        onKeyDown       (ViewPtr spView, int keyCode)
    {
        if (keyCode == KC_S)
        {
            time_t rawtime;
            time(&rawtime);
            struct tm* timeinfo = localtime(&rawtime);
            
            std::string filename = boost::str( boost::format("procedural2d_screenshot-%04d%02d%02d_%02d%02d%02d.png") 
                % (timeinfo->tm_year + 1900) 
                % (timeinfo->tm_mon + 1)
                % (timeinfo->tm_mday)
                % (timeinfo->tm_hour)
                % (timeinfo->tm_min)
                % (timeinfo->tm_sec)
                );
            lx0::save_png(img, filename.c_str());
        }
    }
};


//===========================================================================//

class Renderer : public View::Component
{
public:
    Renderer()
        : mId (0)
    {
    }

    virtual void initialize(ViewPtr spView)
    {
        GLuint id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        mId = id;
    }  

    virtual void render (void)	
    {
        glEnable(GL_TEXTURE_2D);
        
        glBindTexture(GL_TEXTURE_2D, mId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0, GL_RGB, GL_FLOAT, img.ptr());
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glColor3f(0, 1, 1);
        glBegin(GL_QUADS);
            glTexCoord2f(0, 1);
            glVertex3f(-1, -1, 0);

            glTexCoord2f(1, 1);
            glVertex3f(1, -1, 0);

            glTexCoord2f(1, 0);
            glVertex3f(1, 1, 0);

            glTexCoord2f(0, 0);
            glVertex3f(-1, 1, 0);
        glEnd();
    }

protected:
    GLuint mId;
};

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    using namespace lx0;

    int exitCode = -1;
  
    try
    {
        EnginePtr   spEngine   = Engine::acquire();
        spEngine->globals().add("file",     eAcceptsString, lx0::validate_filename(), "media2/appdata/procedural2d/basic.xml");

        if (spEngine->parseCommandLine(argc, argv, "file"))
        {
            spEngine->attachComponent(lx0::createJavascriptSubsystem());

            DocumentPtr spDocument = spEngine->loadDocument(spEngine->globals().find("file"));

            //
            // Find the <Texture> element to determine the height and width, as
            // well as the callback function to use for generating the pixels
            //
            //@todo create a view per Texture element
            //
            auto spTexture = spDocument->getElementsByTagName("Texture")[0];
            int width = spTexture->value().find("width");
            int height = spTexture->value().find("height");
            std::string sourceName = spTexture->value().find("source")[1];

            //
            // Load any <Script> elements, as these contain the code to do the
            // actual generation
            //
            auto sourceElems = spDocument->getElementsByTagName("Script");
            for (auto it = sourceElems.begin(); it != sourceElems.end(); ++it)
            {
                auto text = (*it)->value().as<std::string>();
                spDocument->getComponent<IJavascriptDoc>()->run(text);
            }

            //
            // Resize the image to the requested size
            //
            img = glgeom::image3f(width, height);

            //
            // Do the actual image generation
            //
            // This works by running some code inside the Javascript "context", which allows
            // us to grab a function handle to the source function.  Then a GLGeom function
            // is used to iterate over the pixels in the image and call the function.
            //
            lx0::Timer timer;
            timer.start();
            
            auto spIJavascriptDoc = spDocument->getComponent<IJavascriptDoc>();
            spIJavascriptDoc->runInContext([&](void) 
            {
                auto func = spIJavascriptDoc->acquireFunction<glm::vec3 (float, float)>(sourceName.c_str());

                glgeom::generate_image3f(img, [&](glm::vec2 st, glm::ivec2 xy) -> glm::vec3 {
                    return func(st.s, st.t);
                });
            });

            timer.stop();
            std::cout << "Generation time [" << timer.totalMs() << " ms]\n";

            //
            // Construct and present a view of the finished image
            //
            ViewPtr spView = spDocument->createView("Canvas", "view", new Renderer );
            spView->addUIBinding( new UIBindingImp );

            lxvar options;
            options.insert("title",  "2D Procedural Generator");
            options.insert("width",  img.width());
            options.insert("height", img.height());
            spView->show(options);

            exitCode = spEngine->run();
        }
        spEngine->shutdown();
    }
    catch (lx0::error_exception& e)
    {
        std::cout << "Error: " << e.details().c_str() << std::endl
            << "Code: " << e.type() << std::endl
            << std::endl;
    }

    return exitCode;
}
