(function ($) {

    function _tryEval(s, def) {
        try {
            var value;
            eval("value = " + s);
            return value;
        }
        catch (e) {
            return def;
        }
    }

    var methods =
    {
        init: function (options) {

            // Preconditions
            //
            if (this.tagName != "CANVAS") throw "jQuery.raytrace only valid on CANVAS elements";

            //
            // If there's already an engine object associated with the canvas, reuse that object;
            // this is mostly done simply to abort any prior rendering process (rather than any
            // savings from reuse).
            //
            var engine = $.data(this, "raytracer-engine") || new Engine();
            $.data(this, "raytracer-engine", engine);

            var scene = options.scene || loadScene(options.sceneUrl, options.template);

            engine.gametick = 0;
            engine.run(new raytracer.RenderLoop(this, scene, options));
        },

        extendOptions: function (baseOptions) {

            var defaultOptions = {};
            var inlineOptions = _tryEval($(this).attr("data-options"), {});
            var options = $.extend(defaultOptions, baseOptions, inlineOptions);

            options.sceneUrl = $(this).attr("data-src") || options.sceneUrl;

            var template = $(this).text();

            //if (template) template = template.replace(/^[\s\n]+/, "").replace(/[\s\n]+$/, "");
            if (template.length < 1) template = $(this).attr("data-template") || "{}";
            options.template = _tryEval(template, {});
            return options;
        }
    };

    $.fn.raytracer = function (options) {
        return this.each(function () {
            var elemOptions = methods.extendOptions.call(this, options);
            methods.init.call(this, elemOptions);
        });
    };

})(jQuery);
