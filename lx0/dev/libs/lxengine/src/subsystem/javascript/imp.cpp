//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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
//   H E A D E R S
//===========================================================================//

#include <iostream>

#include <boost/format.hpp>

#include <v8/v8.h>

#include <lx0/engine/engine.hpp>
#include <lx0/engine/document.hpp>
#include <lx0/engine/element.hpp>
#include <lx0/engine/view.hpp>
#include <lx0/engine/mesh.hpp>
#include <lx0/util/misc/util.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>
#include <lx0/subsystem/javascript.hpp>
#include "v8bind.hpp"

using namespace v8;
using namespace lx0::core;
using namespace lx0::core::v8bind;
using namespace lx0::util;
using namespace lx0;

//===========================================================================//
//   I M P L E M E N T A T I O N 
//===========================================================================//

namespace {

    using v8::Object;

    //===========================================================================//
    // Engine Context
    //===========================================================================//

    class JsEngineContext 
        : public lx0::core::v8bind::_V8Context
        , public Engine::Component
    {
    public:
                            JsEngineContext (Engine* pEngine);

        virtual const char* name                (void) const { return "javascript"; }
        virtual void        onDocumentCreated   (EnginePtr spEngine, DocumentPtr spDocument);

        Engine*     mpEngine;
    protected:
        void        _addEngine  ();
    };

    namespace wrappers_engine
    {
        /*
            This is a fairly "old" function from initial prototyping in the code.
            It doesn't make a lot of sense in the current design, as it's very
            specific to the idea of a CSS-like Element attribute parser.  The 
            concept of attribute parsers still makes sense eventually - but this
            likely isn't the implementation that is desired.
         */
        static v8::Handle<v8::Value> 
        addAttributeParser (const v8::Arguments& args)
        {
            auto                 pThis = lx0::core::v8bind::_nativeThis<JsEngineContext>(args);
            auto                 attr  = lx0::core::v8bind::_marshal2<std::string>(args[0]); 
            Persistent<Function> func  = Persistent<Function>::New( Handle<Function>::Cast(args[1]) ); 
                
            auto wrapper = [pThis, func] (std::string value) -> lxvar { 
                Context::Scope context_scope(pThis->context);

                HandleScope handle_scope;
                Handle<v8::Object> recv = pThis->context->Global();
                Handle<Value> callArgs[1];
                
                Handle<Value> ret;
                callArgs[0] = String::New(value.c_str());
                ret = func->Call(recv, 1, callArgs);

                return lx0::core::v8bind::_marshal(ret);
            };
            pThis->mpEngine->addAttributeParser(attr, wrapper);
            return v8::Undefined();
        }

        static v8::Handle<v8::Value> 
        debug (const v8::Arguments& args)
        {
            std::string msg = *v8::String::AsciiValue(args[0]);                 
            std::cout << "JS: " << msg << std::endl;
            return v8::Undefined();
        }
    }

    void 
    JsEngineContext::_addEngine()
    {
        namespace W = wrappers_engine;

        Context::Scope context_scope(context);
        HandleScope handle_scope;

        Handle<FunctionTemplate> templ( FunctionTemplate::New() );
        
        Handle<ObjectTemplate> objInst( templ->InstanceTemplate() );
        objInst->SetInternalFieldCount(2);

        Handle<Template> proto_t( templ->PrototypeTemplate() );
        proto_t->Set("debug",               FunctionTemplate::New(W::debug));
        proto_t->Set("addAttributeParser",  FunctionTemplate::New(W::addAttributeParser));

        Handle<Function> ctor( templ->GetFunction() );
        Handle<v8::Object> obj( ctor->NewInstance() );
        obj->SetPointerInInternalField(0, new std::shared_ptr<JsEngineContext>(this));
        obj->SetPointerInInternalField(1, reinterpret_cast<void*>( typeid(JsEngineContext).hash_code() ) );

        context->Global()->Set(String::New("engine"), obj);
    }

    //===========================================================================//
    // Element Component
    //===========================================================================//

    /*!
        This Component is attached to every Element in every Document.

        Dev Notes:

        Eventually this likely should be a smart implementation that is a minimal
        object if no callbacks are registered on the particular Element; once
        JS callbacks and other extended information is added to the Component, it
        should internally switch a heavier-weight implementation.
     */
    class JavascriptElem : public Element::Component
    {
    public:
                        ~JavascriptElem();

        virtual const char* name() const { return "javascript"; }

        virtual lx0::uint32 flags() const;

        virtual void    onUpdate            (ElementPtr spElem);

        Persistent<Function>            mOnUpdate;

        std::map<std::string, v8::Persistent<v8::Function>> mCallbacks;
    };

    //===========================================================================//
    // Document Component
    //===========================================================================//

    class JavascriptDoc : public lx0::IJavascriptDoc
    {
    public:
                            JavascriptDoc       (DocumentPtr spDocument);
        virtual             ~JavascriptDoc      (void);

        virtual void        onElementAdded      (DocumentPtr spDocument, ElementPtr spElem);
        virtual void        onUpdate            (DocumentPtr spDocument);


        virtual void        addObject           (const char* name, void* pointerToHandleToObject);
        virtual lx0::lxvar  run                 (const std::string& source);        
        
