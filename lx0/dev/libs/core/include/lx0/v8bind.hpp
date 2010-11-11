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


    template <typename NativeType, typename Source>
    static NativeType*
    _nativeThisImp (const Source& args)
    {
        using v8::Object;

        //
        // Assumes the function was invoked with a this object (i.e. Holder is not null) and that
        // the object was set with exactly one internal field of type T.   It is difficult to
        // verify these assumptions at runtime, so this is a somewhat dangerous function.
        //
        Local<Object> self = args.Holder();

        lx_check_error(self->InternalFieldCount() == 1);
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        
        NativeType* pThis = reinterpret_cast<NativeType*>( wrap->Value() );
        lx_check_error(pThis != nullptr);

        return pThis;
    }

    /*!
        Utility method for extracing the "this" pointer for a JS method call, assuming
        the JS object has been set up with the native pointer in internal field 0.
     */
    template <typename NativeType>
    static NativeType*
    _nativeThis (const v8::Arguments& args)
    {
        return _nativeThisImp<NativeType,Arguments>(args);
    }

    /*!
        Utility method for extracing the "this" pointer for a JS method call, assuming
        the JS object has been set up with the native pointer in internal field 0.
     */
    template <typename NativeType>
    static NativeType*
    _nativeThis (const v8::AccessorInfo& info)
    {
        return _nativeThisImp<NativeType,AccessorInfo>(info);
    }

    template <typename NativeType>
    static NativeType*
    _nativeData (const v8::Arguments& args)
    {
        return reinterpret_cast<NativeType*>( External::Unwrap(args.Data()) );
    }

    template <typename NativeType>
    static NativeType*
    _nativeData (const v8::AccessorInfo& info)
    {
        return reinterpret_cast<NativeType*>( External::Unwrap(info.Data()) );
    }

    //===========================================================================//
    //!
    /*!
        Simple wrapper on v8::Context with a few utility methods for convenience.
     */
    class _V8Context
    {
    public:
                    _V8Context();
                    ~_V8Context();

        void        runFile         (const char* filename);

        v8::Persistent<v8::Context> context;
    };

}}}

namespace lx0 { namespace core 
{
}}
