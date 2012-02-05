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

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/javascript.hpp>
#include <lx0/util/misc/util.hpp>

#include <v8/v8.h>
#include "../../../libs/lxengine/src/subsystem/javascript/v8bind.hpp"

//===========================================================================//
//   L O C A L   F U N C T I O N S
//===========================================================================//

static v8::Handle<v8::Object>
addValidators ()
{
    using namespace v8;
    using namespace lx0::core::v8bind;

    struct W
    {
        static Handle<Value> _modifyCallbackWrapper (lx0::ModifyCallback cb)
        {
            Handle<FunctionTemplate> templ( FunctionTemplate::New() );
            Handle<ObjectTemplate> objInst( templ->InstanceTemplate() );
            objInst->SetInternalFieldCount(1);

            lx0::ModifyCallback* func = new lx0::ModifyCallback;
            *func = cb;

            Handle<Object> obj( templ->GetFunction()->NewInstance() );
            obj->SetInternalField(0, External::New(func));
            return obj;
        }

        static Handle<Value> filename (const Arguments& args)
        {
            return _modifyCallbackWrapper( lx0::validate_filename() );
        }

        static Handle<Value> string (const Arguments& args)
        {
            return _modifyCallbackWrapper( lx0::validate_string() );
        }

        static Handle<Value> range (const Arguments& args)
        {
            int mn = _marshal(args[0]);
            int mx = _marshal(args[1]);
            return _modifyCallbackWrapper( lx0::validate_int_range(mn, mx) );
        }
    };


    Handle<FunctionTemplate>    templ( FunctionTemplate::New() );
    Handle<ObjectTemplate>      objInst( templ->InstanceTemplate() );
    objInst->SetInternalFieldCount(1);

    Handle<Template> proto_t( templ->PrototypeTemplate() );

    proto_t->Set("filename", FunctionTemplate::New(W::filename) );
    proto_t->Set("string", FunctionTemplate::New(W::string) );
    proto_t->Set("range", FunctionTemplate::New(W::range) );

    return templ->GetFunction()->NewInstance();
}

static void
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

    spJavascriptDoc->runInContext([&]() {
        spJavascriptDoc->addObject("validate", &addValidators());
    });   

    //
    // Grab the source
    //
    std::string source = lx0::string_from_file(filename);
    source = "var _manifest = " + source + ";";

    //
    // Process the manifest
    //
    spJavascriptDoc->run(source);

    return;
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
        spEngine->globals().add("manifest",     eAcceptsString, lx0::validate_filename());

        // Parse the command line (specifying "file" as the default unnamed argument)
        if (spEngine->parseCommandLine(argc, argv, "manifest"))
        {	
            spEngine->attachComponent(lx0::createJavascriptSubsystem());

            processManifest("../dev/samples/manifest/raytracer2/manifest.lx");
        
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