        virtual void        _addObject           (const char* objectName,  size_t type_hash, void* pObject, std::function<void()> dtor);

        template <typename R, typename T0, typename T1>
        void _acquireFunction (const char* functionName, std::function<R(T0,T1)>& func);

        template <typename R, typename T0, typename T1, typename T2>
        void _acquireFunction (const char* functionName, std::function <R(T0,T1,T2)>& func);

        template <typename T>
        bool _checkFunction (const char* functionName, boost::any& func);
        
        virtual void        _acquireFunction     (const char* functionName, boost::any& func);


        virtual void        runInContext        (std::function<void(void)> func);

        void                _processTimeoutQueue        (void);

        //@name Add functions/objects to JS context
        //@{
        void                            _addGlobals     (v8::Handle<v8::ObjectTemplate>& globalTempl);
        void                            _addWindow      (void);
        void                            _addDocument    (DocumentPtr spDocument);
        v8::Persistent<v8::Function>    _addElement     (void);
        v8::Persistent<v8::Function>    _addKeyEvent    (void);
        v8::Persistent<v8::Function>    _addLxVar       (void);
        void                            _addMath        (void);
        //@}

        Document*                       mpDocument;

        v8::Persistent<v8::Context>     mContext;
        
        v8::Persistent<v8::Function>    mElementCtor;
        v8::Persistent<v8::Function>    mLxVarCtor;
        v8::Persistent<v8::Function>    mKeyEventCtor;

        Persistent<Function>            mOnUpdate;
        Persistent<Function>            mWindowOnKeyDown;
        std::vector<std::pair<int, Persistent<Function>>> mTimeoutQueue;



    };

    //=======================================================================//

    JsEngineContext::JsEngineContext (Engine* pEngine)
        : mpEngine (pEngine)
    {
        _addEngine();
    }

    void
    JsEngineContext::onDocumentCreated (EnginePtr spEngine, DocumentPtr spDocument)
    {
        spDocument->attachComponent(new JavascriptDoc(spDocument));

        //
        // Auto-loading attribute parsers is a relic of earlier code: the engine 
        // should not being doing something so specific.
        //
        lx0::for_files_in_directory("media2/scripts/engine/attribute_parsers", "js", [&] (std::string path) {
            this->runFile(path.c_str());
        });
    }

    //=======================================================================//

    JavascriptElem::~JavascriptElem()
    {
        for (auto it = mCallbacks.begin(); it != mCallbacks.end(); ++it)
            it->second.Dispose();
    }

    lx0::uint32 
    JavascriptElem::flags() const
    {
        lx0::uint32 flags = 0;

        flags |= mOnUpdate.IsEmpty() ? eSkipUpdate : eCallUpdate;

        return flags;
    }

    void
    JavascriptElem::onUpdate (ElementPtr spElem) 
    {
        // Call the onUpdate() JS function if one has been attached to that event
        if (!mOnUpdate.IsEmpty())
        {
            auto spJsDoc = spElem->document()->getComponent<JavascriptDoc>("javascript");
            Context::Scope context_scope(spJsDoc->mContext);

            //\todo Need to set the "this" ponter to the JS Element wrapper
            HandleScope handle_scope;
            Handle<Object> recv = spJsDoc->mContext->Global();
            mOnUpdate->Call(recv, 0, 0);
        }
    }

    //=======================================================================//

    JavascriptDoc::JavascriptDoc (DocumentPtr spDocument)
        : mpDocument (spDocument.get())
    {
        // Set up the global template before initiating the Context.   The ObjectTemplate lets
        // FunctionTemplates be added for the free functions.  Context::Global() returns an
        // Object, not an ObjectTemplate - therefore, it limits what can be added after the
        // Context has been created.
        //
        HandleScope handle_scope;
        Handle<ObjectTemplate> global_templ( ObjectTemplate::New() ); 

        _addGlobals(global_templ);
        
        mContext = Context::New(0, global_templ);

        Context::Scope context_scope(mContext);
        _addWindow();
        _addDocument(spDocument);
        mElementCtor    = _addElement();
        mKeyEventCtor   = _addKeyEvent();
        mLxVarCtor      = _addLxVar();
        _addMath();

        spDocument->slotKeyDown += [&](KeyEvent& e) {

            if (!mWindowOnKeyDown.IsEmpty())
            {
                // This callback could be invoked outside the normal execution of a JS script.
                // Therefore, it's necessary to set up the right context to call the Function
                // before invoking it.
                //
                Context::Scope context_scope(mContext);

                HandleScope handle_scope;
                Handle<Object> recv = mContext->Global();
                Handle<Value> callArgs[1];
                callArgs[0] = _marshal(std::shared_ptr<KeyEvent>(new KeyEvent(e)));
                mWindowOnKeyDown->Call(recv, 1, callArgs);
            }
        };

        spDocument->slotUpdateRun += [&]() {
            this->_processTimeoutQueue();
        };
    }

