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

#include "lxextensions/lxvar_wrap.hpp"

lx0::UIBinding*         create_uibinding();
lx0::View::Component*   create_renderer();
lx0::Controller*        create_controller(lx0::DocumentPtr spDoc);

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
        
        spEngine->globals().add("startingCell", lx0::eAcceptsString, lx0::validate_string());

        if (spEngine->parseCommandLine(argc, argv, "startingCell"))
        {
            DocumentPtr spDocument = spEngine->loadDocument("media2/appdata/lxmorrowind/lxmorrowind.xml");
            spDocument->addController( create_controller(spDocument) );
            lx0::processIncludeDocument(spDocument);

            ElementPtr spPlayer = spDocument->createElement("Player");
            spPlayer->value()["position"] = lxvar_wrap(glgeom::point3f(2000, 2000, 2000));
            spPlayer->value()["target"] = lxvar_wrap(glgeom::point3f(0, 0, 0));
            spDocument->root()->append(spPlayer);

            Tes3Io loader;
            loader.initialize("mwdata");

            // Let the command-line override the document's starting cell, if desired
            std::string startingCell = spDocument->getElementsByTagName("Scene")[0]->attr("startingCell").as<std::string>();
            lxvar& startingCellVar = spEngine->globals().find("startingCell"); 
            if (startingCellVar.is_string())
                startingCell = startingCellVar.as<std::string>();
            else
                startingCellVar = startingCell;

            scene_group group;
            loader.cell(startingCell.c_str(), group);

            ElementPtr spGroup = spDocument->createElement("Group");
            for (auto it = group.instances.begin(); it != group.instances.end(); ++it)
            {
                ElementPtr spElement = spDocument->createElement("Instance");
                lxvar value = spElement->value();
                value["transform"] = lxvar_wrap(it->transform);
                value["primitive"] = lxvar_wrap(it->primitive);
                value["material"] = lxvar_wrap(it->material);
                spElement->value(value);
                spGroup->append(spElement);
            }
            for (auto it = group.lights.begin(); it != group.lights.end(); ++it)
            {
                auto spElement = spDocument->createElement("Light");
                spElement->value( lxvar_wrap(*it) );
                spGroup->append(spElement);
            }
            spDocument->root()->append(spGroup);
        
            ViewPtr spView = spDocument->createView("Canvas", "view", create_renderer() );
            spView->addUIBinding( create_uibinding() );

            lxvar options;
            options.insert("title", "LxMorrowind");
            options.insert("width", 800);
            options.insert("height", 400);
            spView->show(options);

            exitCode = spEngine->run();

            spView.reset();
            spEngine->closeDocument(spDocument);
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
