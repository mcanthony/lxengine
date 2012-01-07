function loadScene (url, substitutes)
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
                
                var text = $(this).text();
                if (substitutes)
                {
                    for (var key in substitutes)
                    {
                        var re = new RegExp("%" + key + "%", "g");
                        text = text.replace(re, JSON.stringify(substitutes[key]));
                    }
                }
                
                switch (this.tagName)
                {
                case "Camera":
                    {
                        eval("var options = " + text + ";");
                        scene.camera = options;
                    }
                    break;
                case "Plane":
                case "Sphere":
                    {
                        eval("var options = " + text + ";");
                        scene.objects.push(new lx.vec[this.tagName](options));
                    }
                    break;
                case "Light":
                    {
                        eval("var options = " + text + ";");
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
