(function ($) {
    
    function canvas_context_normalized_2d(canvas, callback) {
        var context = canvas.getContext('2d');
        context.save();
        context.scale(canvas.width / 2, canvas.height / 2);
        context.translate(1, 1);
        callback(context);
        context.restore();
    }

    var methods = 
    {
        draw : function (elem, options) {
            canvas_context_normalized_2d(elem, function(ctx) {
                var hue = parseFloat(options.hue);
                var offset = parseFloat(options.offset) * Math.PI / 180.0;
                var arc = options.arcRadius * Math.PI / 180.0;
                var step = 2 * Math.PI / options.count;

                for (var i = 0; i < Math.PI * 2; i += step) {
                  var angle = offset + (2 * Math.PI -  i);
                  ctx.fillStyle = lx.color.parse('hsv', (i * 180.0 / Math.PI + hue) % 360, options.saturation, options.value).html('rgba');
                  ctx.beginPath();
                  ctx.moveTo(0, 0);
                  ctx.arc(0, 0, 1, angle - arc / 1.95, angle + arc / 1.95);
                  ctx.lineTo(0, 0);
                  ctx.fill();
                  ctx.closePath();
                }

                ctx.fillStyle = "white";
                ctx.beginPath();
                ctx.arc(0, 0, options.innerRadius, 0, 2 * Math.PI);
                ctx.fill();
                ctx.closePath();
            });
          }
    };

    $.fn.colorWheel = function (options) {

        var settings = $.extend({
            'hue': 0,
            'saturation': 99,
            'value' : 99,
            'count' : 360,
            'offset' : 0,
            'arcRadius' : 2,
            'innerRadius' : .6,

            //stops
        }, options);
        
        return this.each(function () {
            
            var options = settings;
            
            var params = $(this).attr("data-colorWheel"); 
            if (params)
            {
                try
                {
                    var params = lx.color.lib._trim(params);
                    if (params[0] != "{")
                        params = "{ " + params + " }";
                    eval("params = " + params + ";");
                    options = $.extend({}, options, params);
                } catch (e)
                {
                }
            }
            
            var elem = this;
            window.setTimeout(function() { methods.draw(elem, options); }, 1);
        });

    };
})(jQuery);