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

#include <glgeom/ext/primitive_buffer.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>

using namespace v8;
using namespace lx0::core::v8bind;

//===========================================================================//
//   L O C A L   H E L P E R S 
//===========================================================================//

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

class V8Class
{   
public:
    typedef std::function<v8::Handle<v8::Value>(const v8::Arguments& args)> FunctorV8;

    V8Class ()
    {
        mTempl  = FunctionTemplate::New();
        mObject = mTempl->InstanceTemplate();
        mProto  = mTempl->PrototypeTemplate();        

        // Internal Field 0 = pointer to the native object
        // Internal Field 1 = typeof(T).hash_code() OR "full::typename"
        mObject->SetInternalFieldCount(2);
    }

    void constant (const char* name, lx0::lxvar value)
    {
        mObject->SetAccessor( String::New(name), _genericConstant, 0, _marshal(value) );
    }
    
    void function (const char* name, FunctorV8 func)
    {
        // Note: this functor pointer is leaked...
        auto funcPtr = new FunctorV8(func);
        auto funcV8 = FunctionTemplate::New(_genericFunction, v8::External::New(funcPtr));
        mProto->Set(name, funcV8);
    }

    void function (const char* name, v8::InvocationCallback func)
    {
        auto funcV8 = FunctionTemplate::New(func);
        mProto->Set(name, funcV8);
    }

    v8::Persistent<v8::Function> 
    getConstructor ()
    {
        return v8::Persistent<v8::Function>::New(mTempl->GetFunction());
    }

protected:
    static v8::Handle<Value> 
    _genericConstant (Local<String> prop, const AccessorInfo& info)
    {
        return info.Data();
    }

    static v8::Handle<Value> 
    _genericFunction (const v8::Arguments& args)
    {
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast( args.Data() );               
        FunctorV8* pFunc = reinterpret_cast<FunctorV8*>( wrap->Value() );
        return (*pFunc)(args);
    }

    Handle<FunctionTemplate> mTempl;
    Handle<ObjectTemplate>   mObject;
    Handle<Template>         mProto;      
};
    
//===========================================================================//
//   L O C A L   F U N C T I O N S
//===========================================================================//

namespace 
{
    //---------------------------------------------------------------------------//
    // Lx0 object/namespace
    //---------------------------------------------------------------------------//

    v8::Persistent<v8::Function> 
    construct_lx0()
    {
        V8Class lx;

        //
        // Add the keyboard constants
        //
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

        lx.function("assert", [](const v8::Arguments& args) -> v8::Handle<v8::Value> {
            bool b = _marshal(args[0]);
            if (!b)
            {
                int lineno = args.Callee()->GetScriptLineNumber();
                lx_message("Failed assert in Javascript script around line %1%", lineno);                
            }
            lx_check_error(b);
            return v8::Undefined();
        });

        lx.function("message", [](const v8::Arguments& args) -> v8::Handle<v8::Value> {
            lx_message("%s", (std::string)_marshal(args[0]));
            return v8::Undefined();
        });

        lx.function("log", [](const v8::Arguments& args) -> v8::Handle<v8::Value> {
            lx_log("%s", (std::string)_marshal(args[0]));
            return v8::Undefined();
        });

        return lx.getConstructor();
    }


    //---------------------------------------------------------------------------//
    // View
    //---------------------------------------------------------------------------//
     
    v8::Persistent<v8::Function> 
    construct_View ()
    {
        struct L
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
        };
        
        V8Class c;
        c.function("show",          _genericV1<lx0::View, lx0::lxvar, &lx0::View::show> );
        c.function("sendEvent",     _genericV2<lx0::View, std::string, lx0::lxvar, &lx0::View::sendEvent> );
        c.function("addUIBinding",  L::addUIBinding);
        return c.getConstructor();
    }

    //---------------------------------------------------------------------------//
    // Document
    //---------------------------------------------------------------------------//

    v8::Persistent<v8::Function> 
    construct_Document ()
    {
        V8Class c;
        c.function("createView", _genericR3<lx0::ViewPtr, lx0::Document, std::string, std::string, std::string, &lx0::Document::createView>);
        return c.getConstructor();
    }

    //---------------------------------------------------------------------------//
    // Engine
    //---------------------------------------------------------------------------//

    v8::Persistent<v8::Function> 
    construct_Engine ()
    {
        V8Class c;
        c.function("loadDocument",  [](const v8::Arguments& args) -> v8::Handle<v8::Value> {
            auto pEngine = _nativeThis<lx0::Engine>(args);
            std::string filename = _marshal(args[0]);

            lx0::DocumentPtr spDocument = pEngine->loadDocument(filename);
            void addLxDOMtoContext (lx0::DocumentPtr spDocument);
            addLxDOMtoContext(spDocument);

            return _marshal(spDocument);
        });

        //_genericR1<lx0::DocumentPtr, lx0::Engine, std::string, &lx0::Engine::loadDocument>);
        c.function("sendEvent",     _genericV1<lx0::Engine, std::string, &lx0::Engine::sendEvent>);
        c.function("loadPlugin",    _genericV1<lx0::Engine, std::string, &lx0::Engine::loadPlugin>);
        return c.getConstructor();
    }
}


ConstructorMap constructors;

void
addLxDOMtoContext (lx0::DocumentPtr spDocument)
{
    using namespace v8;

    _marshalActiveConstructorMap = &constructors;

    //
    // The wrappers need to be added inside the proper context
    //
    auto spJavascriptDoc = spDocument->getComponent<lx0::IJavascriptDoc>();
    spJavascriptDoc->runInContext([&]() {
                
        constructors.insert<lx0::Engine>  ( construct_Engine() );  
        constructors.insert<lx0::Document>( construct_Document() );            
        constructors.insert<lx0::View>    ( construct_View() );

        // Create a name for the object in the global namespace (i.e. global variable).
        //        
        spJavascriptDoc->addObject("engine", lx0::Engine::acquire() );

        // So "lx" is not backed by a native object, but for symmetry let's back it by
        // a stub object.
        struct Lx0Placeholder { virtual ~Lx0Placeholder() {} };
        constructors.insert<Lx0Placeholder>( construct_lx0() );
        spJavascriptDoc->addObject("lx0", std::shared_ptr<Lx0Placeholder>(new Lx0Placeholder));
    });
}