    /*!
        The JS interface has a setTimeout() function for registering functions to be called
        after a certain amount of elapsed time: this processes the queue of waiting functions
        to see if any have "expired" and need to be called.
     */
    void
    JavascriptDoc::_processTimeoutQueue (void)
    {
        const int now = int( lx_milliseconds() );

        Context::Scope context_scope(mContext);

        HandleScope handle_scope;
        Handle<Value> callArgs[1];
        Handle<Object> recv = mContext->Global();

        size_t i = 0; 
        while (i < mTimeoutQueue.size())
        {
            auto item = mTimeoutQueue[i];
            if (now >= item.first)
            {
                item.second->Call(recv, 0, 0);
                item.second.Dispose();

                mTimeoutQueue[i]  = mTimeoutQueue.back();
                mTimeoutQueue.pop_back();
            }
            else
                ++i;
        }
    }

    /*
        Persistent handle callbacks may need to be called.

        Credit: http://www.my-ride-home.com/2011/01/v8-garbage-collection/
     */
    static void 
    _forceGarbageCollection()
    {
        for (unsigned int i = 0; i < 4096; ++i)
        {
            if (v8::V8::IdleNotification())
                break;
        }
    }

    JavascriptDoc::~JavascriptDoc()
    {
        mElementCtor.Dispose();
        mKeyEventCtor.Dispose();
        mWindowOnKeyDown.Dispose();
        
        {
            Context::Scope    context_scope(mContext);
            HandleScope       handle_scope;
            _forceGarbageCollection();
        }

        mContext.Dispose();
    }

    void
    JavascriptDoc::onElementAdded (DocumentPtr spDocument, ElementPtr spElem)
    {
        static std::set<Element*> sVisited;

        if (!sVisited.insert(spElem.get()).second)
            assert(0);

        // Whenever a new Element is added to the Document, ensure the JS Element
        // Component is added to that Element.
        spElem->attachComponent(new JavascriptElem);
    }

    void
    JavascriptDoc::onUpdate (DocumentPtr spDocument) 
    {
        // Call the onUpdate() JS function if one has been attached to that event
        if (!mOnUpdate.IsEmpty())
        {
            Context::Scope context_scope(mContext);

            HandleScope handle_scope;
            Handle<Object> recv = mContext->Global();
            mOnUpdate->Call(recv, 0, 0);
        }
    }

    void        
    JavascriptDoc::_addObject (const char* objectName, size_t hash, void* pspObject, std::function<void()> dtor)
    {
        //
        // First, we need to allocate a JS object of the right type.  The JS wrapper 
        // constructors are stored in a table of native type_info hash -> V8 constructor.
        //
        // Then allocate the object and link to the native object by setting the 
        // "internal field" to point to that object.
        //
        v8::Persistent<v8::Function> ctor = _marshalActiveConstructorMap->getFromHash(hash);
        v8::Handle<v8::Object> obj = ctor->NewInstance();
        obj->SetPointerInInternalField(0, pspObject);
        obj->SetPointerInInternalField(1, reinterpret_cast<void*>(hash));
        
        //
        // Now, to keep the reference counting in sync, we create a Persistent handle
        // that will call Release::apply when the V8 garbage collector is about to 
        // dispose of the JS wrapper on the object (i.e. the object is no longer 
        // referenced in the script).
        //
        // The "dtor" argument is called when the GC would occur.  A copy of the dtor 
        // argument is made on the heap.
        //
        struct Release
        {
            static void apply(v8::Persistent<v8::Value> persistentObj, void* pData)
            {
                auto pFunc = reinterpret_cast<std::function<void()>*>(pData);
                (*pFunc)();
                delete pFunc;

                persistentObj.Dispose();
                persistentObj.Clear();
            }
        };

        v8::Persistent<v8::Object> persistentObj( v8::Persistent<v8::Object>::New(obj));        
        auto pFunctor = new std::function<void()>(dtor);
        persistentObj.MakeWeak(pFunctor, &Release::apply);

        //
        // Lastly, now that the object is created and the proper clean-up callback is in place,
        // we add the object to the current context under the specified name.
        //
        mContext->Global()->Set(String::New(objectName), obj);
    }

    /*!
        Add a V8 object to the document context
     */
    void
    JavascriptDoc::addObject (const char* name, void* pointerToHandleToObject)
    {
        //
        // The reinterpret_cast is used to avoid polluting the namespace with V8 symbols.
        //
        Handle<Object>& handle = *reinterpret_cast< Handle<Object>* >( pointerToHandleToObject );
        mContext->Global()->Set(String::New(name), handle);
    }

    static const char* ToCString(const v8::String::Utf8Value& value) 
    {
     return *value ? *value : "<string conversion failed>";
    }

    //
    // Adapted from: http://v8.googlecode.com/svn/trunk/samples/lineprocessor.cc
    //
    static void reportException(v8::TryCatch* try_catch) 
    {
        v8::HandleScope handle_scope;
        v8::String::Utf8Value exception(try_catch->Exception());

        const char* exception_string = ToCString(exception);

        v8::Handle<v8::Message> message = try_catch->Message();
        if (message.IsEmpty()) 
        {
            // V8 didn't provide any extra information about this error; just
            // print the exception.
            lx_message("V8 Exception: %s", exception_string);
        } 
        else 
        {
            // Print (filename):(line number): (message).
            v8::String::Utf8Value filename(message->GetScriptResourceName());
            const char* filename_string = ToCString(filename);
            int linenum = message->GetLineNumber();
            
            lx_message("V8 Exception: %s:%i: %s\n", filename_string, linenum, exception_string);
            
            // Print line of source code.
            v8::String::Utf8Value sourceline(message->GetSourceLine());
            const char* sourceline_string = ToCString(sourceline);
            
            lx_message("%s", sourceline_string);

            std::string s;
            int start = message->GetStartColumn();
            for (int i = 0; i < start; i++) 
              s += " ";
            int end = message->GetEndColumn();
            for (int i = start; i < end; i++) 
              s += "^";
            lx_message("%s", s);
        }
    }

