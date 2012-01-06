function loadScene (url)
{
    var scene = 
    {
        camera : null,
        lights : [],
        objects : [],
    };

    //
    // A JQuery dependency for the convenience loading the scene
    //
    //@todo The scene loading should/might be better if it were
    // moved outside the ray tracer code itself.  The ray-tracer takes
    // an already loaded scene?
    //  
    $.ajax(url + "?rand=" + Math.random(), {
        dataType : "xml",
        async : false,
        success : function(xml) {

            //
            // Use <Text> elements to allow for long-strings, then 
            // convert them inline to escaped strings.
            //
            $(xml).find("Text").each(function() {
                var text = $(this).text();
                text = text.replace(/\n/g, " ");
                text = text.replace(/\"/g, "\\\"");
                text = "\"" + text + "\""; 
                $(this).replaceWith(text);
            });


            $(xml).find("Objects").children().each(function() {
                switch (this.tagName)
                {
                case "Camera":
                    {
                        eval("var options = " + $(this).text() + ";");
                        scene.camera = options;
                    }
                    break;
                case "Plane":
                case "Sphere":
                    {
                        eval("var options = " + $(this).text() + ";");
                        scene.objects.push(new lx.vec[this.tagName](options));
                    }
                    break;
                case "Light":
                    {
                        eval("var options = " + $(this).text() + ";");
                        scene.lights.push(new lx.vec[this.tagName](options));
                    }
                    break;
                default:
                    console.log("Unrecognized Object type '" + this.tagName + "'");        
                }
            });
        },
    });

    return scene;
};
