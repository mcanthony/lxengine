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

// Standard headers
#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

// Library headers
#include <boost/program_options.hpp>

// Lx0 headers
#include <lx0/core/core.hpp>
#include <lx0/core/math/matrix4.hpp>
#include <lx0/core/util/util.hpp>
#include <lx0/canvas/canvas.hpp>
#include <lx0/prototype/prototype.hpp>
#include <lx0/view.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>

using namespace lx0::core;
using namespace lx0::prototype;
using namespace lx0::canvas::platform;

Camera             gCamera;

//===========================================================================//

class Renderer
{
public:
    void initialize()
    {
        set(gCamera.mPosition, 10, 10, 15);
        set(gCamera.mTarget, 0, 0, 0);
        set(gCamera.mWorldUp, 0, 0, 1);
        gCamera.mFov = 60.0f;
        gCamera.mNear = 0.01f;  // 1 cm
        gCamera.mFar = 2000.0f; // 2 km
    }  

    void 
    resize (int width, int height)
    {

    }

    void 
    render (void)	
    {
    }

protected:
};

//===========================================================================//

class LxCanvasImp : public ViewImp
{
public:
    virtual void        createWindow    (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height);
    virtual void        destroyWindow   (void);
    virtual void        show            (View* pHostView, Document* pDocument);

    virtual     void        _onElementAdded             (ElementPtr spElem) {}
    virtual     void        _onElementRemoved           (ElementPtr spElem) {}

    virtual     void        updateBegin     (void) {}
    virtual     void        updateFrame     (DocumentPtr spDocument);
    virtual     void        updateEnd       (void) {}

protected:
    std::auto_ptr<CanvasGL> mspWin;
    CanvasHost              mHost;
    Renderer                mRenderer;
};

void 
LxCanvasImp::createWindow (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height)
{
    width = 800;
    height = 400;

    mspWin.reset( new CanvasGL("Lx Raytracer Sample", 16, 16, width, height, false) );
    handle = mspWin->handle();

    mRenderer.initialize();
    mRenderer.resize(width, height);

    mspWin->slotRedraw += [&]() { mRenderer.render(); };
}

void
LxCanvasImp::destroyWindow (void)
{
    mspWin->destroy();
}

void 
LxCanvasImp::show (View* pHostView, Document* pDocument)
{
    mspWin->show();
}

void 
LxCanvasImp::updateFrame (DocumentPtr spDocument) 
{
    if (mspWin->keyboard().bDown[KC_ESCAPE])
        Engine::acquire()->sendMessage("quit");
}

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        EnginePtr   spEngine   = Engine::acquire();
        spEngine->addViewPlugin("LxCanvas", [] (View* pView) { return new LxCanvasImp; });
        
        DocumentPtr spDocument = spEngine->loadDocument("media2/appdata/sm_terrain/scene.xml");
        ViewPtr     spView     = spDocument->createView("LxCanvas", "view");
        spView->show();

        exitCode = spEngine->run();
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
