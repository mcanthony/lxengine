//
// Based on the spec at: http://www.w3.org/TR/2010/PR-css3-color-20101028/
// Note: this parsing code intentionally does *not* deal with 
// alpha/opacity/transparency as LxCSS differs from CSS in that area.
//

(function() {
    
    function _col(r, g, b)
    {
        return [ (1.0 * r) / 255.0, 
                 (1.0 * g) / 255.0, 
                 (1.0 * b) / 255.0 ];
    }

    function basic_keywords(s)
    {
        switch (s.toLowerCase())
        {
            case "black": return [0/255.0,0/255.0,0/255.0];
            case "silver": return [192/255.0,192/255.0,192/255.0];
            case "gray": return [128/255.0,128/255.0,128/255.0];
            case "white": return [255/255.0,255/255.0,255/255.0];
            case "maroon": return [128/255.0,0/255.0,0/255.0];
            case "red": return [255/255.0,0/255.0,0/255.0];
            case "purple": return [128/255.0,0/255.0,128/255.0];
            case "fuchsia": return [255/255.0,0/255.0,255/255.0];
            case "green": return [0/255.0,128/255.0,0/255.0];
            case "lime": return [0/255.0,255/255.0,0/255.0];
            case "olive": return [128/255.0,128/255.0,0/255.0];
            case "yellow": return [255/255.0,255/255.0,0/255.0];
            case "navy": return [0/255.0,0/255.0,128/255.0];
            case "blue": return [0/255.0,0/255.0,255/255.0];
            case "teal": return [0/255.0,128/255.0,128/255.0];
            case "aqua": return [0/255.0,255/255.0,255/255.0];
            default:     return undefined;
        }
    }
    
    function hex_notation(s)
    {
        function htoi (a)
        {
            switch (a.toLowerCase())
            {
            case "0" : return 0;
            case "1" : return 1;
            case "2" : return 2;
            case "3" : return 3;
            case "4" : return 4;
            case "5" : return 5;
            case "6" : return 6;
            case "7" : return 7;
            case "8" : return 8;
            case "9" : return 9;
            case "a" : return 10;
            case "b" : return 11;
            case "c" : return 12;
            case "d" : return 13;
            case "e" : return 14;
            case "f" : return 15;
            }
        }
        
        if (/^#[A-Fa-f0-9]{3}$/.test(s))
        {
            // Per http://www.w3.org/TR/2010/PR-css3-color-20101028/#numerical, the hex digits
            // are replicated, not set to zero.
            //
            var i0 = htoi(s[1]);
            var i1 = htoi(s[2]);
            var i2 = htoi(s[3]);
            return _col(
                i0 * 16 + i0, 
                i1 * 16 + i1, 
                i2 * 16 + i2
            );
        }
        else if (/^#[A-Fa-f0-9]{6}$/.test(s))
        {
            return _col(
                htoi(s[1]) * 16 + htoi(s[2]), 
                htoi(s[3]) * 16 + htoi(s[4]), 
                htoi(s[5]) * 16 + htoi(s[6])
            );
        }
    }
    
    function rgb_notation (s)
    {
        var rgbStart = "^rgb\\s*\\(";
        var rgbNumber = "\\s*(\\d{1,3})\\s*";
        var rgbPercent = "\\s*(\\d{1,3})\\%\\s*";
        var rgbEnd = "\\)\\s*$";
        
        var re = new RegExp(rgbStart + rgbNumber + "," + rgbNumber + "," + rgbNumber + rgbEnd);
        var match = re.exec(s); 
        if (match)
        {
            var r = parseInt(match[1]);
            var g = parseInt(match[2]);
            var b = parseInt(match[3]);
            
            if (   r >= 0 && r <= 255
                && g >= 0 && g <= 255
                && b >= 0 && b <= 255)
            {
                return _col(r, g, b);
            }
        }

        re = new RegExp(rgbStart + rgbPercent + "," + rgbPercent + "," + rgbPercent + rgbEnd);
        match = re.exec(s); 
        if (match)
        {
            var r = parseInt(match[1]) * 255 / 100;
            var g = parseInt(match[2]) * 255 / 100;
            var b = parseInt(match[3]) * 255 / 100;
            
            if (   r >= 0 && r <= 255
                && g >= 0 && g <= 255
                && b >= 0 && b <= 255)
            {
                return _col(r, g, b);
            }
        }
    }

    // The engine will try all parsers for an attribute, in-order, until one
    // returns a defined value.
    //
    engine.addAttributeParser("color", basic_keywords);
    engine.addAttributeParser("color", hex_notation);
    engine.addAttributeParser("color", rgb_notation);
    // todo hsl notation
    // todo extended keywords
})();
