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

#include <lx0/lxengine.hpp>

#ifdef _MSC_VER
    #include <windows.h>
#endif
#include <gl/gl.h>

//===========================================================================//
//   U I - B I N D I N G
//===========================================================================//

class UIBindingImp : public lx0::UIBinding
{
public:
    virtual void updateFrame (lx0::ViewPtr spView, const lx0::KeyboardState& keyboard)
    {
        if (keyboard.bDown[lx0::KC_ESCAPE])
            lx0::Engine::acquire()->sendEvent("quit");
        if (keyboard.bDown[lx0::KC_R])
            spView->sendEvent("redraw");
    }
};

//===========================================================================//
//   R E N D E R E R
//===========================================================================//

class Renderer : public lx0::View::Component
{
public:
    virtual void initialize(lx0::ViewPtr spView)
    {
        glClearColor(0.1f, 0.3f, 0.8f, 1.0f);
    }

    virtual void render (void)	
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }

};

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        lx0::EnginePtr spEngine = lx0::Engine::acquire();

        spEngine->initialize();
        {
            lx0::DocumentPtr spDocument = spEngine->createDocument();

            lx0::ViewPtr spView = spDocument->createView("Canvas", "view", new Renderer );
            spView->addUIBinding( new UIBindingImp );

            lx0::lxvar options;
            options.insert("title", "LxQuake2");
            options.insert("width", 640);
            options.insert("height", 480);
            spView->show(options);

            exitCode = spEngine->run();
        }
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        throw lx_fatal_exception("Fatal: unhandled std::exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
