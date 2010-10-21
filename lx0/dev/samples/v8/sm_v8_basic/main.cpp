// Adapted directly from: http://code.google.com/apis/v8/get_started.html

#include <iostream>
#include <functional>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <v8/v8.h>

#include <lx0/util.hpp>

using namespace v8;

//
// C++ code
//
int objectCount = 0;
class LxQuerySet
{
public:
    LxQuerySet() { objectCount++; }
    ~LxQuerySet() { objectCount--; }

    void append (std::string s)
    {
        std::cout << "append called: " << s << std::endl;
    }
};

LxQuerySet* LxQuery (std::string s)
{
    std::cout << "LxQuery called: " << s << std::endl;
    return new LxQuerySet;
}

namespace lx0 { namespace v8_bind {

    class V8Bind
    {
    public:

        template <typename T>
        void            addFunction (Handle<ObjectTemplate>& global, std::string name);

        template <typename T>
        void            addClass    (std::string name);
        template <typename Invoker>
        void            addMethod   (std::string className, std::string method);

        Handle<Object>  newObject   (std::string name);
        Handle<Object>  wrapObject  (std::string name, void* pObject);

    protected:
        struct Class
        {
            Handle<FunctionTemplate>    mTemplate;
            std::function<void*()>      mCtor;
        };
        std::map<std::string, Class> mClasses;
    };

    V8Bind gV8Bind;

    /*!
        The entire purpose of this template class is to minimize amount of code necessary
        to wrap a C++ method as a JS accessible method.
     */
    template <typename NativeType>
    class MethodWrapper
    {
    public:
        template <typename Self>
        static Handle<Value> 
        invoke (const Arguments& args)
        {
            // Access the native object
            Native* ptr;
            Local<Object> self = args.Holder();
            if (self->InternalFieldCount() == 1)
            {
                Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
                ptr = reinterpret_cast<Native*>( wrap->Value() );
            }
            else
                ptr = 0;
        
            // Invoke "this" object
            Self wrapper;
            wrapper.invokeSelf(args, ptr);

            return wrapper.returnValue();
        }

    protected:
        typedef NativeType                Native;

        struct Any
        {
            Any() : mValue( v8::Undefined() ) {}
            Any(Handle<Value>&v) : mValue(v) {}
            Any(Handle<Object>&v) : mValue(v) {}
            Any(std::string s) 
            {
                mValue = String::New(s.c_str());
            }

            Handle<Value> mValue;

            Handle<Value> handle() { return mValue; }

            operator std::string () 
            {
                String::AsciiValue value (mValue);
                return *value;
            }
        };

        Handle<Value> returnValue ()
        {
            return mReturn.handle();
        }

        Handle<Object> wrap (std::string name, void* pObject)
        {
            return gV8Bind.wrapObject(name, pObject);
        }

        void invokeSelf (const Arguments& args, Native* pNative)
        {
            // Copy the native object to a member for easier access in the derived class
            mpObj = pNative;

            // Copy the arguments to the Any class, so they can be implicitly converted
            // to common types, again for easier use in the derived class
            mA.reserve(args.Length());
            for (int i = 0; i < args.Length(); ++i)
                mA.push_back(args[i]);

            // Call the derived class
            imp();
        }

        virtual void imp() = 0;

        Native*          mpObj;
        std::vector<Any> mA;
        Any              mReturn;
    };

    /*!
        This is a quick hack for a FunctionWrapper.  It essentially creates an unused
        "int" object so that MethodWrapper can be reused.
     */
    class FunctionWrapper : public MethodWrapper<int>
    {
    };

    template <typename T>
    void V8Bind::addFunction (Handle<ObjectTemplate>& global, std::string name)
    {
        global->Set(String::New(name.c_str()), FunctionTemplate::New(T::invoke<T>));
    }

    template <typename T>
    void V8Bind::addClass(std::string name)
    {
        // Create the "class" (which is a function in Javascript's prototype-based inheritance scheme)
        Handle<FunctionTemplate> templ = FunctionTemplate::New();
        templ->SetClassName(String::New(name.c_str()));

        // Save space to associate any objects with a native C++ object
        Handle<ObjectTemplate> inst = templ->InstanceTemplate();
        inst->SetInternalFieldCount(1);

        // Store the JS template and a constructor for the type in the class map
        Class c;
        c.mCtor = []() -> void* { return new T; };
        c.mTemplate = templ;
        mClasses.insert(std::make_pair(name, c));
    }

    template <typename Invoker>
    void V8Bind::addMethod (std::string className, std::string method)
    {
        Class& klass = mClasses.find(className)->second;

        // Set up the prototype object, which is cloned when a new object is created
        // (i.e. think the Prototype design pattern)
        Handle<ObjectTemplate> proto = klass.mTemplate->PrototypeTemplate();
        proto->Set(String::New(method.c_str()), FunctionTemplate::New(Invoker::invoke<Invoker>));
    }

    Handle<Object>
    V8Bind::wrapObject (std::string name, void* pObject)
    {
        Class& klass = mClasses.find(name)->second;
        Handle<Function> ctor = klass.mTemplate->GetFunction();
        Handle<Object> obj = ctor->NewInstance();
        obj->SetInternalField(0, External::New(pObject));

        return obj;
    }

    /*!
        @todo This leaks memory.  The native object is allocated here, but there's no
            notification mechanism to let the engine know that the script no longer
            is using the native object.
     */
    Handle<Object>
    V8Bind::newObject (std::string name)
    {
        Class& klass = mClasses.find(name)->second;
        return wrapObject(name, klass.mCtor());
    }
}}

using lx0::v8_bind::gV8Bind;

int 
main (int argc, char** argv)
{
    HandleScope handle_scope;
    Handle<ObjectTemplate> global = ObjectTemplate::New();

    struct InvokeLxQuery : public lx0::v8_bind::FunctionWrapper {
        void imp() { mReturn = wrap("LxQuerySet", LxQuery(mA[0])); }
    };
    gV8Bind.addFunction<InvokeLxQuery>(global, "$");

    gV8Bind.addClass<LxQuerySet>("LxQuerySet");
    struct InvokeAppend : public lx0::v8_bind::MethodWrapper<LxQuerySet> { 
        void imp () { mpObj->append(mA[0]); }
    };
    gV8Bind.addMethod<InvokeAppend>("LxQuerySet", "append");
    

    Persistent<Context> context = Context::New(0, global);
  
    Context::Scope context_scope(context);
    Handle<String> source = String::New(lx0::util::lx_file_to_string("script.js").c_str());
    Handle<Script> script = Script::Compile(source);

    Handle<Value> result = script->Run();

    context.Dispose();

    return 0;
}
