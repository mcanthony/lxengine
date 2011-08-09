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

#include <sstream>

#include <lx0/core/lxvar/lxvar.hpp>
#include <lx0/core/log/log.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

namespace lx0 { namespace core {  namespace lxvar_ns {

    static lxvar
    _query_path (lxvar v, std::string path)
    {
        //
        // Split the path into components
        //
        std::vector<std::string> keys;
        boost::split(keys, path, boost::is_any_of("/"));

        //
        // Walk the path
        //
        for (auto it = keys.begin(); it != keys.end(); ++it)
        {
            if (v.is_map())
            {
                v = v.find(*it);
            }
            else if (v.is_array())
            {
                int index;
                std::istringstream iss (*it);
                iss >> index;
                if (!iss.eof())
                    v = lxvar();
                else
                    v = v.at(index);
            }
            else
            {
                v = lxvar();
                break;
            }
        }

        return v;
    }

    lxvar
    find (lxvar& v, const char* path)
    {
        return _query_path(v, path);
    }
     
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
            if (v.is_array())
            {
                bool bInline = (v.size() <= 8);
                if (bInline)
                {
                    for (auto it = v.begin(); it != v.end(); ++it)
                    {
                        if (!((*it).is_int() || (*it).is_float()))
                        {
                            bInline = false;
                            break;
                        }
                    }
                }

                if (bInline)
                {
                    buffer += "    ";
                    for (auto i = 0; i < v.size(); ++i)
                    {
                        if (v[i].is_int())
                            buffer += boost::str( boost::format("%d") % v[i].as<int>() );
                        else if (v[i].is_float())
                            buffer += boost::str( boost::format("%f") % v[i].as<float>() );
                        else if (v[i].is_string())
                            buffer += boost::str( boost::format("%s") % v[i].as<std::string>().c_str() );
                        else
                            buffer += "<unknown>";

                        if (i == v.size() -1)
                            buffer += "\n";
                        else
                            buffer += ", ";
                    }
                }
                else
                {
                    for (auto it = v.begin(); it != v.end(); ++it)
                    {
                        fmt(*it, indent);
                    }
                }
            }
            else if (v.is_map())
            {
                for (auto it = v.begin(); it != v.end(); ++it)
                {
                    buffer += boost::str( boost::format("%s%s : ") % indent % it.key() );
                    if ((*it).is_array() || (*it).is_map())
                    {
                        buffer += "\n" + indent;
                        fmt(*it, indent + "    ");
                    }
                    else
                        fmt(*it, indent);
                        
                }
            }
            else if (v.is_int())
                buffer += boost::str( boost::format("%d\n") % v.as<int>() );
            else if (v.is_float())
                buffer += boost::str( boost::format("%f\n") % v.as<float>() );
            else if (v.is_string())
                buffer += boost::str( boost::format("%s\n") % v.as<std::string>().c_str() );
            else
                buffer += "<unknown>";
        };
        fmt(v, "");

        return buffer;
    }         

    static std::string
    _escape_string (const std::string& s)
    {
        std::string t;
        t.reserve(s.size());
        for (size_t i = 0; i < s.length(); ++i)
        {
            switch (s[i])
            {
            case '\"':      t += "\\\"";    break;
            case '\\':      t += "\\\\";    break;
            case '/':       t += "\\/";     break;
            case '\b':      t += "\\b";     break;
            case '\f':      t += "\\f";     break;
            case '\n':      t += "\\n";     break;
            case '\r':      t += "\\r";     break;
            case '\t':      t += "\\t";     break;
            default:        t += s[i];      break;
            }
        }
        return t;
    }

    static void
    _format_json_imp (std::stringstream& ss, lxvar& v, std::string indent)
    {
        if (v.is_int())
            ss << indent << v.as<int>();
        else if (v.is_float())
            ss << indent << v.as<float>();
        else if (v.is_string())
            ss << indent << "\"" << _escape_string(v) << "\"";
        else if (v.is_array())
        {
            bool bInline = v.size() <= 4;

            ss << " [ ";
            if (!bInline) ss << "\n";

            int i = 0;
            for (auto it = v.begin(); it != v.end(); ++it)
            {
                _format_json_imp(ss, *it, bInline ? "" : indent + "    ");
                if (++i < v.size())
                    ss << ", ";
                if (!bInline)
                    ss << "\n";
            }
            ss << " ] ";
        }
        else if (v.is_map())
        {
            ss << "{\n";
            int i = 0;
            for (auto it = v.begin(); it != v.end(); ++it)
            {
                ss << "    " << indent << "\"" << it.key() << "\" : ";
                bool bInline = (*it).is_int() || (*it).is_float() || (*it).is_bool() || (*it).is_string();
                _format_json_imp(ss, *it, bInline ? "" : indent + "    ");
                if (++i < v.size())
                    ss << ",";
                ss << "\n";
            }
            ss << indent << "} ";
        }
    }
    

    std::string
    format_json (lxvar& v)
    {
        std::stringstream ss;
        _format_json_imp(ss, v, "");
        return ss.str();
    }

}}}
