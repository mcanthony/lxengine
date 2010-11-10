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
//   I M P O R T A N T   N O T E :
//===========================================================================//
/*
    The code in this file is not currently used and may possibly be 
    deprecated.  It will be deemed at a later date whether or not V8 helper
    bindings are necessary or if the V8 APIs can simply be used directly.
 */
//===========================================================================//

#pragma once

#include <memory>
#include <deque>
#include <string>
#include <vector>
#include <map>
#include <functional>

#include <v8/v8.h>

#include <lx0/detail/forward_decls.hpp>
#include <lx0/core.hpp>
#include <lx0/lxvar.hpp>

namespace lx0 { namespace core { namespace v8bind
{
    // --------------------------------------------------------------------- //
    //! Utility class for converting JS <=> native values
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
        operator v8::Handle<v8::Function> (){ return v8::Handle<v8::Function>::Cast(mValue); }
        operator std::string ()             { return *v8::String::AsciiValue(mValue);  }
        operator int ()                     { return mValue->Int32Value(); }
        
        _marshal (lxvar v)
        {
            if (v.isUndefined())
                mValue = v8::Undefined();
            else if (v.isString())
                *this = _marshal(v.asString());
            else if (v.isFloat())
                *this = _marshal(v.asFloat());
            else if (v.isInt())
                *this = _marshal(v.asInt());
            else
                lx_error("Not implemented");
        }

        operator lxvar ()
        {
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
                v8::Local<v8::Array> arr = v8::Array::Cast(*mValue);

                lxvar v;
                for (int i = 0; i < int( arr->Length() ); ++i)
                {
                    v8::Local<v8::Value> e = arr->Get(i);
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
}}}

namespace lx0 { namespace core 
{
}}

namespace lx0 { namespace v8_bind {

    class V8Bind
    {
    public:

        template <typename T>
        void            addFunction (v8::Handle<v8::ObjectTemplate>& global, std::string name);

        template <typename T>
        void            addClass    (std::string name);
        template <typename Invoker>
        void            addMethod   (std::string className, std::string method);

        v8::Handle<v8::Object>  newObject   (std::string name);
        v8::Handle<v8::Object>  wrapObject  (std::string name, void* pObject);

    protected:
        struct Class
        {
            v8::Handle<v8::FunctionTemplate>    mTemplate;
            std::function<void*()>      mCtor;
        };
        std::map<std::string, Class> mClasses;
    };

    ///@todo Change me to a singleton owned by the Engine.
    extern V8Bind gV8Bind;

    /*!
        The entire purpose of this template class is to minimize amount of code necessary
        to wrap a C++ method as a JS accessible method.
     */
    template <typename NativeType>
    class MethodWrapper
    {
    public:
        template <typename Self>
        static v8::Handle<v8::Value> 
        invoke (const v8::Arguments& args)
        {
            // Access the native object
            Native* ptr;
            v8::Local<v8::Object> self = args.Holder();
            if (self->InternalFieldCount() == 1)
            {
                v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
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
            Any(v8::Handle<v8::Value>&v) : mValue(v) {}
            Any(v8::Handle<v8::Object>&v) : mValue(v) {}
            Any(std::string s) 
            {
                mValue = v8::String::New(s.c_str());
            }

            v8::Handle<v8::Value> mValue;

            v8::Handle<v8::Value> handle() { return mValue; }

            operator std::string () 
            {
                v8::String::AsciiValue value (mValue);
                return *value;
            }
        };

        v8::Handle<v8::Value> returnValue ()
        {
            return mReturn.handle();
        }

        v8::Handle<v8::Object> wrap (std::string name, void* pObject)
        {
            return gV8Bind.wrapObject(name, pObject);
        }

        void invokeSelf (const v8::Arguments& args, Native* pNative)
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
    void V8Bind::addFunction (v8::Handle<v8::ObjectTemplate>& global, std::string name)
    {
        global->Set(v8::String::New(name.c_str()), v8::FunctionTemplate::New(T::invoke<T>));
    }

    template <typename T>
    void V8Bind::addClass(std::string name)
    {
        // Create the "class" (which is a function in Javascript's prototype-based inheritance scheme)
        v8::Handle<v8::FunctionTemplate> templ = v8::FunctionTemplate::New();
        templ->SetClassName(v8::String::New(name.c_str()));

        // Save space to associate any objects with a native C++ object
        v8::Handle<v8::ObjectTemplate> inst = templ->InstanceTemplate();
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
        v8::Handle<ObjectTemplate> proto = klass.mTemplate->PrototypeTemplate();
        proto->Set(v8::String::New(method.c_str()), v8::FunctionTemplate::New(Invoker::invoke<Invoker>));
    }

}}
