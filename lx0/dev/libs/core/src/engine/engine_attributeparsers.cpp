//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010 athile@athile.net (http://www.athile.net)

    Permission is hereby granted, free of charge, to any person obtaining a 
    copy of this software and associated documentation files (the "Software"), 
    to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, and to permit persons to whom the 
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
    IN THE SOFTWARE.
*/
//===========================================================================//

//===========================================================================//
//   H E A D E R S
//===========================================================================//

#include <iostream>
#include <string>
#include <algorithm>

#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/lxvar.hpp>
#include <lx0/util.hpp>

using namespace lx0::core;

namespace {

    class ColorNamed : public lx0::core::detail::AttributeParser
    {
    public:
        ColorNamed()
        {
            _add("AliceBlue", "#F0F8FF");
            _add("AntiqueWhite", "#FAEBD7");
            _add("Aqua", "#00FFFF");
            _add("Aquamarine", "#7FFFD4");
            _add("Azure", "#F0FFFF");
            _add("Beige", "#F5F5DC");
            _add("Bisque", "#FFE4C4");
            _add("Black", "#000000");
            _add("BlanchedAlmond", "#FFEBCD");
            _add("Blue", "#0000FF");
            _add("BlueViolet", "#8A2BE2");
            _add("Brown", "#A52A2A");
            _add("BurlyWood", "#DEB887");
            _add("CadetBlue", "#5F9EA0");
            _add("Chartreuse", "#7FFF00");
            _add("Chocolate", "#D2691E");
            _add("Coral", "#FF7F50");
            _add("CornflowerBlue", "#6495ED");
            _add("Cornsilk", "#FFF8DC");
            _add("Crimson", "#DC143C");
            _add("Cyan", "#00FFFF");
            _add("DarkBlue", "#00008B");
            _add("DarkCyan", "#008B8B");
            _add("DarkGoldenRod", "#B8860B");
            _add("DarkGray", "#A9A9A9");
            _add("DarkGreen", "#006400");
            _add("DarkKhaki", "#BDB76B");
            _add("DarkMagenta", "#8B008B");
            _add("DarkOliveGreen", "#556B2F");
            _add("Darkorange", "#FF8C00");
            _add("DarkOrchid", "#9932CC");
            _add("DarkRed", "#8B0000");
            _add("DarkSalmon", "#E9967A");
            _add("DarkSeaGreen", "#8FBC8F");
            _add("DarkSlateBlue", "#483D8B");
            _add("DarkSlateGray", "#2F4F4F");
            _add("DarkTurquoise", "#00CED1");
            _add("DarkViolet", "#9400D3");
            _add("DeepPink", "#FF1493");
            _add("DeepSkyBlue", "#00BFFF");
            _add("DimGray", "#696969");
            _add("DodgerBlue", "#1E90FF");
            _add("FireBrick", "#B22222");
            _add("FloralWhite", "#FFFAF0");
            _add("ForestGreen", "#228B22");
            _add("Fuchsia", "#FF00FF");
            _add("Gainsboro", "#DCDCDC");
            _add("GhostWhite", "#F8F8FF");
            _add("Gold", "#FFD700");
            _add("GoldenRod", "#DAA520");
            _add("Gray", "#808080");
            _add("Green", "#008000");
            _add("GreenYellow", "#ADFF2F");
            _add("HoneyDew", "#F0FFF0");
            _add("HotPink", "#FF69B4");
            _add("IndianRed", "#CD5C5C");
            _add("Indigo", "#4B0082");
            _add("Ivory", "#FFFFF0");
            _add("Khaki", "#F0E68C");
            _add("Lavender", "#E6E6FA");
            _add("LavenderBlush", "#FFF0F5");
            _add("LawnGreen", "#7CFC00");
            _add("LemonChiffon", "#FFFACD");
            _add("LightBlue", "#ADD8E6");
            _add("LightCoral", "#F08080");
            _add("LightCyan", "#E0FFFF");
            _add("LightGoldenRodYellow", "#FAFAD2");
            _add("LightGrey", "#D3D3D3");
            _add("LightGreen", "#90EE90");
            _add("LightPink", "#FFB6C1");
            _add("LightSalmon", "#FFA07A");
            _add("LightSeaGreen", "#20B2AA");
            _add("LightSkyBlue", "#87CEFA");
            _add("LightSlateGray", "#778899");
            _add("LightSteelBlue", "#B0C4DE");
            _add("LightYellow", "#FFFFE0");
            _add("Lime", "#00FF00");
            _add("LimeGreen", "#32CD32");
            _add("Linen", "#FAF0E6");
            _add("Magenta", "#FF00FF");
            _add("Maroon", "#800000");
            _add("MediumAquaMarine", "#66CDAA");
            _add("MediumBlue", "#0000CD");
            _add("MediumOrchid", "#BA55D3");
            _add("MediumPurple", "#9370D8");
            _add("MediumSeaGreen", "#3CB371");
            _add("MediumSlateBlue", "#7B68EE");
            _add("MediumSpringGreen", "#00FA9A");
            _add("MediumTurquoise", "#48D1CC");
            _add("MediumVioletRed", "#C71585");
            _add("MidnightBlue", "#191970");
            _add("MintCream", "#F5FFFA");
            _add("MistyRose", "#FFE4E1");
            _add("Moccasin", "#FFE4B5");
            _add("NavajoWhite", "#FFDEAD");
            _add("Navy", "#000080");
            _add("OldLace", "#FDF5E6");
            _add("Olive", "#808000");
            _add("OliveDrab", "#6B8E23");
            _add("Orange", "#FFA500");
            _add("OrangeRed", "#FF4500");
            _add("Orchid", "#DA70D6");
            _add("PaleGoldenRod", "#EEE8AA");
            _add("PaleGreen", "#98FB98");
            _add("PaleTurquoise", "#AFEEEE");
            _add("PaleVioletRed", "#D87093");
            _add("PapayaWhip", "#FFEFD5");
            _add("PeachPuff", "#FFDAB9");
            _add("Peru", "#CD853F");
            _add("Pink", "#FFC0CB");
            _add("Plum", "#DDA0DD");
            _add("PowderBlue", "#B0E0E6");
            _add("Purple", "#800080");
            _add("Red", "#FF0000");
            _add("RosyBrown", "#BC8F8F");
            _add("RoyalBlue", "#4169E1");
            _add("SaddleBrown", "#8B4513");
            _add("Salmon", "#FA8072");
            _add("SandyBrown", "#F4A460");
            _add("SeaGreen", "#2E8B57");
            _add("SeaShell", "#FFF5EE");
            _add("Sienna", "#A0522D");
            _add("Silver", "#C0C0C0");
            _add("SkyBlue", "#87CEEB");
            _add("SlateBlue", "#6A5ACD");
            _add("SlateGray", "#708090");
            _add("Snow", "#FFFAFA");
            _add("SpringGreen", "#00FF7F");
            _add("SteelBlue", "#4682B4");
            _add("Tan", "#D2B48C");
            _add("Teal", "#008080");
            _add("Thistle", "#D8BFD8");
            _add("Tomato", "#FF6347");
            _add("Turquoise", "#40E0D0");
            _add("Violet", "#EE82EE");
            _add("Wheat", "#F5DEB3");
            _add("White", "#FFFFFF");
            _add("WhiteSmoke", "#F5F5F5");
            _add("Yellow", "#FFFF00");
            _add("YellowGreen", "#9ACD32");
        }
        
