/*
    jquery.lx.imageEffects

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
        elementOptions: function (elem, name) {
            var options = {};
            var optionsAttr = $(elem).attr("data-" + name);
            if (optionsAttr) {
                try {
                    // Use eval(), not JSON.parse() so the options do not have to be
                    // in the strictly well-formed format.
                    eval("options =  " + optionsAttr + ";");
                } catch (e) {
                }
            }
            return options;
        }
    };

    var effects =
    {
        _perpixel: function (pixels, width, height, op) {
            for (var y = 0; y < height; ++y) {
                for (var x = 0; x < width; ++x) {
                    var offset = ((y * width) + x) * 4;
                    var r = pixels[offset + 0];
                    var g = pixels[offset + 1];
                    var b = pixels[offset + 2];
                    var a = pixels[offset + 3];
                    var rgba = op(r,g,b,a);
                    pixels[offset+0] = rgba[0];
                    pixels[offset+1] = rgba[1];
                    pixels[offset+2] = rgba[2];
                    pixels[offset+3] = rgba[3];
                }
            }
        },

        _ops : {
            grayscale : function(r, g, b, a) {
                var gray = (r + g + b) /3;
                return [ gray, gray, gray, a];
            },

            grayscaleBiasRed : function(r, g, b, a) {
                var gray = effects._ops.grayscale(r,g,b,a);
                gray[0] = (gray[0] + r) / 2;
                return gray;
            },
            grayscaleBiasGreen : function(r, g, b, a) {
                var gray = effects._ops.grayscale(r,g,b,a);
                gray[1] = (gray[1] + g) / 2;
                return gray;
            },
            grayscaleBiasBlue : function(r, g, b, a) {
                var gray = effects._ops.grayscale(r,g,b,a);
                gray[2] = (gray[2] + b) / 2;
                return gray;
            },
        },

        none : function() { },

        hueBuckets : function(pixels, width, height) {
            var bucketSize = 360 / 3;
            effects._perpixel(pixels, width, height, function(r, g, b, a) {
                var rgba = lx.color.parse('rgb', r, g, b);
                var hue = rgba.hue();
                var hueCenter = Math.floor(hue / bucketSize) * bucketSize + bucketSize / 2;
                var biasedHue = .2 * hue + .8 * hueCenter;
                var hsv = lx.color.parse('hsv', biasedHue, rgba.saturation(), rgba.value());
                return [hsv.red(), hsv.green(), hsv.blue(), a];
            });
        },
    }

    //
    // Automatically generate functions for each of the per-pixel ops
    //
    $.each(effects._ops, function(key, value) {
        effects[key] = function(pixels, width, height) {
            effects._perpixel(pixels, width, height, value);
        };
    });


    var methods =
    {
        init: function (elem, options) {

            //
            // Set up some state on the object
            //
            var state =
            {
        }
        $.data(elem, "imageEffects", state);

        var src = $(elem).attr("src");

        //
        // Wait until the onload() callback is called to start doing anything
        // in order to ensure the img.width / img.height info is accurate.
        //
        var img = new Image();
        img.onload = function () {

            var canvas = $("<canvas/>");
            canvas.attr("width", this.width);
            canvas.attr("height", this.height);

            var ctx = canvas[0].getContext('2d');
            ctx.drawImage(this, 0, 0, this.width, this.height);

            var imageData = ctx.getImageData(0, 0, this.width, this.height);
            effects[options.effect](imageData.data, this.width, this.height);

            ctx.putImageData(imageData, 0, 0);
            $(elem).attr("src", canvas[0].toDataURL("image/png"));
        };
        img.src = src;
    },
};

    $.fn.imageEffects = function () {

        var defaultOptions = {
            'effect' : 'grayscale',
        };
        var userOptions = arguments[0];

        return this.each(function () {

            var elemOptions = util.elementOptions(this, "imageEffects");
            var options = $.extend({}, defaultOptions, userOptions, elemOptions);

            methods.init(this, options);
        });
    };

})(jQuery);