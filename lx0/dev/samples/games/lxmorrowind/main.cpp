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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

#include <iostream>
#include <algorithm>
#include <lx0/lxengine.hpp>

#include <fstream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "tes3io/tes3io.hpp"


lx0::View::Component*   create_renderer();

//===========================================================================//


//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    using namespace lx0;

    int exitCode = -1;
    try
    {
        EnginePtr   spEngine   = Engine::acquire();
        
        DocumentPtr spDocument = spEngine->createDocument();
        Tes3Io loader;
        loader.initialize("mwdata");

        scene_group group;
        loader.cell("Beshara", group);

        
        ViewPtr spView = spDocument->createView("Canvas", "view", create_renderer() );

        lxvar options;
        options.insert("title", "LxMorrowind");
        options.insert("width", 800);
        options.insert("height", 400);
        spView->show(options);

        spEngine->sendEvent("quit");
        exitCode = spEngine->run();

        spView.reset();
        spEngine->closeDocument(spDocument);
        spEngine->shutdown();
    }
    catch (lx0::error_exception& e)
    {
        std::cout << "Error: " << e.details().c_str() << std::endl
                    << "Code: " << e.type() << std::endl
                    << std::endl;
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled std::exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
