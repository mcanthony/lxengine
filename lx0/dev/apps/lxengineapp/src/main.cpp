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

#include <v8/v8.h>
#include "../../../libs/lxengine/src/subsystem/javascript/v8bind.hpp"

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

using namespace v8;
using namespace lx0::core::v8bind;

class V8Class
{   
public:
    V8Class (const char* name)
        : mName (name)
    {
        mTempl  = FunctionTemplate::New();
        mObject = mTempl->InstanceTemplate();
        mProto  = mTempl->PrototypeTemplate();            
    }

    void constant (const char* name, lx0::lxvar value)
    {
        mObject->SetAccessor( String::New(name), _genericConstant, 0, _marshal(value) );
    }

    void addToContext(std::shared_ptr<lx0::IJavascriptDoc> spJavascriptDoc)
    {
        Handle<Function>   ctor = mTempl->GetFunction();
        Handle<v8::Object> obj  = ctor->NewInstance();

        spJavascriptDoc->addObject(mName.c_str(), &obj);
    }

protected:
    static v8::Handle<Value> 
    _genericConstant (Local<String> prop, const AccessorInfo& info)
    {
        return info.Data();
    }

    Handle<FunctionTemplate> mTempl;
    Handle<ObjectTemplate>   mObject;
    Handle<Template>         mProto;
    std::string              mName;
       
};




ConstructorMap constructors;
    
using namespace lx0::core::v8bind;   

template <typename T, typename A0, void (T::*Method)(A0)>
v8::Handle<v8::Value> _genericV1 (const v8::Arguments& args)
{
    T* pThis = _nativeThis<T>(args);
    A0 a0    = _marshal(args[0]);        
    (pThis->*Method)(a0);
    return v8::Undefined(); 
}

template <typename R, typename T, typename A0, R (T::*Method)(A0)>
v8::Handle<v8::Value> _genericR1 (const v8::Arguments& args)
{
    T* pThis = _nativeThis<T>(args);
    A0 a0    = _marshal(args[0]); 
    R  ret   = (pThis->*Method)(a0);
    return _marshal(ret);
}

template <typename T, typename A0, typename A1, void (T::*Method)(A0,A1)>
v8::Handle<v8::Value> _genericV2 (const v8::Arguments& args)
{
    T* pThis = _nativeThis<T>(args);
    A0 a0    = _marshal(args[0]);
    A1 a1    = _marshal(args[1]);                
    (pThis->*Method)(a0, a1);
    return v8::Undefined(); 
}

template <typename R, typename T, typename A0, typename A1, R (T::*Method)(A0,A1)>
v8::Handle<v8::Value> _genericR2 (const v8::Arguments& args)
{
    T* pThis = _nativeThis<T>(args);
    A0 a0    = _marshal(args[0]);
    A1 a1    = _marshal(args[1]);                
    R  ret   = (pThis->*Method)(a0, a1);
    return _marshal(ret); 
}

template <typename R, typename T, typename A0, typename A1, typename A2, R (T::*Method)(A0,A1,A2)>
v8::Handle<v8::Value> _genericR3 (const v8::Arguments& args)
{
    T* pThis = _nativeThis<T>(args);
    A0 a0    = _marshal(args[0]);
    A1 a1    = _marshal(args[1]);                
    A2 a2    = _marshal(args[2]);                
    R  ret   = (pThis->*Method)(a0, a1, a2);
    return _marshal(ret); 
}

