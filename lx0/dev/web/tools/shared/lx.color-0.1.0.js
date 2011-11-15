
try { eval("lx"); } catch (e) { lx = {}; }
if (!lx.color) lx.color = {};

(function (NS) {

    var lib = {};
    
    lib._clamp = function(i, n, m) {
        return Math.max(Math.min(i, m), n);
    };
    lib._trim = function(s) {
        return s.replace(/^\s+|\s+$/g,"")    
    };
    lib._hex_to_int = function(hex) {
        return parseInt(hex, 16);
    };


        /**
        * Converts RGB to HSV value.
        *
        * @param {Integer} r Red value, 0-255
        * @param {Integer} g Green value, 0-255
        * @param {Integer} b Blue value, 0-255
        * @returns {Array} The HSV values EG: [h,s,v], [0-360 degrees, 0-100%, 0-100%]
        */
        lib.rgb_to_hsv = function(r, g, b) {

          var r = (r / 255),
         g = (g / 255),
  	 b = (b / 255);

          var min = Math.min(Math.min(r, g), b),
        max = Math.max(Math.max(r, g), b),
        delta = max - min;

          var value = max,
        saturation,
        hue;

          // Hue
          if (max == min) {
            hue = 0;
          } else if (max == r) {
            hue = (60 * ((g - b) / (max - min))) % 360;
          } else if (max == g) {
            hue = 60 * ((b - r) / (max - min)) + 120;
          } else if (max == b) {
            hue = 60 * ((r - g) / (max - min)) + 240;
          }

          if (hue < 0) {
            hue += 360;
          }

          // Saturation
          if (max == 0) {
            saturation = 0;
          } else {
            saturation = 1 - (min / max);
          }

          return [Math.round(hue), Math.round(saturation * 100), Math.round(value * 100)];
        };

    /*
    * @credit http://matthaynes.net/blog/2008/08/07/javascript-colour-functions/
    */
    lib.rgb_to_hex = function (r,g,b) {
          function i2h(i) {
            var hex = parseInt(i).toString(16).toUpperCase();
            return (hex.length < 2) ? "0" + hex : hex;
          }
          return "#" + i2h(r) + i2h(g) + i2h(b);
        };

        /**
        * Converts HSV to RGB value.
        *
        * @param {Integer} h Hue as a value between 0 - 360 degrees
        * @param {Integer} s Saturation as a value between 0 - 100 %
        * @param {Integer} v Value as a value between 0 - 100 %
        * @returns {Array} The RGB values  EG: [r,g,b], [255,255,255]
        * 
        * @credit http://matthaynes.net/blog/2008/08/07/javascript-colour-functions/
        */
      lib.hsv_to_rgb = function (h, s, v) {

          var s = s / 100,
          v = v / 100;

          var hi = Math.floor((h / 60) % 6);
          var f = (h / 60) - hi;
          var p = v * (1 - s);
          var q = v * (1 - f * s);
          var t = v * (1 - (1 - f) * s);

          var rgb = [];

          switch (hi) {
            case 0: rgb = [v, t, p]; break;
            case 1: rgb = [q, v, p]; break;
            case 2: rgb = [p, v, t]; break;
            case 3: rgb = [p, q, v]; break;
            case 4: rgb = [t, p, v]; break;
            case 5: rgb = [v, p, q]; break;
          }

          var r = Math.min(255, Math.round(rgb[0] * 256)),
          g = Math.min(255, Math.round(rgb[1] * 256)),
          b = Math.min(255, Math.round(rgb[2] * 256));
          return [r, g, b];
        };
  
    var _RGBA = function(r,g,b,a) { 
        this.r = r;
        this.g = g;
        this.b = b;
        this.a = a || 255;
    };
    _RGBA.prototype.r = 0;
    _RGBA.prototype.g = 0;
    _RGBA.prototype.b = 0;
    _RGBA.prototype.a = 255;
    _RGBA.prototype.html = function(type) {
      this.r = lib._clamp(this.r, 0, 255);
      this.g = lib._clamp(this.g, 0, 255);
      this.b = lib._clamp(this.b, 0, 255);
      return lib.rgb_to_hex(this.r, this.g, this.b);  
    };

    lib.parse_hex7 = function(color) {
        return new _RGBA(
            lib._hex_to_int(color.substr(1,2)),
            lib._hex_to_int(color.substr(3,2)),
            lib._hex_to_int(color.substr(5,2))
        );
    };

    lib._named_colors = 
        {
          aliceblue : lib.parse_hex7("#F0F8FF"),
          antiquewhite : lib.parse_hex7("#FAEBD7"),
          aqua : lib.parse_hex7("#00FFFF"),
          aquamarine : lib.parse_hex7("#7FFFD4"),
          azure : lib.parse_hex7("#F0FFFF"),
          beige : lib.parse_hex7("#F5F5DC"),
          bisque : lib.parse_hex7("#FFE4C4"),
          black : lib.parse_hex7("#000000"),
          blanchedalmond : lib.parse_hex7("#FFEBCD"),
          blue : lib.parse_hex7("#0000FF"),
          blueviolet : lib.parse_hex7("#8A2BE2"),
          brown : lib.parse_hex7("#A52A2A"),
          burlywood : lib.parse_hex7("#DEB887"),
          cadetblue : lib.parse_hex7("#5F9EA0"),
          chartreuse : lib.parse_hex7("#7FFF00"),
          chocolate : lib.parse_hex7("#D2691E"),
          coral : lib.parse_hex7("#FF7F50"),
          cornflowerblue : lib.parse_hex7("#6495ED"),
          cornsilk : lib.parse_hex7("#FFF8DC"),
          crimson : lib.parse_hex7("#DC143C"),
          cyan : lib.parse_hex7("#00FFFF"),
          darkblue : lib.parse_hex7("#00008B"),
          darkcyan : lib.parse_hex7("#008B8B"),
          darkgoldenrod : lib.parse_hex7("#B8860B"),
          darkgray : lib.parse_hex7("#A9A9A9"),
          darkgrey : lib.parse_hex7("#A9A9A9"),
          darkgreen : lib.parse_hex7("#006400"),
          darkkhaki : lib.parse_hex7("#BDB76B"),
          darkmagenta : lib.parse_hex7("#8B008B"),
          darkolivegreen : lib.parse_hex7("#556B2F"),
          darkorange : lib.parse_hex7("#FF8C00"),
          darkorchid : lib.parse_hex7("#9932CC"),
          darkred : lib.parse_hex7("#8B0000"),
          darksalmon : lib.parse_hex7("#E9967A"),
          darkseagreen : lib.parse_hex7("#8FBC8F"),
          darkslateblue : lib.parse_hex7("#483D8B"),
          darkslategray : lib.parse_hex7("#2F4F4F"),
          darkslategrey : lib.parse_hex7("#2F4F4F"),
          darkturquoise : lib.parse_hex7("#00CED1"),
          darkviolet : lib.parse_hex7("#9400D3"),
          deeppink : lib.parse_hex7("#FF1493"),
          deepskyblue : lib.parse_hex7("#00BFFF"),
          dimgray : lib.parse_hex7("#696969"),
          dimgrey : lib.parse_hex7("#696969"),
          dodgerblue : lib.parse_hex7("#1E90FF"),
          firebrick : lib.parse_hex7("#B22222"),
          floralwhite : lib.parse_hex7("#FFFAF0"),
          forestgreen : lib.parse_hex7("#228B22"),
          fuchsia : lib.parse_hex7("#FF00FF"),
          gainsboro : lib.parse_hex7("#DCDCDC"),
          ghostwhite : lib.parse_hex7("#F8F8FF"),
          gold : lib.parse_hex7("#FFD700"),
          goldenrod : lib.parse_hex7("#DAA520"),
          gray : lib.parse_hex7("#808080"),
          grey : lib.parse_hex7("#808080"),
          green : lib.parse_hex7("#008000"),
          greenyellow : lib.parse_hex7("#ADFF2F"),
          honeydew : lib.parse_hex7("#F0FFF0"),
          hotpink : lib.parse_hex7("#FF69B4"),
          indianred  : lib.parse_hex7("#CD5C5C"),
          indigo  : lib.parse_hex7("#4B0082"),
          ivory : lib.parse_hex7("#FFFFF0"),
          khaki : lib.parse_hex7("#F0E68C"),
          lavender : lib.parse_hex7("#E6E6FA"),
          lavenderblush : lib.parse_hex7("#FFF0F5"),
          lawngreen : lib.parse_hex7("#7CFC00"),
          lemonchiffon : lib.parse_hex7("#FFFACD"),
          lightblue : lib.parse_hex7("#ADD8E6"),
          lightcoral : lib.parse_hex7("#F08080"),
          lightcyan : lib.parse_hex7("#E0FFFF"),
          lightgoldenrodyellow : lib.parse_hex7("#FAFAD2"),
          lightgray : lib.parse_hex7("#D3D3D3"),
          lightgrey : lib.parse_hex7("#D3D3D3"),
          lightgreen : lib.parse_hex7("#90EE90"),
          lightpink : lib.parse_hex7("#FFB6C1"),
          lightsalmon : lib.parse_hex7("#FFA07A"),
          lightseagreen : lib.parse_hex7("#20B2AA"),
          lightskyblue : lib.parse_hex7("#87CEFA"),
          lightslategray : lib.parse_hex7("#778899"),
          lightslategrey : lib.parse_hex7("#778899"),
          lightsteelblue : lib.parse_hex7("#B0C4DE"),
          lightyellow : lib.parse_hex7("#FFFFE0"),
          lime : lib.parse_hex7("#00FF00"),
          limegreen : lib.parse_hex7("#32CD32"),
          linen : lib.parse_hex7("#FAF0E6"),
          magenta : lib.parse_hex7("#FF00FF"),
          maroon : lib.parse_hex7("#800000"),
          mediumaquamarine : lib.parse_hex7("#66CDAA"),
          mediumblue : lib.parse_hex7("#0000CD"),
          mediumorchid : lib.parse_hex7("#BA55D3"),
          mediumpurple : lib.parse_hex7("#9370D8"),
          mediumseagreen : lib.parse_hex7("#3CB371"),
          mediumslateblue : lib.parse_hex7("#7B68EE"),
          mediumspringgreen : lib.parse_hex7("#00FA9A"),
          mediumturquoise : lib.parse_hex7("#48D1CC"),
          mediumvioletred : lib.parse_hex7("#C71585"),
          midnightblue : lib.parse_hex7("#191970"),
          mintcream : lib.parse_hex7("#F5FFFA"),
          mistyrose : lib.parse_hex7("#FFE4E1"),
          moccasin : lib.parse_hex7("#FFE4B5"),
          navajowhite : lib.parse_hex7("#FFDEAD"),
          navy : lib.parse_hex7("#000080"),
          oldlace : lib.parse_hex7("#FDF5E6"),
          olive : lib.parse_hex7("#808000"),
          olivedrab : lib.parse_hex7("#6B8E23"),
          orange : lib.parse_hex7("#FFA500"),
          orangered : lib.parse_hex7("#FF4500"),
          orchid : lib.parse_hex7("#DA70D6"),
          palegoldenrod : lib.parse_hex7("#EEE8AA"),
          palegreen : lib.parse_hex7("#98FB98"),
          paleturquoise : lib.parse_hex7("#AFEEEE"),
          palevioletred : lib.parse_hex7("#D87093"),
          papayawhip : lib.parse_hex7("#FFEFD5"),
          peachpuff : lib.parse_hex7("#FFDAB9"),
          peru : lib.parse_hex7("#CD853F"),
          pink : lib.parse_hex7("#FFC0CB"),
          plum : lib.parse_hex7("#DDA0DD"),
          powderblue : lib.parse_hex7("#B0E0E6"),
          purple : lib.parse_hex7("#800080"),
          red : lib.parse_hex7("#FF0000"),
          rosybrown : lib.parse_hex7("#BC8F8F"),
          royalblue : lib.parse_hex7("#4169E1"),
          saddlebrown : lib.parse_hex7("#8B4513"),
          salmon : lib.parse_hex7("#FA8072"),
          sandybrown : lib.parse_hex7("#F4A460"),
          seagreen : lib.parse_hex7("#2E8B57"),
          seashell : lib.parse_hex7("#FFF5EE"),
          sienna : lib.parse_hex7("#A0522D"),
          silver : lib.parse_hex7("#C0C0C0"),
          skyblue : lib.parse_hex7("#87CEEB"),
          slateblue : lib.parse_hex7("#6A5ACD"),
          slategray : lib.parse_hex7("#708090"),
          slategrey : lib.parse_hex7("#708090"),
          snow : lib.parse_hex7("#FFFAFA"),
          springgreen : lib.parse_hex7("#00FF7F"),
          steelblue : lib.parse_hex7("#4682B4"),
          tan : lib.parse_hex7("#D2B48C"),
          teal : lib.parse_hex7("#008080"),
          thistle : lib.parse_hex7("#D8BFD8"),
          tomato : lib.parse_hex7("#FF6347"),
          turquoise : lib.parse_hex7("#40E0D0"),
          violet : lib.parse_hex7("#EE82EE"),
          wheat : lib.parse_hex7("#F5DEB3"),
          white : lib.parse_hex7("#FFFFFF"),
          whitesmoke : lib.parse_hex7("#F5F5F5"),
          yellow : lib.parse_hex7("#FFFF00"),
          yellowgreen : lib.parse_hex7("#9ACD32"),
        };

    lib._named_colors_keys = undefined;
    lib.named_colors = function() {
        if (!lib._named_colors_keys) 
        {
            lib._named_colors_keys = [];
            $.each(lib._named_colors, function (k, v) { lib._named_colors_keys.push(k); });
        }
        return lib._named_colors_keys;
    };

    // Formats
    var _HSVA = function(h,s,v,a) 
    {
        this.h = h;
        this.s = s;
        this.v = v;
        this.a = a;
    };
    _HSVA.prototype.h = 0.0;     // 0-360 (normalized)
    _HSVA.prototype.s = 0.0;     // 0-100 (normalized)
    _HSVA.prototype.v = 0.0;
    _HSVA.prototype.a = 0.0;
    _HSVA.prototype.html = function() 
    {
        var rgb = lib.hsv_to_rgb(this.h, this.s, this.v);
        var rgba = new _RGBA(rgb[0], rgb[1], rgb[2], this.a);
        return rgba.html(arguments);
    };
    _RGBA.prototype.convert = function(type)
    {
        if (type == 'HSVA')
        {
            var arr = lib.rgb_to_hsv(this.r, this.g, this.b);
            return new _HSVA(arr[0], arr[1], arr[2], this.a);
        }
    }

    function delegate_to_hsv(method, args)
    {
        return function() {
            var obj = this.convert('HSVA');
            return obj[method].apply(obj, arguments);
        };
    }
    $.each(['hue', 'saturation','value'], function(i, value) {
        _RGBA.prototype[value] = delegate_to_hsv(value);
    });
    
    _HSVA.prototype.hue = function(d)
    {
        if (typeof(d) == "number")
        {
            this.h = (this.h + d) % 360;
            if (this.h < 0) this.h += 360;
        }
        return this;
    }
    _HSVA.prototype.saturation = function(d)
    {
        if (typeof(d) == "number")
            this.s = lib._clamp(this.s + d, 0, 100);
        return this;
    }
    _HSVA.prototype.value = function(d)
    {
        if (typeof(d) == "number")
            this.v = lib._clamp(this.v + d, 0, 100);
        return this;
    }


    NS.parse = function (color) {
        if (arguments.length == 1)
        {
            if (typeof(color) == "string") {
                color = lib._trim(color);
                if (color.substr(0,1) == "#") {
                    if (color.length == 7)
                        return lib.parse_hex7(color);
                    else if (color.length == 4)
                        return undefined;
                }
            
                var named = lib._named_colors[color];
                if (named)
                    return named;
            }
        }
        else if (arguments[0] == 'hsv' && arguments.length == 4)
        {
            return new _HSVA(arguments[1], arguments[2], arguments[3], 255);
        }
        return undefined;
    };

    NS.lib = lib;

})(lx.color);

