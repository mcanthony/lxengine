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
#include <boost/program_options.hpp>
#include <lx0/lxengine.hpp>
#include <lx0/views/canvas.hpp>

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

lx0::View::Component*       create_renderer();
lx0::UIBinding*             create_uibinding();
lx0::Controller*            create_controller(lx0::DocumentPtr spDoc);

int 
main (int argc, char** argv)
{
    using namespace lx0;

    int exitCode = -1;
    try
    {
        EnginePtr   spEngine   = Engine::acquire();
        spEngine->addViewPlugin("Canvas", [] (View* pView) { return lx0::createCanvasViewImp(); });
        
        DocumentPtr spDocument = spEngine->loadDocument("media2/appdata/sm_lxcraft/scene-000.xml");
        spDocument->addController( create_controller(spDocument) );

        ViewPtr spView = spDocument->createView("Canvas", "view", create_renderer() );
        spView->addUIBinding( create_uibinding() );


        lxvar options;
        options.insert("title", "LxCraft");
        options.insert("width", 800);
        options.insert("height", 400);
        spView->show(options);

        exitCode = spEngine->run();
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
