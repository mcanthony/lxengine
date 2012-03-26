//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2012 athile@athile.net (http://www.athile.net)

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
//   U I B I N D I N G
//===========================================================================//

var uiBinding =
{
    onKeyDown: function (view, keyCode) {
        if (keyCode == lx0.KC_G) {
            lx0.message("Changing to next model...");
            view.sendEvent("change_geometry", "next");
        }
        if (keyCode == lx0.KC_F) {
            lx0.message("Changing to previous model...");
            view.sendEvent("change_geometry", "prev");
        }
        if (keyCode == lx0.KC_M)
            view.sendEvent("next_material");
        if (keyCode == lx0.KC_N)
            view.sendEvent("prev_material");

        if (keyCode == lx0.KC_R)
            view.sendEvent("toggle_rotation");
        if (keyCode == lx0.KC_W)
            view.sendEvent("toggle_wireframe");
        if (keyCode == lx0.KC_A) {
            lx0.message("Changing render algorithm...");
            view.sendEvent("cycle_renderalgorithm");
        }

        if (keyCode == lx0.KC_Z)
            view.sendEvent("zoom_in");
        if (keyCode == lx0.KC_X)
            view.sendEvent("zoom_out");

        if (keyCode == lx0.KC_SPACE)
            view.sendEvent("cancel_event");

        if (keyCode == lx0.KC_ESCAPE)
            engine.sendEvent("quit");
    },

    updateFrame: function (view, keyboard) {
        if (keyboard.bDown[lx0.KC_ESCAPE])
            engine.sendEvent("quit");
        if (keyboard.bDown[lx0.KC_R])
            view.sendEvent("redraw");
    }
};

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

function main()
{
    lx0.log("Beginning main script");

    lx0.message("Loading document...");
    var document = engine.loadDocument("media2/appdata/tutorial_04/document.xml");

    lx0.message("Creating view...");
    var view = document.createView("Canvas", "view", "Renderer");
    view.show({
        title : "Tutorial 4",
        width : 512,
        height : 512
    });

    lx0.message("\n");
    lx0.message("Press ESCAPE to quit");

    view.addUIBinding(uiBinding);
}

