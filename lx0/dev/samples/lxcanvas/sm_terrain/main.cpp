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
    - Add Controller object that maps UI to Engine events
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
#include <lx0/subsystem/canvas.hpp>
#include <lx0/prototype/misc.hpp>

#include "main.hpp"
#include "terrain.hpp"
#include "viewimp.hpp"

using namespace lx0;
using namespace lx0::prototype;

lx0::prototype::Camera             gCamera;

//===========================================================================//

void PhysicsSubsystem::onElementAdded (DocumentPtr spDocument, ElementPtr spElem) 
{
    if (spElem->tagName() == "Terrain")
    {
        mElems.insert(std::make_pair(spElem.get(), spElem));
    }
}

void PhysicsSubsystem::onElementRemoved (Document*   pDocument, ElementPtr spElem) 
{
    auto it = mElems.find(spElem.get());
    if (it != mElems.end())
        mElems.erase(it);
}
    
float PhysicsSubsystem::drop (float x, float y)
{
    float maxZ = std::numeric_limits<float>::min();
    for (auto it = mElems.begin(); it != mElems.end(); ++it)
    {
        auto spTerrain = it->second->getComponent<Terrain::Runtime>("runtime");
        maxZ = std::max(maxZ, spTerrain->calcHeight(x, y));
    }
    return maxZ; 
}

void PhysicsSubsystem::onUpdate (DocumentPtr spDocument)
{
    const float terrainHeight = drop(gCamera.mPosition.x, gCamera.mPosition.y);
    const float deltaZ = (terrainHeight + 32.0f) - gCamera.mPosition.z;
    gCamera.mPosition.z += deltaZ;
    gCamera.mTarget.z += deltaZ;

    if (deltaZ > 0.001)
        spDocument->view(0)->sendEvent("redraw", lxvar::undefined());
}


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
        spEngine->addDocumentComponent("physics2", [] () { return new PhysicsSubsystem; } );
        spEngine->addDocumentComponent("rasterizer", lx0::createIRasterizer);
        spEngine->addViewPlugin("LxCanvas", [] (View* pView) { return create_lxcanvasimp(); });
        spEngine->addElementComponent("Terrain", "runtime", [](ElementPtr spElem) { return new Terrain::Runtime(spElem); }); 
        spEngine->addElementComponent("Terrain", "renderable", [](ElementPtr spElem) { return new Terrain::Render; });
        spEngine->addElementComponent("Sprite", "renderable", [](ElementPtr spElem) { return new_Sprite(); });
        spEngine->addElementComponent("SkyMap", "renderable", [](ElementPtr spElem) { return new_SkyMap(); });
        
        DocumentPtr spDocument = spEngine->loadDocument("media2/appdata/sm_terrain/scene.xml");
        ViewPtr     spView     = spDocument->createView("LxCanvas", "view");
        spView->show();

        exitCode = spEngine->run();
        spDocument->destroyView("view");
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
