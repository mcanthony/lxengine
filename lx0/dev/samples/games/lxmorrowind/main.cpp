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

#include <iostream>
#include <fstream>
#include <algorithm>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/physics.hpp>
#include "tes3loader.hpp"
#include "physics/mwphysics.hpp"

lx0::UIBinding*         create_uibinding();
lx0::View::Component*   create_renderer();
lx0::Controller*        create_controller(lx0::DocumentPtr spDoc);

//===========================================================================//

/*
    Ensure any settings set in the Document are pushed into Engine::globals().

    This allows the rest of the code to check the setting in Engine::globals()
    and know that's the appropriate value regardless of whether it was the
    default value, a command-line value, or a Document value.  In other words,
    it prevents each setting query from having to check multiple sources.
 */ 
static
void _processDocumentSettings (lx0::EnginePtr spEngine, lx0::DocumentPtr spDocument)
{
    lx0::lxvar& startingCellVar   = spEngine->globals()["startingCell"]; 
    if (!startingCellVar.is_string())
    {     
        std::string startingCell = spDocument->getElementsByTagName("Scene")[0]->attr("startingCell").as<std::string>();
        startingCellVar = startingCell;
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
        
        spEngine->globals().add("startingCell", lx0::eAcceptsString, lx0::validate_string());

        if (spEngine->parseCommandLine(argc, argv, "startingCell"))
        {
            spEngine->attachComponent(lx0::createPhysicsSubsystem());
            
            //
            // A bit of a hack at the moment...
            //
            void initializePhysics();
            initializePhysics();

            //
            // Load up the document and do the very initial processsing
            //
            DocumentPtr spDocument = spEngine->loadDocument("media2/appdata/lxmorrowind/lxmorrowind.xml");
            spDocument->attachComponent(new MwPhysicsDoc);
            spDocument->addController( create_controller(spDocument) );
            _processDocumentSettings(spEngine, spDocument);            
            lx0::processIncludeDocument(spDocument);

            //
            // Create the TES3 loader and attach it to the Engine to ensure it has the same lifetime
            // as the Engine (i.e. we can load data at any point during the run).
            //
            spEngine->attachComponent( ITES3Loader::create() );
            spEngine->getComponent<ITES3Loader>()->initialize("mwdata");

            //
            // Load the initial Morrowind cell
            //
            scene_group group;
            spEngine->getComponent<ITES3Loader>()->cell( spEngine->globals()["startingCell"].as<std::string>().c_str(), group);

            //
            // Process the loaded the scene_group data loaded from the Morrowind ESM/BSA
            // and push the data into the Document in the form of <Instance>, <Texture>,
            // <Light>, etc. Elements.
            // 
            glgeom::abbox3f sceneBounds;
            ElementPtr spGroup = spDocument->createElement("Group");
            for (auto it = group.instances.begin(); it != group.instances.end(); ++it)
            {
                ElementPtr spElement = spDocument->createElement("Instance");
                lxvar value = spElement->value();
                value["transform"] = lxvar::wrap(*it->spTransform);
                value["primitive"] = lxvar::wrap(*it->spPrimitive);
                value["material"]  = lxvar::wrap(it->material);
                spElement->value(value);
                spGroup->append(spElement);

                auto p0 = (*it->spTransform) * it->spPrimitive->bbox.max;
                auto p1 = (*it->spTransform) * it->spPrimitive->bbox.min;
                sceneBounds.merge(p0);
                sceneBounds.merge(p1);
            }
            for (auto it = group.textures.begin(); it != group.textures.end(); ++it)
            {
                auto spElement = spDocument->createElement("Texture");
                spElement->value( lxvar::wrap(*it) );
                spGroup->append(spElement);
            }
            for (auto it = group.lights.begin(); it != group.lights.end(); ++it)
            {
                auto spElement = spDocument->createElement("Light");
                spElement->value( lxvar::wrap(*it) );
                spGroup->append(spElement);
            }
            spDocument->root()->append(spGroup);

            //
            // Shift the player to a view of everything
            //
            ElementPtr spPlayer = spDocument->getElementsByTagName("Player")[0];
            spPlayer->value()["target"] = lxvar::wrap(sceneBounds.center());
            spPlayer->value()["position"] = lxvar::wrap( sceneBounds.center() + .6f * (sceneBounds.max - sceneBounds.min) );
            spPlayer->notifyValueChanged();
        
            //
            // Create a view of the Document now that it has data
            //
            ViewPtr spView = spDocument->createView("Canvas", "view", create_renderer() );
            spView->addUIBinding( create_uibinding() );

            lxvar options;
            options.insert("title", "LxMorrowind");
            options.insert("width", 800);
            options.insert("height", 400);
            spView->show(options);

            //
            // Run the main loop
            //
            exitCode = spEngine->run();

            spView.reset();
            spEngine->closeDocument(spDocument);
        }
        spEngine->shutdown();
    }
    catch (lx0::error_exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled std::exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