    /*!
        Run the associated source text within the context of the given document.
     */
    lx0::lxvar 
    JavascriptDoc::run (const std::string& text)
    {      
        Context::Scope context_scope(mContext);
        HandleScope    handle_scope;

        Handle<String> source = String::New(text.c_str());
        Handle<Value>  result;
       
        //
        // Compile
        //
        TryCatch trycatch;
        Handle<Script> script = Script::Compile(source);

        if (script.IsEmpty())
        {
            reportException(&trycatch);
            throw lx_error_exception("Javascript script failed to compile!");
            result = v8::Undefined();
        }
        else
        {
            // Run the script
            TryCatch trycatch;
            
            result = script->Run();
            
            if (result.IsEmpty()) 
            {  
                Handle<Value> exception = trycatch.Exception();
                
                String::AsciiValue exception_str(exception);
                lx_warn("Javascript V8 Exception: %s", *exception_str);

                reportException(&trycatch);
            }
        }
        return _marshal(result);
    }
    

    void  JavascriptDoc::runInContext (std::function<void(void)> func)
    {
        Context::Scope    context_scope(mContext);
        HandleScope       handle_scope;
        
        func();
    }


    template <typename R, typename T0, typename T1>
    void
    JavascriptDoc::_acquireFunction (const char* functionName, std::function <R(T0,T1)>& funcObj)
    {
        Handle<v8::Object> global( mContext->Global() );
        Handle<v8::Value>  value = global->Get(String::New(functionName)); 
        
        if (value->IsFunction())
        {
            Handle<v8::Function> func( v8::Handle<v8::Function>::Cast(value) );
                
            funcObj = [func,global](T0 a0, T1 a1) -> R {
                Handle<Value> args[2];
                args[0] = _marshal(a0);
                args[1] = _marshal(a1);
                Handle<Value> ret = func->Call(global, 2, args);
                return _marshal(ret);
            };
        }
    }

    template <typename R, typename T0, typename T1, typename T2>
    void
    JavascriptDoc::_acquireFunction (const char* functionName, std::function <R(T0,T1,T2)>& funcObj)
    {
        Handle<v8::Object> global( mContext->Global() );
        Handle<v8::Value>  value = global->Get(String::New(functionName)); 
        
        if (value->IsFunction())
        {
            Handle<v8::Function> func( v8::Handle<v8::Function>::Cast(value) );
                
            funcObj = [func,global](T0 a0, T1 a1, T2 a2) -> R {
                Handle<Value> args[3];
                args[0] = _marshal(a0);
                args[1] = _marshal(a1);
                args[2] = _marshal(a2);
                Handle<Value> ret = func->Call(global, 3, args);
                return _marshal(ret);
            };
        }
    }

    template <typename T>
    bool JavascriptDoc::_checkFunction (const char* functionName, boost::any& func)
    {
        if (func.type() == typeid(std::function<T>))
        {
            std::function<T> funcObj;
            _acquireFunction(functionName, funcObj);
            func = funcObj; 
            return true;
        }
        else
            return false;
    }


    void 
    JavascriptDoc::_acquireFunction (const char* functionName, boost::any& func)
    {
        if (_checkFunction<float (float,float)>(functionName, func))
            return;
        if (_checkFunction<glm::vec2 (float,float)>(functionName, func))
            return;
        if (_checkFunction<glm::vec3 (float,float)>(functionName, func))
            return;

        if (_checkFunction<float (float,float,float)>(functionName, func))
            return;
        if (_checkFunction<glm::vec2 (float,float,float)>(functionName, func))
            return;
        if (_checkFunction<glm::vec3 (float,float,float)>(functionName, func))
            return;
    }


    void
    JavascriptDoc::_addGlobals (v8::Handle<v8::ObjectTemplate>& globalTempl)
    {
        struct L
        {
            static v8::Handle<v8::Value> __lx_print (const v8::Arguments& args)
            {
                std::string name = _marshal(args[0]);
                std::cout << "JS print: " << name << std::endl;
                return Undefined();
            }

            static v8::Handle<v8::Value> alert (const v8::Arguments& args)
            {
                if (args.Length() == 1)
                {
                    std::string text = _marshal(args[0]->ToString());
                    
                    lx_log("JS alert: '%s'", text.c_str());
                    lx0::lx_message_box("Alert", text);
                }
                else
                    throw lx_error_exception("alert() called from script with too few arguments.");

                return Undefined();
            }
        };


        // Internal debugging methods to make development a little easier.
        //
        globalTempl->Set(String::New("__lx_print"), FunctionTemplate::New(L::__lx_print));

        globalTempl->Set(String::New("alert"), FunctionTemplate::New(L::alert));
    }

