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

#include <lx0/subsystem/canvas.hpp>
#include <lx0/prototype/misc.hpp>
#include "viewimp.hpp"
#include "rasterizer_ext.hpp"
#include "main.hpp"
#include "renderer.hpp"


using namespace lx0;
using namespace lx0::prototype;

extern lx0::Camera2 gCamera;

class Controller2
{
public:
    virtual                 ~Controller2() {}

    virtual     void        onLClick        (ViewPtr spView, const MouseState&, const ButtonState&, KeyModifiers) {}
    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard) = 0;
};

class CameraController : public Controller2
{
public:
    virtual     void        onLClick        (ViewPtr spView, const MouseState&, const ButtonState&, KeyModifiers);
    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard);
};

void
CameraController::onLClick (ViewPtr spView, const MouseState& mouse, const ButtonState&, KeyModifiers)
{
    spView->sendEvent("select_object", lxvar(mouse.x, mouse.y));
}

void
CameraController::updateFrame (ViewPtr spView, const KeyboardState& keyboard)
{
    const float kStep = 2.0f;

    if (keyboard.bDown[KC_ESCAPE])
        Engine::acquire()->sendMessage("quit");

    if (keyboard.bDown[KC_W])
        spView->sendEvent("move_forward", kStep);
    if (keyboard.bDown[KC_S])
        spView->sendEvent("move_backward", kStep);
    if (keyboard.bDown[KC_A])
        spView->sendEvent("move_left", kStep);
    if (keyboard.bDown[KC_D])
        spView->sendEvent("move_right", kStep);
    if (keyboard.bDown[KC_R])
        spView->sendEvent("move_up", kStep);
    if (keyboard.bDown[KC_F])
        spView->sendEvent("move_down", kStep);

    if (keyboard.bDown[KC_M])
        spView->sendEvent("cycle_viewmode", lxvar());
}




class LxCanvasImp : public lx0::ViewImp
{
public:
                        LxCanvasImp();
                        ~LxCanvasImp();

    virtual void        createWindow    (lx0::View* pHostView, size_t& handle, unsigned int& width, unsigned int& height);
    virtual void        destroyWindow   (void);
    virtual void        show            (lx0::View* pHostView, lx0::Document* pDocument);

    virtual     void        _onElementAdded             (lx0::ElementPtr spElem) {}
    virtual     void        _onElementRemoved           (lx0::ElementPtr spElem) {}

    virtual     void        updateBegin     (void) {}
    virtual     void        updateFrame     (lx0::DocumentPtr spDocument);
    virtual     void        updateEnd       (void) {}

    virtual     void        handleEvent     (std::string evt, lx0::lxvar params);

protected:
    lx0::View*                   mpHostView;
    lx0::DocumentPtr             mspDocument;
    lx0::CanvasHost              mHost;
    std::auto_ptr<lx0::CanvasGL> mspWin;
    Renderer                     mRenderer;
    CameraController             mController;
};

ViewImp* create_lxcanvasimp()
{
    return new LxCanvasImp;
}

LxCanvasImp::LxCanvasImp()
{
    lx_debug("LxCanvasImp ctor");
}

LxCanvasImp::~LxCanvasImp()
{
    lx_debug("LxCanvasImp dtor");
}

void 
LxCanvasImp::createWindow (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height)
{
    width = 800;
    height = 400;

    mspWin.reset( new CanvasGL("Terrain Sample (OpenGL 3.2)", 16, 16, width, height, false) );
    handle = mspWin->handle();

    mRenderer.initialize();
    mRenderer.resize(width, height);

    mspWin->slotRedraw += [&]() { mRenderer.render(); };
    mspWin->slotLMouseClick += [&](const MouseState& ms, const ButtonState& bs, KeyModifiers km) { 
        mController.onLClick(mpHostView->shared_from_this(), ms, bs, km);
    };
    mspWin->slotLMouseDrag += [&](const MouseState& ms, const ButtonState& bs, KeyModifiers km) {
        
        // Rotate horizontal
        rotate_horizontal(gCamera, ms.deltaX() * -3.14f / 1000.0f );

        // Rotate vertical..
        //@todo:  only if not going to cause the camera to be staring straight up or straight down
        float vertAngle = ms.deltaY() * -3.1415f / 1000.0f;
        rotate_vertical(gCamera, vertAngle);
        
        mspWin->invalidate(); 
    };
}

void
LxCanvasImp::destroyWindow (void)
{
    mspWin->destroy();
}

void 
LxCanvasImp::show (View* pHostView, Document* pDocument)
{
    mpHostView = pHostView;
    mspDocument = pDocument->shared_from_this();
    mRenderer.mspDocument = mspDocument;
    mspWin->show();
}

void 
LxCanvasImp::updateFrame (DocumentPtr spDocument) 
{
    mController.updateFrame(mpHostView->shared_from_this(), mspWin->keyboard());
    mRenderer.update();

    mspWin->invalidate(); 
}

void 
LxCanvasImp::handleEvent (std::string evt, lx0::lxvar params)
{
    bool bInvalidate = true;

    if (evt == "redraw")
        bInvalidate = true;
    else if (evt == "move_forward")
        move_forward(gCamera, params.asFloat());
    else if (evt == "move_backward")
        move_backward(gCamera, params.asFloat());
    else if (evt == "move_left")
        move_left(gCamera, params.asFloat());
    else if (evt == "move_right")
        move_right(gCamera, params.asFloat());
    else if (evt == "move_up")
        move_vertical(gCamera, params.asFloat());
    else if (evt == "move_down")
        move_vertical(gCamera, -params.asFloat());
    else if (evt == "select_object")
    {
        auto& spItem = mRenderer.select( params.at(0).asInt(), params.at(1).asInt() );
        auto spElement = spItem->getData<ElementPtr>();
        std::string name = spElement
            ? spElement->attr("image").query("unknown").c_str()
            : "no associated element";
        printf("Select: %s (%s)\n", spItem->spMaterial->mShaderFilename.c_str(), name.c_str());
    }
    else if (evt == "cycle_viewmode")
        mRenderer.cycleViewMode();
    else
    {
        lx_warn("Unhandled event '%s'", evt.c_str());
        bInvalidate = false;
    }

    if (bInvalidate)
        mspWin->invalidate();
}