        virtual lxvar parse (std::string name)
        {
            auto it = mMap.find(name);
            if (it != mMap.end())
                return it->second;
            else
                return lxvar();
        }

    protected:
        int    _hexToInt (char c)
        {
            switch (c)
            {
            default:
                lx_error("Illegal hex character");
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case 'A': return 10;
            case 'B': return 11;
            case 'C': return 12;
            case 'D': return 13;
            case 'E': return 14;
            case 'F': return 15;
            }
        }

        void _add (const char* name, const char* hex)
        {
            float r = float(_hexToInt(hex[1]) * 16 + _hexToInt(hex[2])) / 255.0f;
            float g = float(_hexToInt(hex[3]) * 16 + _hexToInt(hex[4])) / 255.0f;
            float b = float(_hexToInt(hex[5]) * 16 + _hexToInt(hex[6])) / 255.0f;

            mMap.insert( std::make_pair(name, lxvar(r, g, b)) );
        }

        std::map<std::string, lxvar> mMap;
    };
}

namespace lx0 { namespace core {

    using namespace detail;

    lxvar 
    Engine::parseAttribute (std::string name, std::string value)
    {
        // First attempt any registered parsers
        //
        auto it = m_attributeParsers.find(name);
        if (it != m_attributeParsers.end())
        {
            for (auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            {
                lxvar parsed = (*jt)->parse(value);
                if (parsed.isDefined())
                    return parsed;
            }
        }

        // Default to parsing as lxson
        //
        return lxvar::parse(value.c_str());
    }

    void
    Engine::_attachAttributeParsers (void)
    {
        m_attributeParsers["color"].push_back( AttributeParserPtr(new ColorNamed) );
    }

}}