    void
    JavascriptDoc::_addWindow (void)
    {
        struct Window
        {
            static v8::Handle<v8::Value> 
            get_onKeyDown (Local<String> property, const AccessorInfo &info) 
            {
                auto   pContext = _nativeData<JavascriptDoc>(info);
                return pContext->mWindowOnKeyDown;
            }

            static void
            set_onKeyDown (Local<String> property, Local<Value> value, const AccessorInfo &info) 
            {
                auto             pContext = _nativeData<JavascriptDoc>(info);
                Handle<Function> func     = _marshal(value);
                pContext->mWindowOnKeyDown = Persistent<Function>::New(func);
            }

            static v8::Handle<v8::Value>
            isKeyDown (const v8::Arguments& args)
            {
                if (args.Length() == 1)
                {
                    auto             pContext = _nativeData<JavascriptDoc>(args);
                    auto             pThis    = _nativeThis<Window>(args); 
                    int              keyCode  = _marshal(args[0]);

                    ViewPtr spView = pContext->mpDocument->view(0);
                    return _marshal( spView->isKeyDown(keyCode) );
                }
                else
                    return v8::Undefined();
            }
            
            static v8::Handle<v8::Value> 
            setTimeout (const v8::Arguments& args)
            {
                lx_check_error(args.Length() == 2);
                lx_check_error(args[0]->IsNumber(), "Expected a number for first argument to window.setTimeout()");
                lx_check_error(args[1]->IsFunction());

                auto             pContext = _nativeData<JavascriptDoc>(args);
                auto             pThis    = _nativeThis<Window>(args); 
                int              delay    = _marshal(args[0]);
                Handle<Function> func     = _marshal(args[1]);
                
                delay += int(lx_milliseconds());

                auto pair = std::make_pair(delay, Persistent<Function>::New(func));
                pContext->mTimeoutQueue.push_back(pair);

                return Undefined();
            }
        };

        // Create the template and add the prototype methods
        //
        Handle<FunctionTemplate> templ( FunctionTemplate::New() );
        Handle<ObjectTemplate> objInst( templ->InstanceTemplate() );
        objInst->SetInternalFieldCount(2);

        objInst->SetAccessor(String::New("onKeyDown"), Window::get_onKeyDown, Window::set_onKeyDown, External::New(this));

        Handle<Template> proto_t( templ->PrototypeTemplate() );
        proto_t->Set("setTimeout",  FunctionTemplate::New(Window::setTimeout, External::New(this)));
        proto_t->Set("isKeyDown",  FunctionTemplate::New(Window::isKeyDown, External::New(this)));

        // Add the object
        Handle<Object> obj( templ->GetFunction()->NewInstance() );
        obj->SetInternalField(0, External::New(new Window));
        mContext->Global()->Set(String::New("window"), obj);
    }

    void
    JavascriptDoc::_addDocument (DocumentPtr spDocument)
    {
        // Local functions
        struct L
        {
            static v8::Handle<v8::Value> 
            get_onUpdate (Local<String> property, const AccessorInfo &info) 
            {
                auto   pContext = _nativeData<JavascriptDoc>(info);
                return pContext->mOnUpdate;
            }

            static void
            set_onUpdate (Local<String> property, Local<Value> value, const AccessorInfo &info) 
            {
                auto             pContext = _nativeData<JavascriptDoc>(info);
                Handle<Function> func     = _marshal(value);
                pContext->mOnUpdate = Persistent<Function>::New(func);
            }

            /*!
                Wraps document.createElement()
             */
            static v8::Handle<v8::Value> 
            createElement (const v8::Arguments& args)
            {
                auto      pContext  = _nativeData<JavascriptDoc>(args);
                Document* pThis     = _nativeThis<Document>(args);
                std::string name    = _marshal(args[0]);
        
                ElementPtr spElem   = pThis->createElement(name);
        
                // Wrap the returned object as a shared pointer: this will keep a native
                // reference to object open as long as there is a JS reference to the object.
                // In other words, it keeps the JS and native reference counting in sync.
                //
                return _marshal(spElem);
            }

            /*
                Wrapper on Document::getElementById()
             */
            static v8::Handle<v8::Value> 
            getElementById (const v8::Arguments& args)
            {
                lx_check_error(args.Length() == 1);

                auto        pContext  = _nativeData<JavascriptDoc>(args);
                Document*   pDoc      = _nativeThis<Document>(args); 
                std::string id        = _marshal(args[0]);
        
                ElementPtr spElem = pDoc->getElementById(id);

                if (!spElem)
                    return Undefined();
                else
                    return _marshal(spElem);
            }

            static v8::Handle<v8::Value> 
            getElementsByTagName (const v8::Arguments& args)
            {
                lx_check_error(args.Length() == 1);

                auto        pContext  = _nativeData<JavascriptDoc>(args);
                Document*   pDoc      = _nativeThis<Document>(args); 
                std::string tag       = _marshal(args[0]);
        
                auto elems = pDoc->getElementsByTagName(tag);
                Local<Array> results = Array::New(elems.size()); 
                for (int i = 0; i < int(elems.size()); ++i)
                    results->Set(i, _marshal(elems[i]) );
                
                return results;
            }

        };


        // Create the FunctionTemplate.  Think of the "Template" part as being the 
        // descriptor or specification that is be used to create the actual Function
        // when it is created and put into the V8 context.
        //
        // Note that this is essentially an anonymous function, since no name is assigned
        // and the only way it is being accessed is via the V8 API.
        //
        Handle<FunctionTemplate> templ( FunctionTemplate::New() );

        // Get the ObjectTemplate for the Function.  This is the specification used when
        // the function is invoked as a constructor (i.e. var obj = new my_func()).
        //
        // Note: it seems this needs to be set before the Function is actually created
        // (http://code.google.com/p/v8/issues/detail?id=262).
        //
        Handle<ObjectTemplate> objInst( templ->InstanceTemplate() );
        objInst->SetInternalFieldCount(2);
        objInst->SetAccessor(String::New("onUpdate"), L::get_onUpdate, L::set_onUpdate, External::New(this));

        // Access the Javascript prototype for the function - i.e. my_func.prototype - 
        // and add the necessary properties and methods.
        //
        Handle<Template> proto_t( templ->PrototypeTemplate() );
        proto_t->Set("createElement",  FunctionTemplate::New(L::createElement, External::New(this)));
        proto_t->Set("getElementById", FunctionTemplate::New(L::getElementById, External::New(this)));
        proto_t->Set("getElementsByTagName", FunctionTemplate::New(L::getElementsByTagName, External::New(this)));

        // Now grab a handle to the Function.  This apparently (?) will invoke the
        // FunctionTemplate to create actual function.  Then call NewInstance, which is
        // the C++ equivalent of "new my_func()".   Then, since this is a wrapper on a
        // C++ object, set the internal field to point to the C++ object.
        //
        Handle<Function> ctor( templ->GetFunction() );
        Handle<Object> obj( ctor->NewInstance() );
        obj->SetInternalField(0, External::New(spDocument.get()));

        // Create a name for the object in the global namespace (i.e. global variable).
        //
        mContext->Global()->Set(String::New("document"), obj);
    }
    
