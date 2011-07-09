//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

#include <lx0/lxengine.hpp>
#include <lx0/elements/core.hpp>


using namespace lx0;

namespace lec = lx0::elements::core_ns;

void lec::processIncludeDocument (DocumentPtr spDocument)
{
    EnginePtr spEngine = Engine::acquire();

    auto includes = spDocument->getElementsByTagName("IncludeDocument");
    for (auto it = includes.begin(); it != includes.end(); ++it)
    {
        std::string filename = (*it)->attr("src").as<std::string>();
        auto spParent = (*it)->parent();

        auto spDoc2 = spEngine->loadDocument(filename);
        spDoc2->iterateElements([&](ElementPtr spElem) -> bool {
            spParent->append( spElem->cloneDeep() );
            return false;
        });
        spEngine->closeDocument(spDoc2);
    }
}
