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

extern lx0::Camera2 gCamera;

class CameraController : public lx0::UIBinding
{
public:
    virtual     void        onLClick        (ViewPtr spView, const MouseState&, const ButtonState&, KeyModifiers);
    virtual     void        onLDrag         (ViewPtr spView, const MouseState& ms, const ButtonState& bs, KeyModifiers km);
    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard);
};

void
CameraController::onLClick (ViewPtr spView, const MouseState& mouse, const ButtonState&, KeyModifiers)
{
    spView->sendEvent("select_object", lxvar(mouse.x, mouse.y));
}

void
CameraController::onLDrag (ViewPtr spView, const MouseState& ms, const ButtonState& bs, KeyModifiers km)
{
    // Rotate horizontal
    rotate_horizontal(gCamera, ms.deltaX() * -3.14f / 1000.0f );

    // Rotate vertical..
    //@todo:  only if not going to cause the camera to be staring straight up or straight down
    float vertAngle = ms.deltaY() * -3.1415f / 1000.0f;
    rotate_vertical(gCamera, vertAngle);
        
    spView->sendEvent("redraw", lxvar());
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

    spView->sendEvent("redraw", lxvar());
}

lx0::UIBinding*        create_camera_controller() { return new CameraController; }

class EventHandler : public lx0::Controller
{
public:
    virtual void handleEvent(std::string evt, lx0::lxvar params)
    {
        if (evt == "move_forward")
            move_forward(gCamera, params.as<float>());
        else if (evt == "move_backward")
            move_backward(gCamera, params.as<float>());
        else if (evt == "move_left")
            move_left(gCamera, params.as<float>());
        else if (evt == "move_right")
            move_right(gCamera, params.as<float>());
        else if (evt == "move_up")
            move_vertical(gCamera, params.as<float>());
        else if (evt == "move_down")
            move_vertical(gCamera, -params.as<float>());
    }
};

lx0::Controller*   create_event_controller()
{
    return new EventHandler;
}