    namespace wrapper_lxvar
    {
        Handle<Value> 
        getNamedProperty (Local<String> name, const AccessorInfo &info)
        {
            auto        pContext = _nativeData<JavascriptDoc>(info);
            auto        pThis    = _nativeThis<lxvar>(info); 
            std::string prop     = _marshal(name);

            if (prop == "toString" || prop == "valueOf")
                return _marshal("[Native lxvar]");
            
            lxvar v = pThis->find(prop.c_str());

            std::shared_ptr<lxvar> spWrap(new lxvar(v));
            return _marshal(spWrap);
        }

        Handle<Value> 
        setNamedProperty(Local<String> name, Local<Value> value, const AccessorInfo& info) 
        {
            return v8::Undefined();
        }

        Handle<Array>
        enumerateIndexedProperties (const AccessorInfo& info)
        {
            auto        pThis    = _nativeThis<lxvar>(info); 

            v8::Local<v8::Array> arr;
            if (pThis->is_array())
            {
                arr = v8::Array::New(pThis->size());
                for (int i = 0; i < pThis->size(); ++i)
                {
                    arr->Set(uint32_t(i), v8::Integer::New(i));
                }
            }
            else
                arr = v8::Array::New(0);

            return arr;
        }

        Handle<Array>
        enumerateNamedProperties (const AccessorInfo &info)
        {
            auto        pThis    = _nativeThis<lxvar>(info); 

            v8::Local<v8::Array> arr;

            if (pThis->is_map())
            {
                uint32_t index = 0;
                arr = v8::Array::New(pThis->size());
                for (auto it = pThis->begin(); it != pThis->end(); ++it)
                {
                    arr->Set( index++, _marshal(it.key()) );
                }
            }
            else
                arr = v8::Array::New(0);

            return arr;
        }


        Handle<Value> 
        getIndexedProperty (uint32_t index, const AccessorInfo &info)
        {
            auto        pContext = _nativeData<JavascriptDoc>(info);
            auto        pThis    = _nativeThis<lxvar>(info); 
            
            lxvar v = pThis->at(int(index));

            if (v.isSharedType())
            {
                std::shared_ptr<lxvar> spWrap(new lxvar(v));
                return _marshal(spWrap);
            }
            else
                return _marshal(v);
        }

        Handle<Value> 
        setIndexedProperty(uint32_t index, Local<Value> value, const AccessorInfo& info) 
        {
            auto    pThis    = _nativeThis<lxvar>(info); 
            lxvar   val      = _marshal(value);

            pThis->at(int(index), val);
            return v8::Undefined();
        }
    };

    Persistent<Function> 
    JavascriptDoc::_addLxVar (void)
    {
        namespace W = wrapper_lxvar;

        Handle<FunctionTemplate> templ( FunctionTemplate::New() );

        // Create an anonymous type which will be used for the lxvar wrapper
        Handle<ObjectTemplate> objInst( templ->InstanceTemplate() );
        objInst->SetInternalFieldCount(2);
        objInst->SetNamedPropertyHandler(W::getNamedProperty, W::setNamedProperty, 0, 0, W::enumerateNamedProperties, External::New(this));
        objInst->SetIndexedPropertyHandler(W::getIndexedProperty, W::setIndexedProperty, 0, 0, W::enumerateIndexedProperties, External::New(this));

        return Persistent<Function>::New( templ->GetFunction() );
    }

