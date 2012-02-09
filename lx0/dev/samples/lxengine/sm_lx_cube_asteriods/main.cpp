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
//   H E A D E R S
//===========================================================================//

// Standard headers
#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

// Library headers
#include <boost/program_options.hpp>

// Lx0 headers
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/physics.hpp>
#include <lx0/subsystem/javascript.hpp>

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
        EnginePtr   spEngine   = Engine::acquire();
        spEngine->registerBuiltInPlugins();
        {
            spEngine->attachComponent( lx0::createJavascriptSubsystem());
            spEngine->attachComponent( lx0::createProcessScriptElement());
            spEngine->attachComponent( lx0::createPhysicsSubsystem());

            DocumentPtr spDocument = spEngine->loadDocument("data/sm_lx_cube_asteriods/level00.xml");
            ViewPtr     spView     = spDocument->createView("OGRE", "view");
            spView->show();
  
            exitCode = spEngine->run();
        }
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        throw lx_fatal_exception("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
