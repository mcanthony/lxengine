//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010 athile@athile.net (http://www.athile.net)

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

#include <v8/v8.h>

#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/view.hpp>
#include <lx0/mesh.hpp>
#include <lx0/util.hpp>
#include <lx0/v8bind.hpp>

using namespace v8;
using namespace lx0::core::v8bind;
using namespace lx0::util;

//===========================================================================//
//   I M P L E M E N T A T I O N 
//===========================================================================//

namespace lx0 { namespace core { namespace detail {

    using v8::Object;



    //===========================================================================//
    // Local Helpers
    //===========================================================================//


    //-----------------------------------------------------------------------//
    //! Wrap a native object without reference counting
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

            V8::AdjustAmountOfExternalAllocatedMemory(nativeBytes);
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
            static void releaseObj (Persistent<Value> persistentObj, void* pData)
            {
                // Release the native object by deleteing the object holding the
                // shared reference.
                auto pShared = reinterpret_cast<_SharedObjectWrapper*>(pData);
                delete pShared;
        
                // Clear out the object fields.  Technically not necessary, but just in case.
                Local<Object> obj( Object::Cast(*persistentObj) );
                obj->SetInternalField(0, Null());

                // Manually dispose of the PersistentHandle
                persistentObj.Dispose();
                persistentObj.Clear();
            }
        };

        Persistent<Object> obj( Persistent<Object>::New( _wrapObject(ctor, sharedPtr.get(), nativeBytes) ));
        obj.MakeWeak(new _SharedObjectWrapperImp<T>(sharedPtr), L::releaseObj);

        return obj;
    }

    //===========================================================================//
    // Engine Context
    //===========================================================================//

    class JsEngineContext 
        : public lx0::core::v8bind::_V8Context
        , public Engine::Component
    {
    public:
                    JsEngineContext (Engine* pEngine);

        Engine*     mpEngine;
    protected:
        void        _addEngine  ();
    };

    JsEngineContext::JsEngineContext (Engine* pEngine)
        : mpEngine (pEngine)
    {
        _addEngine();
    }

    namespace wrappers_engine
    {
        static v8::Handle<v8::Value> 
        addAttributeParser (const v8::Arguments& args)
        {
            auto                 pThis = lx0::core::v8bind::_nativeThis<JsEngineContext>(args);
            std::string          attr  = lx0::core::v8bind::_marshal(args[0]); 
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
        objInst->SetInternalFieldCount(1);

        Handle<Template> proto_t( templ->PrototypeTemplate() );
        proto_t->Set("debug",               FunctionTemplate::New(W::debug));
        proto_t->Set("addAttributeParser",  FunctionTemplate::New(W::addAttributeParser));

        Handle<Function> ctor( templ->GetFunction() );
        Handle<v8::Object> obj( ctor->NewInstance() );
        obj->SetInternalField(0, External::New(this));

        context->Global()->Set(String::New("engine"), obj);
    }

    //===========================================================================//
    // Document Component
    //===========================================================================//

    class JavascriptDoc : public Document::Component
    {
    public:
                        JavascriptDoc   (DocumentPtr spDocument);
        virtual         ~JavascriptDoc  (void);

        virtual void    onUpdate            (DocumentPtr spDocument);

        void            run (DocumentPtr spDocument, std::string source);
        

        void            _processTimeoutQueue        (void);

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
                callArgs[0] = _wrapObject(this->mKeyEventCtor, &e);
                mWindowOnKeyDown->Call(recv, 1, callArgs);
            }
        };

