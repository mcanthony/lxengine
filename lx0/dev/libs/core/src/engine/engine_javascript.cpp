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

#include <v8/v8.h>

#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/mesh.hpp>
#include <lx0/util.hpp>

#include <lx0/v8bind.hpp>

namespace lx0 { namespace core { namespace detail {

    // --------------------------------------------------------------------- //
    //! Local utility class for converting JS <=> native values
    /*
        This class is used to convert from V8 values to primitive types,
        including lxvar. 

        See http://en.wikipedia.org/wiki/Marshalling_(computer_science)
     */
    struct _marshal
    {
        _marshal()                          : mValue( v8::Undefined() ) {}

        _marshal(v8::Handle<v8::Value>&v)   : mValue(v) {}
        _marshal(v8::Handle<v8::Object>&v)  : mValue(v) {}

        _marshal(int i)                     : mValue( v8::Integer::New(i) ) {}
        _marshal(float f)                   : mValue( v8::Number::New(f) ) {}
        _marshal(std::string s)             : mValue( v8::String::New(s.c_str()) ) {}

        operator v8::Handle<v8::Value> ()   { return mValue; }
        operator std::string ()             { return *v8::String::AsciiValue(mValue);  }
        operator int ()                     { return mValue->Int32Value(); }
        
        operator lxvar ()
        {
            using namespace v8;

            if (mValue->IsUndefined())
            {
                return lxvar();
            }
            else if (mValue->IsString())
            {
                return lxvar( std::string( *this ).c_str() );
            }
            else if (mValue->IsArray())
            {
                Local<Array> arr = Array::Cast(*mValue);

                lxvar v;
                for (int i = 0; i < int( arr->Length() ); ++i)
                {
                    Local<Value> e = arr->Get(i);
                    v.push( _marshal(e) );
                }
                return v;
            }
            else if (mValue->IsObject())
            {
                lx_error("Not valid");
            }
            else if (mValue->IsInt32())
            {
                return lxvar( int(*this) );
            }
            else if (mValue->IsNumber())
            {
                return lxvar( float( mValue->NumberValue() ) );
            }
            else if (mValue->IsExternal())
            {
                lx_error("Not valid");
            }
            else if (mValue->IsFunction())
            {
                lx_error("Not valid");
            }
            else
                lx_error("Cannot convert Javascript value to lxvar.");

            lx_error("Unreachable code.");
            return lxvar();
        }

        template <typename T>
        T* pointer ()
        {
            using namespace v8;
            using v8::Object;

            Handle<Object> obj( Handle<Object>::Cast(mValue) );
            lx_check_error(obj->InternalFieldCount() == 1);
            Local<External> wrap = Local<External>::Cast(obj->GetInternalField(0));
        
            T* pNative = reinterpret_cast<T*>( wrap->Value() );
            return pNative;
        }

    protected:
        v8::Handle<v8::Value> mValue;
    };

    //===========================================================================//
    // Local Helpers
    //===========================================================================//

    template <typename NativeType>
    static NativeType*
    _nativeThis (const v8::Arguments& args)
    {
        //
        // Assumes the function was invoked with a this object (i.e. Holder is not null) and that
        // the object was set with exactly one internal field of type T.   It is difficult to
        // verify these assumptions at runtime, so this is a somewhat dangerous function.
        //
        using namespace v8;
        using v8::Object;

        Local<Object> self = args.Holder();

        lx_check_error(self->InternalFieldCount() == 1);
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        
        NativeType* pThis = reinterpret_cast<NativeType*>( wrap->Value() );
        lx_check_error(pThis != nullptr);

        return pThis;
    }

    static v8::Handle<v8::Object>
    _wrapObject (v8::Handle<v8::Function>& ctor, void* pNative)
    {
        v8::Handle<v8::Object> obj = ctor->NewInstance();
        obj->SetInternalField(0, v8::External::New(pNative));

        return obj;
    }
    
    //===========================================================================//
    // Document Component
    //===========================================================================//

    class JavascriptComponent : public Document::Component
    {
    public:
                        JavascriptComponent(DocumentPtr spDocument);
        virtual         ~JavascriptComponent();

        void            run (DocumentPtr spDocument, std::string source);
        
        //@name Add functions/objects to JS context
        //@{
        void                            _addDocument    (DocumentPtr spDocument);
        v8::Persistent<v8::Function>    _addElement     (void);
        void                            _addMath        (void);
        //@}

        v8::Persistent<v8::Context>     mContext;
        v8::Persistent<v8::Function>    mElementCtor;

        std::vector<ElementPtr>         mElements;
    };
    
    JavascriptComponent* s_pActiveContext;

    namespace internal {

        static v8::Handle<v8::Value> print (const v8::Arguments& args)
        {
            using namespace v8;

            std::string name = _marshal(args[0]);
            std::cout << "JS print: " << name << std::endl;

            return Undefined();
        }
    }


