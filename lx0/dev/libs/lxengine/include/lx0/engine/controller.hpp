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

#pragma once

#include <lx0/_detail/forward_decls.hpp>
#include <lx0/engine/dom_base.hpp>

namespace lx0 
{ 
    namespace engine 
    { 
        namespace dom_ns 
        {
            //===========================================================================//
            //!
            /*!
                \ingroup lx0_engine_dom

                A UIController is intended to map UI state such as keyboard presses, 
                mouse movements, frame updates, etc. into high-level application events
                such as "select_object", "move_camera", etc.  

                The high-level mapping is intended allow for reuse between multiple
                applications.  For example, a UIController might be set up to allow for
                FPS-like camera control.  The internal camera representation might vary
                between apps, but the WASD keyboard controls could be reused easily.
             */
            class UIController
            {
            public:
                virtual                 ~UIController() {}

                virtual     void        onLClick        (ViewPtr spView, const MouseState&, const ButtonState&, KeyModifiers) {}
                virtual     void        onLDrag         (ViewPtr spView, const MouseState& ms, const ButtonState& bs, KeyModifiers km) {}
                virtual     void        updateFrame     (ViewPtr spView, const KeyboardState& keyboard) {}
            };

            //===========================================================================//
            //!
            /*!
                \ingroup lx0_engine_dom

                An EventController is intended for use along with a UIController in a 
                processing chain.

                User event -> UIController -> Application Event -> EventController -> Implementation

                The EventController maps the application events produced by the UIController
                into actual calls in the code.
             */
            class EventController
            {
            public:
                virtual                 ~EventController() {}

                virtual     void        handleEvent     (std::string evt, lx0::lxvar params) {}
            };
        }
    }
    using namespace lx0::engine::dom_ns;
}