    namespace ElementWrapper
    {
        static v8::Handle<v8::Value> 
        get_onUpdate (Local<String> property, const AccessorInfo &info) 
        {
            auto     pContext = _nativeData<JavascriptDoc>(info);
            Element* pThis    = _nativeThis<Element>(info);

            return pThis->getComponent<JavascriptElem>("javascript")->mOnUpdate;
        }

        static void
        set_onUpdate (Local<String> property, Local<Value> value, const AccessorInfo &info) 
        {
            auto     pContext = _nativeData<JavascriptDoc>(info);
            Element* pThis    = _nativeThis<Element>(info);

            Handle<Function> func     = _marshal(value);
            pThis->getComponent<JavascriptElem>("_js")->mOnUpdate = Persistent<Function>::New(func);
            pThis->recomputeFlags();
        }

        static v8::Handle<v8::Value> 
        genericCallback (const v8::Arguments& args)
        {
            Element*    pThis = _nativeThis<Element>(args);
            auto     funcName = _nativeData<const char>(args);
                
            std::vector<lxvar> lxargs;
            lxargs.reserve( args.Length() );
            for (int i = 0; i < args.Length(); ++i)
                lxargs.push_back( _marshal(args[i]) );
                
            pThis->call(std::string(funcName), lxargs);
                
            return Undefined();
        }

        static Handle<Value> 
        get_parentNode (Local<String> property, const AccessorInfo &info) 
        {
            auto      pContext  = _nativeData<JavascriptDoc>(info);
            Element* pThis      = _nativeThis<Element>(info);

            return _marshal(pThis->parent());
        }

        static Handle<Value>
        get_value (Local<String> property, const AccessorInfo &info) 
        {
            auto     pContext = _nativeData<JavascriptDoc>(info);
            Element* pThis    = _nativeThis<Element>(info);
                
            /*
                Is it really worth wrapping the lxvar in a custom object rather than 
                converting it to a v8::Value?  

                Originally, it was *not* converted because the code attempted to allow
                the JS code to directly modify the underlying data (i.e. not a clone());
                but that had problems (namely, the native code being notified about
                property changes it should be aware of).  Since that's no longer being
                pursued - is this wrapper actually adding any value or just more code?
                */
            std::shared_ptr<lxvar> spWrapper(new lxvar(pThis->value().clone()));
            return _marshal(spWrapper);
        }

        static void
        set_value (Local<String> property, Local<Value> value, const AccessorInfo &info) 
        {
            Element* pThis    = _nativeThis<Element>(info);
            lxvar    val      = _marshal(value);

            pThis->value(val); 
        }

        static v8::Handle<v8::Value> 
        removeChild (const v8::Arguments& args)
        {
            Element* pThis = _nativeThis<Element>(args);
            Element* pChild = _marshal(args[0]).pointer<Element>();
                          
            if (!pChild->parent().get())
            {
                lx_debug("Ignoring call to remove orphaned Element.");
                return Undefined();
            }

            if (pChild->parent().get() != pThis)
            {
                lx_warn("Ignoring call to remove Element from incorrect parent.");
                return Undefined();
            }

            lx_check_error(pThis != nullptr);   // Should not be possible given the above conditions
                
            pThis->removeChild(pChild->shared_from_this());
                
            return Undefined();
        }

        static v8::Handle<v8::Value> 
        appendChild (const v8::Arguments& args)
        {
            Element* pThis = _nativeThis<Element>(args);
            Element* pChild = _marshal(args[0]).pointer<Element>();
        
            pThis->append(pChild->shared_from_this());

            return Undefined();
        }

        static v8::Handle<v8::Value> 
        getAttribute (const v8::Arguments& args)
        {
            Element* pThis = _nativeThis<Element>(args);
            std::string name = _marshal(args[0]);
            lxvar value = pThis->attr(name);

            return _marshal(value);
        }

        static v8::Handle<v8::Value> 
        setAttribute (const v8::Arguments& args)
        {
            Element* pThis = _nativeThis<Element>(args);
            std::string name = _marshal(args[0]);
            lxvar value = _marshal(args[1]);
                
            if (value.is_string())
                value = Engine::acquire()->parseAttribute(name, value.as<std::string>());

            pThis->attr(name, value);

            return Undefined();
        }

