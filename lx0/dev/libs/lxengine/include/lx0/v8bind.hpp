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

#include <lx0/core/detail/forward_decls.hpp>
#include <lx0/core/core.hpp>
#include <lx0/core/data/lxvar.hpp>

namespace lx0 { namespace core { namespace v8bind
{
    // --------------------------------------------------------------------- //
    //! Utility class for converting JS objects to and from native C++ types
    /*
        This class is used to convert from V8 values to primitive types,
        including lxvar (for nested data structures).

        See http://en.wikipedia.org/wiki/Marshalling_(computer_science)

        The basic design premise of the class is quite simple:
        
        # Overload the constructor for each supported JS object type and
          native C++ type.
        # Store the internal value as a V8 Handle<Value>
        # Overload the implicit cast operator to convert the Handle<Value>
          to other types

        Since this is an internal helper class designed for convenience, it makes 
        the assumption that the caller is careful to use the implicit cast 
        operator correctly: in other words, it does not do type conversion
        checks for the caller - it assumes the caller has already done that
        (to do otherwise would seriously hamper the convenience factor of
        this class).

        The conversions for basic types are generally quite simple and merely
        involve looking up the correct constructor and method calls in the V8
        documentation for conversions.

        The conversions for lxvar and native object types are the only 
        non-trivial operations.

        The native object conversion is not complex, but makes unchecked
        assumptions about the native object type (i.e. relying on the caller
        to enforce proper type-checking prior to the call) as well as the 
        assumption that the object is stored in the first "InternalField" of 
        the V8 object (which is always true in LxEngine, but not necessarily
        true for all possible uses of V8).  Therefore, the native object 
        conversion is not a robust, generalized mechanism, but rather a 
        tailored one for LxEngine's use.
       
        The lxvar object conversion is also not complex, but uses LxEngine's
        lxvar class - which, of course, is therefore intrinsically bound to
        LxEngine's lxvar API.   The lxvar is a class designed to hold multiple
        data types based largely off JSON and Javascript variables.  It can
        hold ints, floats, strings, arrays, and maps.  Therefore there is 
        largely a 1:1 mapping between a V8 object and an lxvar, *but* this 
        relies on the lxvar data type so it is not by any means generalizable
        outside of LxEngine (without losing the functionality it represents).
     */
    class _marshal
    {
    public:
        _marshal()                          : mValue( v8::Undefined() ) {}

        _marshal(v8::Handle<v8::Value>&v)   : mValue(v) {}
        _marshal(v8::Handle<v8::Object>&v)  : mValue(v) {}
        _marshal(v8::Handle<v8::String>&v)  : mValue(v) {}

        _marshal(bool b)                    : mValue( v8::Boolean::New(b) ) {}
        _marshal(int i)                     : mValue( v8::Integer::New(i) ) {}
        _marshal(float f)                   : mValue( v8::Number::New(f) ) {}
        _marshal(const char* s)             : mValue( v8::String::New(s) ) { }
        _marshal(std::string s)             : mValue( v8::String::New(s.c_str()) ) {}
        _marshal(lxvar v);


        operator v8::Handle<v8::Value> ()   { return mValue; }
        operator v8::Handle<v8::Function> (){ return v8::Handle<v8::Function>::Cast(mValue); }
        operator std::string ()             
        {
            lx_check_error( mValue->IsString() );
            return *v8::String::AsciiValue(mValue);  
        }
        operator int ()                     { return mValue->Int32Value(); }
        operator float ()                   { return float( mValue->NumberValue() ); }
        operator lxvar ();

        template <typename T>
        T* pointer ()
        {
            using namespace v8;
            using v8::Object;

            // Assume the native object is stored in "internal field" index 0 of
            // the V8 object.  This is the convention in LxEngine, but is not 
            // necessarily true in all uses of V8.
            Handle<Object> obj( Handle<Object>::Cast(mValue) );
            lx_check_error(obj->InternalFieldCount() == 1);
            Local<External> wrap = Local<External>::Cast(obj->GetInternalField(0));
        
            // Assume the caller has done proper type checking and simply cast
            // the data type to the desired pointer type.
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
        lx_check_error(!wrap.IsEmpty(), 
            "Internal field of Javascript object is null.  Is this a valid wrapped object?");
        
        NativeType* pThis = reinterpret_cast<NativeType*>( wrap->Value() );
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
