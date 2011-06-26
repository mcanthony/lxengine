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
//  T O D O   L I S T
//===========================================================================//
/*!
    - Clean-up code
    - Reduce dependencies
 */

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

// Standard headers
#include <iostream>
#include <limits>

// Library headers
#include <boost/program_options.hpp>

// Lx0 headers
#include <glgeom/glgeom.hpp>

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/rasterizer.hpp>
#include <lx0/prototype/misc.hpp>
#include <lx0/views/canvas.hpp>

#include "main.hpp"
#include "terrain.hpp"
#include "viewimp.hpp"
#include "physics.hpp"

using namespace lx0;

//===========================================================================//

ViewImp* create_lxcanvasimp();
Element::Component* new_Sprite();
Element::Component* new_SkyMap();

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        EnginePtr   spEngine   = Engine::acquire();
        spEngine->addViewPlugin("Canvas", [] (View* pView) { return lx0::createCanvasViewImp(); });

        spEngine->addDocumentComponent("physics2", [] () { return new PhysicsSubsystem; } );

        spEngine->addElementComponent("Terrain", "runtime", [](ElementPtr spElem) { return new Terrain::Runtime(spElem); }); 
        spEngine->addElementComponent("Terrain", "renderable", [](ElementPtr spElem) { return new Terrain::Render; });
        spEngine->addElementComponent("Sprite", "renderable", [](ElementPtr spElem) { return new_Sprite(); });
        spEngine->addElementComponent("SkyMap", "renderable", [](ElementPtr spElem) { return new_SkyMap(); });
        
        DocumentPtr spDocument = spEngine->loadDocument("media2/appdata/sm_terrain/scene.xml");
        ViewPtr     spView     = spDocument->createView("Canvas", "view", new Renderer(spDocument));
        spView->addUIBinding( create_camera_controller() );
        spView->addController( create_event_controller() );
        
        lxvar options;
        options.insert("title", "Terrain Sample (OpenGL 3.2)");
        options.insert("width", 800);
        options.insert("height", 400);
        spView->show(options);

        exitCode = spEngine->run();
        spDocument->destroyView("view");
        spEngine->shutdown();
    }
    catch (lx0::error_exception& e)
    {
        std::cout << "Lx Error: " << e.details() << std::endl;
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
