var uiBinding =
{
    onKeyDown : function (view, keyCode)
    {
        __lx_print("onKeyDown called! KeyCode = " + keyCode);

        if (keyCode == lx0.KC_G)
            view.sendEvent("change_geometry", "next");
        if (keyCode == lx0.KC_F)
            view.sendEvent("change_geometry", "prev");
        if (keyCode == lx0.KC_M)
            view.sendEvent("next_material");
        if (keyCode == lx0.KC_N)
            view.sendEvent("prev_material");

        // Temporary code for testing
        if (keyCode == lx0.KC_ESCAPE)
            engine.sendEvent("quit");
    },

    updateFrame : function (view, keyboard)
    {
        if (keyboard.bDown[lx0.KC_ESCAPE])
            engine.sendEvent("quit");
        if (keyboard.bDown[lx0.KC_R])
            view.sendEvent("redraw");
    }
};


function main()
{
    lx0.log("Beginning main script");

    lx0.message("Loading document...");
    var document = engine.loadDocument("media2/appdata/tutorial_03/document.xml");

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