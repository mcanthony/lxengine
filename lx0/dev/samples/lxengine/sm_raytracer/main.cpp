//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)

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
#include <glgeom/prototype/image.hpp>
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/javascript.hpp>
#include <lx0/prototype/misc.hpp>

#include "raytracer.hpp"
#include "viewer.hpp"

//===========================================================================//

glgeom::image3f img;

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
        
        spEngine->globals().add("file",     eAcceptsString, lx0::validate_filename());
        spEngine->globals().add("output",   eAcceptsString, lx0::validate_string());
        spEngine->globals().add("width",    eAcceptsInt,    lx0::validate_int_range(1, 16 * 1024), 512);
        spEngine->globals().add("height",   eAcceptsInt,    lx0::validate_int_range(1, 16 * 1024), 512);

        if (spEngine->parseCommandLine(argc, argv, "file"))
        {
            // Resize the image to the requested size
            img = glgeom::image3f( spEngine->globals().find("width"), spEngine->globals().find("height") );

            spEngine->attachComponent("Scripting", new lx0::JavascriptPlugin);
        
            DocumentPtr spDocument = spEngine->loadDocument(spEngine->globals().find("file"));
            spDocument->attachComponent("ray", create_raytracer() );

            ViewPtr spView = spDocument->createView("Canvas", "view", create_renderer() );
            spView->addUIBinding( create_uibinding() );

            lxvar options;
            options.insert("title",  "LxEngine Raytracer Sample");
            options.insert("width",  img.width());
            options.insert("height", img.height());
            spView->show(options);

            exitCode = spEngine->run();
        }
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
