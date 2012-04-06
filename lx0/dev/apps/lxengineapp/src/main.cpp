//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2012 athile@athile.net (http://www.athile.net)

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
#include <lx0/util/misc/util.hpp>

//===========================================================================//
//   L O C A L   F U N C T I O N S
//===========================================================================//

static lx0::lxvar
processManifest (std::string filename)
{
    lx_log("Processing manifest");

    //
    // Create a document to act as the context for the manifest file
    // processing.  A Document acts as it's own self-contained
    // Javascript processing environment and context.
    // 
    auto spEngine = lx0::Engine::acquire();
    auto spDocument = spEngine->createDocument();

    //
    // Add pre-defined symbols to the environment
    //
    auto spJavascriptDoc = spDocument->getComponent<lx0::IJavascriptDoc>();

    //
    // Process the manifest
    //
    std::string source = lx0::string_from_file(filename);
    source = "(function() { return " + source + "; })();";
    lx0::lxvar manifest = spJavascriptDoc->run(source);

    return manifest;
}

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    using namespace lx0;

    int exitCode = -1;

    std::cout << 
        lx0::_lx_format("LxEngineApp.exe prototype (%1%.%2%.%3%)", 
            lx0::LXENGINE_VERSION_MAJOR,
            lx0::LXENGINE_VERSION_MINOR,
            lx0::LXENGINE_VERSION_REVISION)
        << std::endl;
    std::cout << "================================================================================" << std::endl;
    std::cout << std::endl;

    try
    {
        EnginePtr   spEngine   = Engine::acquire();

        spEngine->initialize();

        //
        // Set up global options prior to parsing the command line
        //
        spEngine->globals().add("base_directory",     eAcceptsString, lx0::validate_string());

        // Parse the command line (specifying "base_directory" as the default unnamed argument)
        if (spEngine->parseCommandLine(argc, argv, "base_directory"))
        {	
            spEngine->attachComponent(lx0::createJavascriptSubsystem());
            spEngine->attachComponent(lx0::createProcessScriptElement());

            //
            // Add resource directories (geometry, materials, textures, etc.)
            //
            spEngine->addResourceDirectory("common");
            std::string baseDir    = spEngine->globals()["base_directory"];
            if (!lx0::file_exists(baseDir))
                baseDir = lx0::_lx_format("apps/%1%/data", baseDir);
            spEngine->addResourceDirectory(baseDir);

            lx0::lxvar  manifest   = processManifest( spEngine->findResource("manifest.lx") );
            std::string mainScript = manifest["mainScript"].as<std::string>();

            //
            // LxEngine has a notion of fixed-duration frames.  Use the frameRate option
            // to set the duration of the frames.
            //
            if (manifest["frameRate"].is_defined())
                spEngine->setFrameDuration(1000 / manifest["frameRate"].as<int>());

            //
            // Load specified plug-ins
            //
            for (auto it = manifest["plugins"].begin(); it != manifest["plugins"].end(); ++it)
            {
                std::string name = (*it).as<std::string>();
                spEngine->loadPlugin(name);
            }

            //
            //
            //
            DocumentPtr spDocument = spEngine->createDocument();
            auto spJavascriptDoc = spDocument->getComponent<lx0::IJavascriptDoc>();

            void addLxDOMtoContext (lx0::DocumentPtr spDocument);
            addLxDOMtoContext(spDocument);
            std::string source = lx0::string_from_file(baseDir + "/" + mainScript);
            spJavascriptDoc->run(source);

            spEngine->sendTask( [&](void) -> void { spJavascriptDoc->run("main();"); } );

            exitCode = spEngine->run();
        }
        spEngine->shutdown();
    }
    catch (lx0::error_exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
    catch (std::exception& e)
    {
        throw lx_fatal_exception("Fatal: unhandled std::exception.\nException: %s\n", e.what());
    }

    return exitCode;
}