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

        // Temporary code for testing
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