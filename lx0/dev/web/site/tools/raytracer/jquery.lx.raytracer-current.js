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

    var imageCache = acquireCache('imageCache');

    var methods =
    {
        _raytrace: function (options) {

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

            //
            if (options.linkToImage)
            {
                var next = options.oncomplete || function () { };
                options.oncomplete = (function (next, elem) {
                    return function(stats) { 
                        methods._ensureLink.call(elem); 
                        next(stats); 
                    };
                })(next, this);
            }

            var scene = options.scene || loadScene(options.sceneUrl, options.template);

            engine.gametick = 0;
            engine.run(new raytracer.RenderLoop(this, scene, options));
        },

        _ensureLink : function() {
            var parent = $(this).parent();
            if (parent.tagName != "A") {
                parent = $("<A/>");
                $(this).wrap(parent);
                parent = $(this).parent();
            }

            parent.attr("target", "_blank");
            parent.attr("href", this.toDataURL("image/png"));
        },

        init: function (options) {

            // If there's a cacheId associated with this element, then 
            // check if there's a cached image that can be drawn directly;
            // otherwise, set up the options to save the image to the cache when the
            // ray-tracing is done.
            //
            if (options.cacheId) {
                var id = options.cacheId;

                if (imageCache.data[id]) {
                    var ctx = this.getContext('2d');
                    var img = new Image();
                    var elem = this;
                    img.onload = function () {
                        ctx.drawImage(this, 0, 0);
                        
                        if (options.linkToImage)
                            methods._ensureLink.call(elem);
                    };
                    img.src = imageCache.data[id];
                }
                else {
                    var next = options.oncomplete || function () { };
                    options.oncomplete = (function (next, id) {
                        return function (stats) {
                            imageCache.data[id] = stats.dataUri;
                            imageCache.save();
                            next(stats);
                        }
                    })(next, id);

                    methods._raytrace.call(this, options);
                }
            }
            else
                methods._raytrace.call(this, options);
        },

        queryCache: function (options) {
            var id = options.cacheId;
            return (id && imageCache.data[id] != undefined);
        },

        extendOptions: function (baseOptions) {

            var defaultOptions = {
                linkToImage : false,
            };
            var inlineOptions = _tryEval($(this).attr("data-options"), {});
            var options = $.extend(defaultOptions, baseOptions, inlineOptions);

            options.cacheId = $(this).attr("data-cacheId") || undefined;
            options.sceneUrl = $(this).attr("data-src") || options.sceneUrl;

            var template = $(this).text();

            //if (template) template = template.replace(/^[\s\n]+/, "").replace(/[\s\n]+$/, "");
            if (template.length < 1) template = $(this).attr("data-template") || "{}";
            options.template = _tryEval(template, {});
            return options;
        }
    };

    $.fn.raytracer = function (options) {

        var method = 'init';

        if (methods[options] != undefined) {
            method = options;
            options = arguments[1];
        }

        var ret;
        var chain = this.each(function () {
            var elemOptions = methods.extendOptions.call(this, options);
            ret = methods[method].call(this, elemOptions);
        });
        return (ret != undefined) ? ret : chain;
    };

})(jQuery);
