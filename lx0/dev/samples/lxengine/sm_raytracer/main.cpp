//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2012 athile@athile.net (http://www.athile.net)

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
glgeom::abbox2i imgRegion;

//===========================================================================//
//   L O C A L   F U N C T I O N S
//===========================================================================//

namespace 
{
    void
    validateConfiguration (lx0::EnginePtr spEngine)
    {
        const auto& config = spEngine->globals();

        lx_check_error( config.find("width").is_int(), "Invalid configuration for image width.  Not parseable as an integer." );
        lx_check_error( config.find("height").is_int(), "Invalid configuration for image height.  Not parseable as an integer." );

        const auto file = config.find("file");
        if (file.is_undefined())
            throw lx_error_exception("Argument 'file' undefined");
        if (!file.is_string())
            throw lx_error_exception("Argument 'file' not parseable as a string.");
    }

}


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
     
        //
        // Set up global options prior to parsing the command line
        //
        spEngine->globals().add("file",     eAcceptsString, lx0::validate_filename());
        spEngine->globals().add("output",   eAcceptsString, lx0::validate_string());
        spEngine->globals().add("width",    eAcceptsInt,    lx0::validate_int_range(1, 16 * 1024), 512);
        spEngine->globals().add("height",   eAcceptsInt,    lx0::validate_int_range(1, 16 * 1024), 512);
        spEngine->globals().add("sampler",  eAcceptsString, lx0::validate_string(), "adaptive");

        // Parse the command line (specifying "file" as the default unnamed argument)
        if (spEngine->parseCommandLine(argc, argv, "file"))
        {	
            validateConfiguration(spEngine);

            spEngine->attachComponent(lx0::createJavascriptSubsystem());
            spEngine->attachComponent(createScriptHandler());

            // Resize the image to the requested size
            img = glgeom::image3f( spEngine->globals().find("width"), spEngine->globals().find("height") );

            DocumentPtr spDocument = spEngine->loadDocument(spEngine->globals().find("file"));
            spDocument->attachComponent(create_raytracer() );

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
        std::cout << "Error: " << e.what() << std::endl
                    << std::endl;
    }
    catch (std::exception& e)
    {
        throw lx_fatal_exception("Fatal: unhandled std::exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
