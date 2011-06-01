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

#include <deque>
#include <functional>
#include <map>
#include <iostream>

// Lx0 headers
#include <lx0/lxengine.hpp>
#include <lx0/util/misc/util.hpp>
#include <lx0/subsystem/javascript.hpp>

using namespace lx0;

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

//===========================================================================//
/*!
 */
class Scripting : public Document::Component
{
public: 
    Scripting()
    {
        mHandlers.insert(std::make_pair("Script", [&](ElementPtr spElem) {
            
            std::string source;
            if (spElem->attr("src").isString())
                source = lx0::lx_file_to_string(spElem->attr("src").asString());
            else
                source = spElem->value().asString();

            spElem->document()->getComponent<lx0::IJavascript>("javascript")->run(source);
        }));
    }

    virtual void onAttached (DocumentPtr spDocument) 
    {
        spDocument->iterateElements([&](ElementPtr spElem) -> bool { 
            _onElementAddRemove(spElem, true); return false; 
        });
    }

protected:
    void _onElementAddRemove (ElementPtr spElem, bool bAdd)
    {
        auto it = mHandlers.find(spElem->tagName());
        if (it != mHandlers.end())
            it->second(spElem);
    }

    std::map<std::string, std::function<void (ElementPtr spElem)>> mHandlers;
};


lx0::DocumentComponent* create_scripting() 
{ 
    return new Scripting; 
}
