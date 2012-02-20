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


//-----------------------------------------------------------------------//
//! Wrap a native object that does not have reference counting
/*!
    Stores the pointer to the native object in the "0" internal field of
    the JS object.

    \param ctor         JS Function for creating a JS object that will
        wrap the native object

    \param pNative      Pointer to the native object
        
    \param nativeBytes  The approximate weight of the native object, which
        the Javascript garbage collector will use as a hint for when to
        collect the object.
    */
static v8::Handle<v8::Object>
_wrapObject (v8::Handle<v8::Function>& ctor, void* pNative, size_t nativeBytes = 0)
{
    // Give V8 a hint about the native object size so that it invokes the garbage
    // collector at the right time.
    if (nativeBytes > 0)
    {
        #ifdef _DEBUG
        // Put more pressure on the GC in _DEBUG
        nativeBytes *= 32;          
        #endif

        v8::V8::AdjustAmountOfExternalAllocatedMemory(nativeBytes);
    }

    // Allocate the JS object wrapper and assign the native object to its
    // internal field.
    v8::Handle<v8::Object> obj = ctor->NewInstance();
    obj->SetInternalField(0, v8::External::New(pNative));

    return obj;
}

//=========================================================================
// V8 Ref-Counted Object Wrapper
//=========================================================================

class _SharedObjectWrapper
{
public:
    virtual ~_SharedObjectWrapper() {}
};

template <typename T>
class _SharedObjectWrapperImp : public _SharedObjectWrapper
{
public:
    _SharedObjectWrapperImp(T& t) : mValue(t) {}
    T mValue;
};

//-----------------------------------------------------------------------//
//! Wrap a native object that needs to be reference counted.
/*!
    The purpose of this method is to maintain a native shared pointer to the object as
    long as there is a JS reference to the object.  This ensures the JS and native 
    reference counts on the shared object stay in sync.

    The wrapper on shared object works as follows:

    1) Create a PersitentHandle<>.  This has a method MakeWeak(), which takes a callback
        function which is called when the V8 garbage collector is releasing the JS object.
        This is where the native object can be freed.

    2) Store a pointer to a new object that holds the shared reference in the parameters
        field of the PersistentHandle<> callback.

    3) Set up a generic callback function that deletes the wrapper object when called;
        this implicitly will release the shared reference owned by that object and thus
        handle reference counting on that object correctly.

    The advantange of this approach is that it's simple.  The disadvantage is it requires
    an additional heap object for every persistent, shared object.
    */
template <typename T>
static v8::Persistent<v8::Object>
_wrapSharedObject (v8::Handle<v8::Function>& ctor, T sharedPtr, size_t nativeBytes)
{
    struct L
    {
        static void releaseObj (v8::Persistent<v8::Value> persistentObj, void* pData)
        {
            // Release the native object by deleteing the object holding the
            // shared reference.
            auto pShared = reinterpret_cast<_SharedObjectWrapper*>(pData);
            delete pShared;
        
            // Clear out the object fields.  Technically not necessary, but just in case.
            v8::Local<v8::Object> obj( v8::Object::Cast(*persistentObj) );
            obj->SetInternalField(0, v8::Null());

            // Manually dispose of the PersistentHandle
            persistentObj.Dispose();
            persistentObj.Clear();
        }
    };

    v8::Persistent<v8::Object> obj( v8::Persistent<v8::Object>::New( _wrapObject(ctor, sharedPtr.get(), nativeBytes) ));
    obj.MakeWeak(new _SharedObjectWrapperImp<T>(sharedPtr), L::releaseObj);

    return obj;
}

static std::map<std::string,v8::Persistent<v8::Function>> constructors;
    
using namespace lx0::core::v8bind;   

static void
addLxDOMtoContext (lx0::DocumentPtr spDocument)
{
    using namespace v8;

    //
    // The wrappers need to be added inside the proper context
    //
    auto spJavascriptDoc = spDocument->getComponent<lx0::IJavascriptDoc>();
    spJavascriptDoc->runInContext([&]() {
        
        {
            Handle<FunctionTemplate> templ = FunctionTemplate::New();
            Local<ObjectTemplate> objInst = templ->InstanceTemplate();
            objInst->SetInternalFieldCount(1);

            v8::Persistent<v8::Function> ctor ( v8::Persistent<v8::Function>::New(templ->GetFunction()) );
            constructors.insert(std::make_pair("document", ctor));
        }                
        
        //
        // Create a function-local "namespace" for the wrappers
        //
        struct W
        {
            static v8::Handle<v8::Value> 
            loadDocument (const v8::Arguments& args)
            {
                lx0::Engine* pThis    = _nativeThis<lx0::Engine>(args);
                std::string  filename = _marshal(args[0]);
 
                auto spDocument = pThis->loadDocument(filename); 
                return _wrapSharedObject(constructors["document"], spDocument, sizeof(lx0::Document));
            }
        };

        // Create the "Engine" JS class.  Note that it is anonymous and thus inaccessible
        // since we only want to expose the singleton.
        Handle<FunctionTemplate> templ = FunctionTemplate::New();
        Local<ObjectTemplate> objInst = templ->InstanceTemplate();
        objInst->SetInternalFieldCount(1);

        Local<Template> proto_t = templ->PrototypeTemplate();
        proto_t->Set("loadDocument",  FunctionTemplate::New(W::loadDocument));
        
        // Create the JS object, associate it with the native object, and name it in the context
        //
        Handle<Function> ctor = templ->GetFunction();
 
        // Create a name for the object in the global namespace (i.e. global variable).
        //
        auto obj = _wrapSharedObject(ctor, lx0::Engine::acquire(), sizeof(lx0::Engine));
        spJavascriptDoc->addObject("engine", &obj);
    });
}

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    using namespace lx0;

    int exitCode = -1;
    
    std::cout << "LxEngineApp.exe prototype" << std::endl;
    std::cout << "================================================================================" << std::endl;

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

            //
            //
            //
            DocumentPtr spDocument = spEngine->createDocument();
            auto spJavascriptDoc = spDocument->getComponent<lx0::IJavascriptDoc>();

            addLxDOMtoContext(spDocument);
            std::string source = lx0::string_from_file("../dev/samples/manifest/raytracer2/main.js");
            spJavascriptDoc->run(source);

            spEngine->sendTask( [&](void) -> void { spJavascriptDoc->run("main();"); } );
            spEngine->sendEvent("quit");

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