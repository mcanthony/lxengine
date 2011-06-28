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
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/javascript.hpp>
#include <lx0/views/canvas.hpp>
#include "renderer.hpp"

using namespace lx0;

//===========================================================================//

    enum
    {
        ACCEPTS_INT     = (1 << 0),
        ACCEPTS_FLOAT   = (1 << 1),
        ACCEPTS_STRING  = (1 << 2)
    };

static bool 
parse_options (EnginePtr spEngine, int argc, char** argv, boost::program_options::variables_map& vars)
{
    // See http://www.boost.org/doc/libs/1_44_0/doc/html/program_options/tutorial.html
    using namespace boost::program_options;

    // 
    // Build the description of the expected argument format and have
    // Boost parse the command line args.
    //
    std::string caption = boost::str( boost::format("Syntax: %1% [options] <file>.\nOptions") % argv[0] );
    options_description desc (caption);
    desc.add_options()
        ("help", "Print usage information and exit.")
        ("file", value<std::string>()->default_value("media2/appdata/sm_raytracer/current.xml"), "Scene file to display.")
     //   ("view_width", value<int>())
       // ("view_height", value<int>())
        ;

    auto& adder = desc.add_options();
    for (auto it = spEngine->globals().begin(); it != spEngine->globals().end(); ++it)
    {
        auto key = it.key();
        auto flags = spEngine->globals().flags(key.c_str());

        if (flags & ACCEPTS_INT)
            adder((std::string("D") + key).c_str(), value<int>());
    }

    positional_options_description pos;
    pos.add("file", -1);

    try
    {
        store(command_line_parser(argc, argv).options(desc).positional(pos).run(), vars);
    }
    catch (std::exception& e)
    {
        std::cout << desc << std::endl;
        return false;
    }

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
    
    auto check_int = [&spEngine, &vars] (const char* name) {
        if (vars.count(std::string("D") + name))
            spEngine->globals().insert(name, vars[std::string("D") + name].as<int>());
    };
    check_int("view_width");
    check_int("view_height");

    return true;
}

static bool
validate_options (EnginePtr spEngine, int argc, char** argv)
{
    boost::program_options::variables_map vars;
    if (!parse_options(spEngine, argc, argv, vars))
        return false;
     
    auto filename = vars["file"].as<std::string>();
    spEngine->globals().insert("input_filename", filename);

    if (!lx0::lx_file_exists(filename))
    {
        std::cout << "Error: file '" << filename << "' could not be found." << std::endl << std::endl;
        return false;
    }

    return true;
}

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

lx0::ValidateFunction validate_range_int (int min, int max)
{
    return [min,max](lxvar v, lxvar& o) -> bool {
        if (v.is_int() && v.as<int>() >= min && v.as<int>() <= max)
        {
            o = v;
            return true;
        }
        return false;
    };
}

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        EnginePtr spEngine = Engine::acquire();
        spEngine->globals().insert("input_filename", lxvar::undefined());       // No default
        spEngine->globals().add("view_width",  ACCEPTS_INT, validate_range_int(32, 4096), 512);
        spEngine->globals().add("view_height", ACCEPTS_INT, validate_range_int(32, 4096), 512);

        if (validate_options(spEngine, argc, argv))
        {

            spEngine->attachComponent("Javascript", new JavascriptPlugin);
            spEngine->addViewPlugin("Canvas", [] (View* pView) { return lx0::createCanvasViewImp(); });
        
            DocumentPtr spDocument = spEngine->loadDocument(spEngine->globals().find("input_filename"));
            spDocument->addController( create_controller(spDocument) );

            ViewPtr spView = spDocument->createView("Canvas", "view", create_renderer() );
            spView->addUIBinding( create_uibinding() );

            lxvar options;
            options.insert("title", "LxEngine Rasterizer Sample");
            options.insert("width",     spEngine->globals().find("view_width"));
            options.insert("height",    spEngine->globals().find("view_height"));
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