static void
addLxDOMtoContext (lx0::DocumentPtr spDocument)
{
    using namespace v8;

    _marshalActiveConstructorMap = &constructors;

    //
    // The wrappers need to be added inside the proper context
    //
    auto spJavascriptDoc = spDocument->getComponent<lx0::IJavascriptDoc>();
    spJavascriptDoc->runInContext([&]() {
        
        struct View
        {
            static v8::Handle<v8::Value>
            addUIBinding (const v8::Arguments& args)
            {                               
                class Imp : public lx0::UIBinding
                {
                public:
                    virtual void onKeyDown (lx0::ViewPtr spView, int keyCode) 
                    {                        
                        auto spJavascriptDoc = spView->document()->getComponent<lx0::IJavascriptDoc>();
                        spJavascriptDoc->runInContext([&]() {
                            if (!mOnKeyDown->IsUndefined())
                            {
                                HandleScope handle_scope;                        
                        
                                Handle<Value> callArgs[2];                                       
                                callArgs[0] = _marshal(spView);
                                callArgs[1] = _marshal(keyCode);

                                Handle<Object> recv = v8::Object::New();
                                mOnKeyDown->Call(recv, 2, callArgs);
                            }
                        });   
                    }

                    virtual void updateFrame (lx0::ViewPtr spView, const lx0::KeyboardState& keyboard)
                    {
                        auto spJavascriptDoc = spView->document()->getComponent<lx0::IJavascriptDoc>();
                        spJavascriptDoc->runInContext([&]() {
                            if (!mUpdateFrame->IsUndefined())
                            {
                                // TODO: Add KeyboardState wrapper
                            }
                        }); 
                    }

                    v8::Persistent<v8::Function>    mOnKeyDown;
                    v8::Persistent<v8::Function>    mUpdateFrame;
                };
                
                auto pThis                = _nativeThis<lx0::View>(args);
                v8::Local<v8::Object> obj = v8::Object::Cast(*args[0]);
                
                auto pImp = new Imp;
                pImp->mOnKeyDown   = Persistent<Function>::New(Handle<Function>::Cast( obj->Get(v8::String::New("onKeyDown")) )); 
                pImp->mUpdateFrame = Persistent<Function>::New(Handle<Function>::Cast( obj->Get(v8::String::New("updateFrame")) ));
                pThis->addUIBinding(pImp);
                return v8::Undefined(); 
            }

            static void 
            _construct ()
            {
                Handle<FunctionTemplate> templ = FunctionTemplate::New();
                Local<Template> proto_t = templ->PrototypeTemplate();
                proto_t->Set("show",            FunctionTemplate::New( _genericV1<lx0::View, lx0::lxvar, &lx0::View::show> ) );
                proto_t->Set("sendEvent",       FunctionTemplate::New( _genericV2<lx0::View, std::string, lx0::lxvar, &lx0::View::sendEvent> ) );
                proto_t->Set("addUIBinding",    FunctionTemplate::New(addUIBinding));
            
                Local<ObjectTemplate> objInst = templ->InstanceTemplate();
                objInst->SetInternalFieldCount(1);

                v8::Persistent<v8::Function> ctor ( v8::Persistent<v8::Function>::New(templ->GetFunction()) );
                constructors.insert<lx0::View>(ctor);
            }
        };

        View::_construct();

        {
            Handle<FunctionTemplate> templ = FunctionTemplate::New();
            Local<Template> proto_t = templ->PrototypeTemplate();
            proto_t->Set("createView",  FunctionTemplate::New( _genericR3<lx0::ViewPtr, lx0::Document, std::string, std::string, std::string, &lx0::Document::createView> ) );
            
            Local<ObjectTemplate> objInst = templ->InstanceTemplate();
            objInst->SetInternalFieldCount(1);

            v8::Persistent<v8::Function> ctor ( v8::Persistent<v8::Function>::New(templ->GetFunction()) );
            constructors.insert<lx0::Document>(ctor);
        }                

        // Create the "Engine" JS class.  Note that it is anonymous and thus inaccessible
        // since we only want to expose the singleton.
        Handle<FunctionTemplate> templ = FunctionTemplate::New();
        Local<ObjectTemplate> objInst = templ->InstanceTemplate();
        objInst->SetInternalFieldCount(1);

        Local<Template> proto_t = templ->PrototypeTemplate();
        proto_t->Set("loadDocument",  FunctionTemplate::New( _genericR1<lx0::DocumentPtr, lx0::Engine, std::string, &lx0::Engine::loadDocument> ));
        proto_t->Set("sendEvent",     FunctionTemplate::New( _genericV1<lx0::Engine, std::string, &lx0::Engine::sendEvent> ));
        proto_t->Set("loadPlugin",    FunctionTemplate::New( _genericV1<lx0::Engine, std::string, &lx0::Engine::loadPlugin> ));
        
        // Create the JS object, associate it with the native object, and name it in the context
        //
        Handle<Function> ctor = templ->GetFunction();
 
        // Create a name for the object in the global namespace (i.e. global variable).
        //
        auto obj = _wrapSharedPtr(ctor, lx0::Engine::acquire(), sizeof(lx0::Engine));
        spJavascriptDoc->addObject("engine", &obj);

        V8Class lx ("lx0");
        lx.constant("KC_UNASSIGNED", lx0::KC_UNASSIGNED);
        lx.constant("KC_1", lx0::KC_1);
        lx.constant("KC_2", lx0::KC_2);
        lx.constant("KC_3", lx0::KC_3);
        lx.constant("KC_4", lx0::KC_4);
        lx.constant("KC_5", lx0::KC_5);
        lx.constant("KC_6", lx0::KC_6);
        lx.constant("KC_7", lx0::KC_7);
        lx.constant("KC_8", lx0::KC_8);
        lx.constant("KC_9", lx0::KC_9);
        lx.constant("KC_0", lx0::KC_0);
		lx.constant("KC_A", lx0::KC_A);
        lx.constant("KC_B", lx0::KC_B);
        lx.constant("KC_C", lx0::KC_C);
        lx.constant("KC_D", lx0::KC_D);
        lx.constant("KC_E", lx0::KC_E);
        lx.constant("KC_F", lx0::KC_F);
        lx.constant("KC_G", lx0::KC_G);
        lx.constant("KC_H", lx0::KC_H);
        lx.constant("KC_I", lx0::KC_I);
        lx.constant("KC_J", lx0::KC_J);
        lx.constant("KC_K", lx0::KC_K);
        lx.constant("KC_L", lx0::KC_L);
        lx.constant("KC_M", lx0::KC_M);
        lx.constant("KC_N", lx0::KC_N);
        lx.constant("KC_O", lx0::KC_O);
        lx.constant("KC_P", lx0::KC_P);
        lx.constant("KC_Q", lx0::KC_Q);
        lx.constant("KC_R", lx0::KC_R);
        lx.constant("KC_S", lx0::KC_S);
        lx.constant("KC_T", lx0::KC_T);
        lx.constant("KC_U", lx0::KC_U);
        lx.constant("KC_V", lx0::KC_V);
        lx.constant("KC_W", lx0::KC_W);
        lx.constant("KC_X", lx0::KC_X);
        lx.constant("KC_Y", lx0::KC_Y);
        lx.constant("KC_Z", lx0::KC_Z);
        lx.constant("KC_ESCAPE", lx0::KC_ESCAPE);
        lx.constant("KC_SPACE", lx0::KC_SPACE);
        lx.constant("KC_SHIFT", lx0::KC_SHIFT);
        lx.constant("KC_LSHIFT", lx0::KC_LSHIFT);
        lx.constant("KC_RSHIFT", lx0::KC_RSHIFT);
        lx.constant("KC_COUNT", lx0::KC_COUNT);

        lx.addToContext(spJavascriptDoc);
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

        spEngine->initialize();

        //
        // Set up global options prior to parsing the command line
        //
        spEngine->globals().add("manifest",     eAcceptsString, lx0::validate_filename());

        // Parse the command line (specifying "file" as the default unnamed argument)
        if (spEngine->parseCommandLine(argc, argv, "manifest"))
        {	
            spEngine->attachComponent(lx0::createJavascriptSubsystem());

            lx0::lxvar  manifest   = processManifest("media2/appdata/tutorial_04/manifest.lx");
            std::string mainScript = manifest["mainScript"].as<std::string>();

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

            addLxDOMtoContext(spDocument);
            std::string source = lx0::string_from_file(std::string("media2/appdata/tutorial_04/") + mainScript);
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