        spDocument->slotUpdateRun += [&]() {
            this->_processTimeoutQueue();
        };
    }

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

    JavascriptDoc::~JavascriptDoc()
    {
        mElementCtor.Dispose();
        mKeyEventCtor.Dispose();
        mWindowOnKeyDown.Dispose();
        mContext.Dispose();
    }

    
    void
    JavascriptDoc::onUpdate (DocumentPtr spDocument) 
    {
        if (!mOnUpdate.IsEmpty())
        {
            Context::Scope context_scope(mContext);

            HandleScope handle_scope;
            Handle<Object> recv = mContext->Global();
            mOnUpdate->Call(recv, 0, 0);
        }
    }

    void 
    JavascriptDoc::run (DocumentPtr spDocument, std::string text)
    {
        Context::Scope context_scope(mContext);

        HandleScope    handle_scope;
        Handle<String> source = String::New(text.c_str());
        Handle<Script> script = Script::Compile(source);

        // Run the script
        {
            TryCatch trycatch;
            Handle<Value> result = script->Run();
            if (result.IsEmpty()) {  
                Handle<Value> exception = trycatch.Exception();
                
                String::AsciiValue exception_str(exception);
                lx_warn("Javascript Exception: %s\n", *exception_str);
            }
        }
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
                    lx0::util::lx_message_box("Alert", text);
                }
                else
                    lx_error("alert() called from script with too few arguments.");

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
        objInst->SetInternalFieldCount(1);

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
                return _wrapSharedObject(pContext->mElementCtor, spElem, sizeof(Element) * 2);
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
                    return _wrapObject(pContext->mElementCtor, spElem.get());
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
                    results->Set(i, _wrapObject(pContext->mElementCtor, elems[i].get()) );
                
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
        objInst->SetInternalFieldCount(1);
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
            return _wrapSharedObject(pContext->mLxVarCtor, spWrap, 0);
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
            if (pThis->isArray())
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

            if (pThis->isMap())
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
                return _wrapSharedObject(pContext->mLxVarCtor, spWrap, 0);
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

        // Create an anonymous type which will be used for the Element wrapper
        Handle<ObjectTemplate> objInst( templ->InstanceTemplate() );
        objInst->SetInternalFieldCount(1);
        objInst->SetNamedPropertyHandler(W::getNamedProperty, W::setNamedProperty, 0, 0, W::enumerateNamedProperties, External::New(this));
        objInst->SetIndexedPropertyHandler(W::getIndexedProperty, W::setIndexedProperty, 0, 0, W::enumerateIndexedProperties, External::New(this));

        return Persistent<Function>::New( templ->GetFunction() );
    }

    v8::Persistent<v8::Function>
    JavascriptDoc::_addElement (void)
    {
        struct L
        {
            static Handle<Value> 
            get_parentNode (Local<String> property, const AccessorInfo &info) 
            {
                auto      pContext  = _nativeData<JavascriptDoc>(info);
                Element* pThis      = _nativeThis<Element>(info);

                return _wrapObject(pContext->mElementCtor, pThis->parent().get());
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
                return _wrapSharedObject(pContext->mLxVarCtor, spWrapper, 0);
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
                
                if (value.isString())
                    value = Engine::acquire()->parseAttribute(name, value.asString());

                pThis->attr(name, value);

                return Undefined();
            }
        };

        Handle<FunctionTemplate> templ( FunctionTemplate::New() );

        // Create an anonymous type which will be used for the Element wrapper
        Handle<ObjectTemplate> objInst( templ->InstanceTemplate() );
        objInst->SetInternalFieldCount(1);
        objInst->SetAccessor(String::New("parentNode"),  L::get_parentNode, 0, External::New(this));
        objInst->SetAccessor(String::New("value"),       L::get_value, L::set_value, External::New(this));

        // Access the Javascript prototype for the function - i.e. my_func.prototype - 
        // and add the necessary properties and methods.
        //
        Handle<Template> proto_t( templ->PrototypeTemplate() );
        proto_t->Set("getAttribute",  FunctionTemplate::New(L::getAttribute, External::New(this)));
        proto_t->Set("setAttribute",  FunctionTemplate::New(L::setAttribute, External::New(this)));
        proto_t->Set("appendChild", FunctionTemplate::New(L::appendChild, External::New(this)));
        proto_t->Set("removeChild", FunctionTemplate::New(L::removeChild, External::New(this)));

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
        objInst->SetInternalFieldCount(1);
        objInst->SetAccessor(String::New("keyCode"),  L::get_keyCode);
        objInst->SetAccessor(String::New("keyChar"),  L::get_keyChar);

        // Store a persistent reference to the function which will be used to create
        // new object wrappers
        return Persistent<Function>::New( templ->GetFunction() );
    }

    void
    JavascriptDoc::_addMath (void)
    {
        struct Math
        {
            static v8::Handle<v8::Value> random (const v8::Arguments& args)
            {
                lx_check_error(args.Length() == 0);
                auto pThis = _nativeThis<Math>(args); 
        
                float ret = float(rand()) / float(RAND_MAX);

                return _marshal(ret);
            }
        };

        // Create the "Math" function template and add it's methods.
        //
        Handle<FunctionTemplate>    templ( FunctionTemplate::New() );
        Handle<ObjectTemplate>      objInst( templ->InstanceTemplate() );
        objInst->SetInternalFieldCount(1);

        Handle<Template> proto_t( templ->PrototypeTemplate() );
        proto_t->Set("random",  FunctionTemplate::New(Math::random));

        // Create the function and add it to the global object 
        //
        //@todo The native Math object is being leaked
        Handle<Object> obj( templ->GetFunction()->NewInstance() );
        obj->SetInternalField(0, External::New(new Math));
        mContext->Global()->Set(String::New("Math"), obj);
    }

}}}

//===========================================================================//
//   Engine class
//===========================================================================//

namespace lx0 { namespace core { 

    using namespace detail;

     void
     Engine::_attachJavascript (void)
     {
         auto pContext = new JsEngineContext(this);
         this->attachComponent("engineJs", pContext);

         pContext->runFile("media/scripts/engine/attribute_parsers/color.js");
     }

    /*!
        Run a set of Javascript source files together in the same execution context.
     */
    void
    Engine::_runJavascript (DocumentPtr spDocument, std::string source)
    {
        auto ctor = [=]() { return new JavascriptDoc(spDocument); };
        auto spComponent = spDocument->ensureComponent<JavascriptDoc>("javascript", ctor);
        spComponent->run(spDocument, source);
    }

}}
