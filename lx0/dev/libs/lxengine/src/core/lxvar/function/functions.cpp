//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

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

#include <lx0/core/lxvar/lxvar.hpp>
#include <lx0/core/log/log.hpp>
#include <boost/format.hpp>

namespace lx0 { namespace core {  namespace lxvar_ns {

     
    void 
    insert (lxvar& v, const char* path, lxvar value)
    {
        lx_error("Not yet implemented!");
    }

    /*!
        A pretty-print function that prints the lxvar as name value pairs in a tabbed
        fashion.
     */
    std::string 
    format_tabbed (detail::lxvar& v)
    {
        std::string buffer;
        std::function<void (lxvar, std::string)> fmt = [&fmt, &buffer](lxvar v, std::string indent) 
        {
            if (v.isArray())
            {
                for (auto it = v.begin(); it != v.end(); ++it)
                {
                    fmt(*it, indent);
                }
            }
            else if (v.isMap())
            {
                for (auto it = v.begin(); it != v.end(); ++it)
                {
                    buffer += boost::str( boost::format("%s%s : ") % indent % it.key() );
                    if ((*it).isArray() || (*it).isMap())
                    {
                        buffer += "\n" + indent;
                        fmt(*it, indent + "    ");
                    }
                    else
                        fmt(*it, indent);
                        
                }
            }
            else if (v.isInt())
                buffer += boost::str( boost::format("%d\n") % v.as<int>() );
            else if (v.isFloat())
                buffer += boost::str( boost::format("%f\n") % v.as<float>() );
            else if (v.isString())
                buffer += boost::str( boost::format("%s\n") % v.as<std::string>().c_str() );
            else
                buffer += "<unknown>";
        };
        fmt(v, "");

        return buffer;
    }           

}}}