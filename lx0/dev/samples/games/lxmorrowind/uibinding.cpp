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

#include <lx0/lxengine.hpp>

using namespace lx0;

//===========================================================================//

class UIBindingImp : public lx0::UIBinding
{
public:

    virtual void updateFrame (ViewPtr spView, const KeyboardState& keyboard)
    {
        const float step = 20.0f;

        if (keyboard.bDown[KC_ESCAPE])
            Engine::acquire()->sendEvent("quit");
        
        if (keyboard.bDown[KC_R])
            spView->sendEvent("redraw", lxvar());

        if (keyboard.bDown[KC_W])
            spView->document()->sendEvent("move_forward", lxvar(step));
        if (keyboard.bDown[KC_S])
            spView->document()->sendEvent("move_forward", lxvar(-step));
        if (keyboard.bDown[KC_A])
            spView->document()->sendEvent("move_right", lxvar(-step));
        if (keyboard.bDown[KC_D])
            spView->document()->sendEvent("move_right", lxvar(step));
        if (keyboard.bDown[KC_Q])
            spView->document()->sendEvent("move_up", lxvar(-step));
        if (keyboard.bDown[KC_E])
            spView->document()->sendEvent("move_up", lxvar(step));
    }

    void onLDrag (ViewPtr spView, const MouseState& ms, const ButtonState& bs, KeyModifiers km)
    {
        float horz = ms.deltaX() * -3.1415f / 1000.0f;
        float vert = ms.deltaY() * -3.1415f / 1000.0f;

        if (fabs(horz) > 1e-3f)
            spView->document()->sendEvent("rotate_horizontal", horz);
        if (fabs(vert) > 1e-3f)
            spView->document()->sendEvent("rotate_vertical", vert);

        spView->sendEvent("redraw");
    }
};

lx0::UIBinding*         create_uibinding()      { return new UIBindingImp; }
