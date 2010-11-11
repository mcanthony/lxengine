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
#include <lx0/lxvar.hpp>

namespace lx0 { namespace core {

    /*!
        By default all attributes in the XML document are parsed as lxson formatted 
        strings (i.e. a superset of JSON).  The Engine allows for custom parsers to
        be attached to elements which can attempt to parse the element before the 
        lxson parser is attempted.

        This method invokes the list of registered parsers and then defaults to the
        lxson parser - returning an lxvar wrapper on the raw string if all fail.
     */
    lxvar 
    Engine::parseAttribute (std::string name, std::string value)
    {
        // First attempt any registered parsers
        //
        auto it = m_attributeParsers.find(name);
        if (it != m_attributeParsers.end())
        {
            auto group = it->second;
            for (auto jt = group.begin(); jt != group.end(); ++jt)
            {
                lxvar parsed = (*jt)(value);
                if (parsed.isDefined())
                    return parsed;
            }
        }

        // Default to parsing as lxson
        //
        return lxvar::parse(value.c_str());
    }

    void
    Engine::addAttributeParser  (std::string attr, std::function<lxvar(std::string)> parser)
    {
        m_attributeParsers[attr].push_back(parser);
    }

}}
