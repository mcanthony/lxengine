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

// Standard headers
#include <iostream>
#include <boost/program_options.hpp>
#include <glgeom/prototype/image.hpp>
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/canvas.hpp>
#include <lx0/subsystem/javascript.hpp>
#include <lx0/prototype/misc.hpp>

#include "raytracer.hpp"
#include "scripting.hpp"
#include "viewer.hpp"

//===========================================================================//

glgeom::image3f img(512, 512);


//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

struct Options
{
    std::string filename;
};

static bool 
parse_options (int argc, char** argv, boost::program_options::variables_map& vars)
{
    // See http://www.boost.org/doc/libs/1_44_0/doc/html/program_options/tutorial.html
    using namespace boost::program_options;

    // 
    // Build the description of the expected argument format and have
    // Boost parse the command line args.
    //
    std::string caption ("Syntax: %1 [options] <file>.\nOptions");
    size_t p = caption.find("%1");
    caption = caption.substr(0, p) + argv[0] + caption.substr(p + 2);

    options_description desc (caption);
    desc.add_options()
        ("help", "Print usage information and exit.")
        ("file", value<std::string>()->default_value("media2/appdata/sm_raytracer/basic_default_scene.xml"), "Scene file to display.")
        ;

    positional_options_description pos;
    pos.add("file", -1);

    store(command_line_parser(argc, argv).options(desc).positional(pos).run(), vars);

    //
    // Now check the options for anything that might prevent execution 
    //

    if (vars.count("help"))
    {
        std::cout << desc << std::endl;
        return false;
    }
    if (vars.count("file") != 1)
    {
        std::cout << "Error: expected exactly one scene file to be specified." << std::endl << std::endl;
        std::cout << desc << std::endl;
        return false;
    }


    return true;
}

static bool
validate_options (Options& options, int argc, char** argv)
{
    boost::program_options::variables_map vars;
    if (!parse_options(argc, argv, vars))
        return false;
     
    options.filename = vars["file"].as<std::string>();

    if (!lx0::lx_file_exists(options.filename))
    {
        std::cout << "Error: file '" << options.filename << "' could not be found." << std::endl << std::endl;
        return false;
    }

    return true;
}

int 
main (int argc, char** argv)
{
    using namespace lx0;

    int exitCode = -1;
    
    Options options;
    try
    {
        if (validate_options(options, argc, argv))
        {
            EnginePtr   spEngine   = Engine::acquire();
            spEngine->addViewPlugin("LxCanvas", [] (View* pView) { return create_viewer(); });
        
            DocumentPtr spDocument = spEngine->loadDocument(options.filename);
            spDocument->attachComponent("javascript", lx0::createIJavascript() );
            spDocument->attachComponent("ray", create_raytracer() );
            spDocument->attachComponent("scripting", create_scripting() );

            ViewPtr     spView     = spDocument->createView("LxCanvas", "view");
            spView->show();

            exitCode = spEngine->run();
            spEngine->shutdown();
        }
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
