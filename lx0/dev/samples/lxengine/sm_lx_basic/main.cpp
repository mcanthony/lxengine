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

#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/space.hpp>
#include <lx0/element.hpp>
#include <lx0/object.hpp>
#include <lx0/mesh.hpp>
#include <lx0/view.hpp>
#include <lx0/controller.hpp>
#include <lx0/transaction.hpp>
#include <lx0/point3.hpp>
#include <lx0/jsonio.hpp>
#include <lx0/util.hpp>

using namespace lx0::core;

int 
main (int argc, char** argv)
{
    int exitCode = -1;

    try
    {
        EnginePtr spEngine( Engine::acquire() );

        DocumentPtr spDocument = spEngine->loadDocument("scene_000.xml");

        {
            ViewPtr spView(new View);
            spDocument->connect("view", spView);
            spView->show();
        }

        ControllerPtr spController;
   
        exitCode = spEngine->run();

        // Demostrating that views *can* be detached by name.  This is not
        // necessary, as the Document releases all views on destruction 
        // automatically
        spDocument->disconnect("view");

        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