        /*!
            @todo There's a design problem here: the C++ side of addFunction is adding a member
            function to the class.  However, on the DOM side, it likely makes sense to have
            functions that apply to individual elements (callbacks).
            */
        static v8::Handle<v8::Value>
        addFunction (const v8::Arguments& args)
        {
            Element* pThis      = _nativeThis<Element>(args);
            std::string name    = _marshal(args[0]);
            Handle<Function> func = _marshal(args[1]);
                
            Persistent<Function> mFunc = Persistent<Function>::New(func);

            auto spJElem = pThis->getComponent<JavascriptElem>("javascript");
            spJElem->mCallbacks.insert(std::make_pair(name, mFunc));

            Element::Function wrapper = [=] (ElementPtr spElem, std::vector<lxvar>& args) {
                DocumentPtr spDoc = spElem->document();
                if (spDoc.get())
                {
                    auto spJDoc = spDoc->getComponent<JavascriptDoc>("javascript");
                    auto spJElem = spElem->getComponent<JavascriptElem>("javascript");

                    if (!mFunc.IsEmpty())
                    {
                        Context::Scope context_scope(spJDoc->mContext);
                        HandleScope handle_scope;
                        Handle<Object> recv = _marshal(spElem);
                        
                        Handle<Value> callArgs[8];
                        size_t i;
                        for (i = 0; i < args.size(); ++i)
                        {
                            if (args[i].isHandle())
                            {
                                lx0::ElementPtr* pspElem = args[i].unwrap3<ElementPtr>();
                                if (pspElem)
                                    callArgs[i] = _marshal(*pspElem);
                                else
                                {
                                    lx_warn("Dispatching function call to Javascript with unknown native handle.");
                                    callArgs[i] = Undefined();
                                }
                            }
                            else
                                callArgs[i] = _marshal(args[i]);
                        }
                        
                        mFunc->Call(recv, i, callArgs);
                        args.clear();
                    }
                    else
                        throw lx_error_exception("Callback wrapper set with an empty JS function!");
                }
            };
            pThis->addCallback(name, wrapper);
                
            return Undefined();
        }
    }

    v8::Persistent<v8::Function>
    JavascriptDoc::_addElement (void)
    {
        namespace W = ElementWrapper;

        Handle<FunctionTemplate> templ( FunctionTemplate::New() );

        // Create an anonymous type which will be used for the Element wrapper
        Handle<ObjectTemplate> objInst( templ->InstanceTemplate() );
        objInst->SetInternalFieldCount(2);
        objInst->SetAccessor(String::New("parentNode"),  W::get_parentNode, 0, External::New(this));
        objInst->SetAccessor(String::New("value"),       W::get_value, W::set_value, External::New(this));
        objInst->SetAccessor(String::New("onUpdate"),    W::get_onUpdate, W::set_onUpdate, External::New(this));
        
        // Access the Javascript prototype for the function - i.e. my_func.prototype - 
        // and add the necessary properties and methods.
        //
        Handle<Template> proto_t( templ->PrototypeTemplate() );
        proto_t->Set("getAttribute",  FunctionTemplate::New(W::getAttribute, External::New(this)));
        proto_t->Set("setAttribute",  FunctionTemplate::New(W::setAttribute, External::New(this)));
        proto_t->Set("appendChild", FunctionTemplate::New(W::appendChild, External::New(this)));
        proto_t->Set("removeChild", FunctionTemplate::New(W::removeChild, External::New(this)));

        proto_t->Set("addFunction", FunctionTemplate::New(W::addFunction, External::New(this)));

        // Add any generic functions added by other plug-ins
        //
        ///@todo Remove this workaround: this static vector is used to ensure the pData pointers below
        /// remain valid for the duration of the application run.  Otherwise, the data pointed to by
        /// the External::New() may go stale.
        //
        static std::vector<std::string> funcNames;
        Element::getFunctions(funcNames);
        for (auto it = funcNames.begin(); it != funcNames.end(); ++it)
        {
            void* pData = (void*)it->c_str();
            proto_t->Set(it->c_str(), FunctionTemplate::New(W::genericCallback, External::New(pData)) );
        }

        // Store a persistent reference to the function which will be used to create
        // new object wrappers
        return Persistent<Function>::New( templ->GetFunction() );
    }

    v8::Persistent<v8::Function>
    JavascriptDoc::_addKeyEvent (void)
    {
        struct L
        {
            static Handle<Value> 
            get_keyCode(Local<String> property, const AccessorInfo &info) 
            {
                return Integer::New( _nativeThis<KeyEvent>(info)->keyCode );
            }

            static Handle<Value> 
            get_keyChar(Local<String> property, const AccessorInfo &info) 
            {
                auto* pThis = _nativeThis<KeyEvent>(info);
                char buffer[2] = { pThis->keyChar, 0 };
                return String::New(buffer);
            }
        };

        Handle<FunctionTemplate> templ( FunctionTemplate::New() );

        // Create an anonymous type which will be used for the Element wrapper
        Handle<ObjectTemplate> objInst( templ->InstanceTemplate() );
        objInst->SetInternalFieldCount(2);
        objInst->SetAccessor(String::New("keyCode"),  L::get_keyCode);
        objInst->SetAccessor(String::New("keyChar"),  L::get_keyChar);

        // Store a persistent reference to the function which will be used to create
        // new object wrappers
        return Persistent<Function>::New( templ->GetFunction() );
    }

    void
    JavascriptDoc::_addMath (void)
    {
        Handle<Object> create_javascript_math();
        auto obj = create_javascript_math();
        mContext->Global()->Set(String::New("Math"), obj);
    }
}

namespace lx0
{
    namespace subsystem
    {
        namespace javascript_ns
        {
            Engine::Component* createJavascriptSubsystem()
            {
                return new JsEngineContext(Engine::acquire().get());
            }
        }
    }
}