    JavascriptComponent::JavascriptComponent (DocumentPtr spDocument)
    {
        using namespace v8;
        using v8::Object;
        using namespace lx0::core::detail::internal;

        // Set up the global template before initiating the Context.   The ObjectTemplate lets
        // FunctionTemplates be added for the free functions.  Context::Global() returns an
        // Object, not an ObjectTemplate - therefore, it limits what can be added after the
        // Context has been created.
        //
        HandleScope handle_scope;
        Handle<ObjectTemplate> global_templ( ObjectTemplate::New() ); 
  
        // Internal debugging methods to make development a little easier.
        //
        global_templ->Set(String::New("__lx_print"), FunctionTemplate::New(print));
        
        mContext = Context::New(0, global_templ);

        Context::Scope context_scope(mContext);
        _addDocument(spDocument);
        mElementCtor = _addElement();
        _addMath();
    }

    JavascriptComponent::~JavascriptComponent()
    {
        using namespace v8;
        using v8::Object;


        mElementCtor.Dispose();

        mContext.Dispose();
    }

    void 
    JavascriptComponent::run (DocumentPtr spDocument, std::string text)
    {
        using namespace v8;
        using v8::Object;

        s_pActiveContext = this;

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

        s_pActiveContext = nullptr;
    }

    void
    JavascriptComponent::_addDocument (DocumentPtr spDocument)
    {
        using namespace v8;
        using v8::Object;

        // Local functions
        struct L
        {
            /*!
                Wraps document.createElement()
             */
            static v8::Handle<v8::Value> 
            createElement (const v8::Arguments& args)
            {
                Document* pThis     = _nativeThis<Document>(args);
                std::string name    = _marshal(args[0]);
        
                ElementPtr spElem   = pThis->createElement(name);
        
                // The Javascript could continue to reference this open.   Keep a reference around to
                // the element for the duration of the V8 context.   (The alternative would be to use
                // V8 Persistent weak pointers that provide a callback on the last reference being
                // removed.)
                //
                s_pActiveContext->mElements.push_back(spElem);
        
                return _wrapObject(s_pActiveContext->mElementCtor, spElem.get());
            }

            /*
                Wrapper on Document::getElementById()
             */
            static v8::Handle<v8::Value> 
            getElementById (const v8::Arguments& args)
            {
                using namespace v8;
                lx_check_error(args.Length() == 1);

                Document* pDoc = _nativeThis<Document>(args); 
                std::string id = _marshal(args[0]);
        
                ElementPtr spElem = pDoc->getElementById(id);

                // The code does not yet gracefully handle a failed search
                if (!spElem)
                    lx_error("Could not find element with id '%s' in the document.", id.c_str());

                return _wrapObject(s_pActiveContext->mElementCtor, spElem.get());
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

        // Access the Javascript prototype for the function - i.e. my_func.prototype - 
        // and add the necessary properties and methods.
        //
        Handle<Template> proto_t( templ->PrototypeTemplate() );
        proto_t->Set("createElement",  FunctionTemplate::New(L::createElement));
        proto_t->Set("getElementById", FunctionTemplate::New(L::getElementById));

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

    v8::Persistent<v8::Function>
    JavascriptComponent::_addElement (void)
    {
        using namespace v8;
        using v8::Object;

        struct L
        {
            static v8::Handle<v8::Value> 
            appendChild (const v8::Arguments& args)
            {
                using namespace v8;
                using v8::Object;

                Element* pThis = _nativeThis<Element>(args);
                Element* pChild = _marshal(args[0]).pointer<Element>();
        
                pThis->append(pChild->shared_from_this());

                return Undefined();
            }

            static v8::Handle<v8::Value> 
            setAttribute (const v8::Arguments& args)
            {
                using namespace v8;
                using v8::Object;

                Element* pThis = _nativeThis<Element>(args);
                std::string name = _marshal(args[0]);
                lxvar value = _marshal(args[1]);

                pThis->attr(name, value);

                return Undefined();
            }
        };

        Handle<FunctionTemplate> templ( FunctionTemplate::New() );

        // Create an anonymous type which will be used for the Element wrapper
        Handle<ObjectTemplate> objInst( templ->InstanceTemplate() );
        objInst->SetInternalFieldCount(1);

        // Access the Javascript prototype for the function - i.e. my_func.prototype - 
        // and add the necessary properties and methods.
        //
        Handle<Template> proto_t( templ->PrototypeTemplate() );
        proto_t->Set("setAttribute",  FunctionTemplate::New(L::setAttribute));
        proto_t->Set("appendChild", FunctionTemplate::New(L::appendChild));

        // Store a persistent reference to the function which will be used to create
        // new object wrappers
        return Persistent<Function>::New( templ->GetFunction() );
    }

    void
    JavascriptComponent::_addMath (void)
    {
        using namespace v8;
        using v8::Object;

        struct Math
        {
            static v8::Handle<v8::Value> random (const v8::Arguments& args)
            {
                lx_check_error(args.Length() == 0);
                auto pThis = _nativeThis<Math>(args); 
                std::string id = _marshal(args[0]);
        
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

    /*!
        Run a set of Javascript source files together in the same execution context.
     */
    void
    Engine::_runJavascript (DocumentPtr spDocument, std::string source)
    {
        auto ctor = [=]() { return new JavascriptComponent(spDocument); };
        auto spComponent = spDocument->ensureComponent<JavascriptComponent>("javascript", ctor);
        spComponent->run(spDocument, source);
    }

}}
