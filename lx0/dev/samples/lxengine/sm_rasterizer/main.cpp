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
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/javascript.hpp>
#include "renderer.hpp"

using namespace lx0;

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        EnginePtr spEngine = Engine::acquire();
        
        spEngine->globals().add("file",      lx0::eAcceptsString,  lx0::validate_filename());
        spEngine->globals().add("output",    lx0::eAcceptsString,  lx0::validate_string());
        spEngine->globals().add("width",     lx0::eAcceptsInt,     lx0::validate_int_range(32, 4096), 512);
        spEngine->globals().add("height",    lx0::eAcceptsInt,     lx0::validate_int_range(32, 4096), 512);

        if (spEngine->parseCommandLine(argc, argv, "file"))
        {
            spEngine->attachComponent("jsengine",   lx0::createJavascriptSubsystem());
            spEngine->attachComponent("Javascript", createScriptHandler());
        
            DocumentPtr spDocument = spEngine->loadDocument(spEngine->globals().find("file"));
            spDocument->addController( create_controller(spDocument) );

            ViewPtr spView = spDocument->createView("Canvas", "view", create_renderer() );
            spView->addUIBinding( create_uibinding() );

            lxvar options;
            options.insert("title",  "LxEngine Rasterizer Sample");
            options.insert("width",  spEngine->globals().find("width"));
            options.insert("height", spEngine->globals().find("height"));
            spView->show(options);

            lxvar outputVar = spEngine->globals().find("output");
            if (outputVar.is_defined())
            {
                spView->sendEvent("screenshot", outputVar);
                spEngine->sendEvent("quit");
            }

            exitCode = spEngine->run();
        }
        spEngine->shutdown();
    }
    catch (lx0::error_exception& e)
    {
        std::cerr << "Error: " << e.details().c_str() << std::endl
                    << "Code: " << e.type() << std::endl
                    << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Fatal: unhandled std::exception.\n"
            << "Exception: " << e.what();
    }

    return exitCode;
}
