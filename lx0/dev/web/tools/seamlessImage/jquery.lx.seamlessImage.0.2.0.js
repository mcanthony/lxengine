/*
    jquery.lx.seamlessImage

    === LICENSE INFO ===
    
    Copyright (c) 2011 Arthur Winters, http://athile.net
    
    Licensed under the MIT License, as follows:

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 (function ($) {

    var util = 
    {
    };
    
    var methods =
    {
        init: function (elem, options) {

            //@todo Should check that elem.tagName == "IMG"
            var src = $(elem).attr("src");

            //
            // Wait until the onload() callback is called to start doing anything
            // in order to ensure the img.width / img.height info is accurate.
            //
            var img = new Image();
            img.onload = function () {
            
                // Create a canvas and copy the style info
                var canvas = $("<canvas/>");
                canvas.attr("width", elem.width);
                canvas.attr("height", elem.height);
                canvas.attr("style", $(elem).attr("style"));
                canvas.attr("class", $(elem).attr("class"));
            
                // Store the state for the plug-in
                $.data(canvas, "seamlessImage-count", parseFloat(options.count));
                $.data(canvas, 'seamlessImage-image', img);
                $.data(canvas, "seamlessImage-onDraw", []);

                if (options.ui)
                {
                    var div = $("<div/>");
                    div.append(canvas);
                
                    var inner = $("<div/>");
                    inner.css("width", img.width + "px");
                    {
                        var slider = $("<input/>");
                        slider.attr("type", "range");
                        slider.attr("min", 25);
                        slider.attr("max", 400);
                        slider.attr("value", 100);
                        slider.attr("step", 1);
                        slider.css("width", elem.width + "px");
                        inner.append(slider);

                        var text = $("<div/>");
                        text.css("text-align", "center");
                        text.css("font-size", "80%");
                        inner.append(text);
                    }
                    div.append(inner);

                    slider.change(function() {
                        var count = parseFloat($(this).val()) / 100;
                        $.data(canvas, "seamlessImage-count", count);
                        methods.draw(canvas);
                    });
                    $.data(canvas, "seamlessImage-onDraw").push(function() {
                        var count = $.data(canvas, "seamlessImage-count");
                        text.text(count);
                        slider.val(count * 100);
                    });

                    $(elem).replaceWith(div);
                }
                else
                    $(elem).replaceWith(canvas);
                
                // The HTML is set up, so go ahead and draw the tiled images
                methods.draw(canvas);
            };
            img.src = src;     
        },

        draw: function (elem) {
            var scale = 1.0 / $.data(elem, "seamlessImage-count");
            var img = $.data(elem, 'seamlessImage-image');
            var onDraw = $.data(elem, "seamlessImage-onDraw");

            var ctx = elem[0].getContext('2d');
            ctx.save();
            ctx.translate(img.width / 2, img.height / 2);
            ctx.scale(scale, scale);

            var pattern = ctx.createPattern(img, 'repeat');            
            ctx.fillStyle = pattern;
            ctx.fillRect(-img.width / scale, -img.height / scale, 2 * img.width / scale, 2 * img.height / scale);
            ctx.restore();
                        
            // Call any callbacks registered after draw (for example, updating the
            // UI). 
            $.each(onDraw, function(i,f) { f(); });
        }
    };

    $.fn.seamlessImage = function () {

        var command = "draw";
        var userOptions = arguments[0];

        if (typeof(arguments[0]) == "string")
        {
            command = arguments[0];
            userOptions = arguments[1];
        }

        var settings = $.extend({
            'count' : 1,
            'ui' : true,
        }, userOptions);

        return this.each(function () {
            var options = settings;

            //@todo Move this merging of in-place options into the util method
            var params = $(this).attr("data-seamlessImage");
            if (params) {
                try {
                    var params = lx.color.lib._trim(params);
                    if (params[0] != "{")
                        params = "{ " + params + " }";
                    eval("params = " + params + ";");
                    options = $.extend({}, options, params);
                } catch (e) {
                }
            }

            methods.init(this, options);
        });

    };
})(jQuery);