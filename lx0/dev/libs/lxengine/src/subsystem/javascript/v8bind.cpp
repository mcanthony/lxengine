//===========================================================================//
/*
                                   LxEngine

    LICENSE

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

#include <lx0/util/misc/util.hpp>
#include "v8bind.hpp"

using namespace lx0::core;
using namespace v8;
using v8::Object;

namespace lx0 { namespace core { namespace v8bind
{

    _marshal::_marshal (lxvar v)
    {
        if (v.is_undefined())
            mValue = v8::Undefined();
        else if (v.is_string())
            *this = _marshal(v.as<std::string>());
        else if (v.is_float())
            *this = _marshal(v.as<float>());
        else if (v.is_int())
            *this = _marshal(v.as<int>());
        else if (v.is_array())
        {
            Handle<Array> arr = Array::New(v.size());
            for (int i = 0; i < v.size(); i++)
                arr->Set(uint32_t(i), _marshal(v.at(i)));
            mValue = arr;
        }
        else
            lx_error("Not implemented");
    }

    //---------------------------------------------------------------------------//
    //!
    /*!
     */
    _marshal::operator lxvar ()
    {
        if (mValue->IsUndefined())
        {
            return lxvar();
        }
        else if (mValue->IsExternal())
        {
            lx_error("Not valid!  Cannot automatically marshal a native External value.");
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
            v8::Local<v8::Object> obj = v8::Object::Cast(*mValue);
            v8::Local<v8::Array> props = obj->GetPropertyNames();

            lxvar v;
            if (props->Length() > 0)
            {
                // Below seems like a very weak test to determine if the value is 
                // actually an array not a map - but the question is: how is this
                // supposed to be handled?  mValue->IsArray() is false for a 
                // JS-wrapped lxvar that is an array.
                //
                if (props->Get(0)->IsInt32())
                {
                    v = lxvar::array();
                    for (int i = 0; i < int( props->Length() ); ++i)
                    {
                        lxvar value = _marshal( obj->Get( props->Get(i) ) );
                        v.push(value);
                    }
                }
                else
                {
                    v = lxvar::map();
                    for (int i = 0; i < int( props->Length() ); ++i)
                    {
                        std::string name = _marshal( props->Get(i) );
                        lxvar       value = _marshal( obj->Get( props->Get(i) ) );
                        v.insert(name.c_str(), value);
                    }
                }
            }
            return v;
        }
        else if (mValue->IsInt32())
        {
            return lxvar( int(*this) );
        }
        else if (mValue->IsNumber())
        {
            return lxvar( float( mValue->NumberValue() ) );
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

    //===========================================================================//

    _V8Context::_V8Context ()
    {
        HandleScope handle_scope;
        Handle<ObjectTemplate> global_templ = ObjectTemplate::New(); 

        context = Context::New(0, global_templ);
    }

    _V8Context::~_V8Context ()
    {
        context.Dispose();
    }

    void
    _V8Context::runFile (const char* filename)
    {
        lx_debug("Running JS file '%s'", filename);

        try
        {
            std::string text = lx0::string_from_file(filename);
        
            Context::Scope context_scope(context);
        
            HandleScope handle_scope;
            Handle<String> source = String::New(text.c_str());
            Handle<Script> script = Script::Compile(source);
 
            Handle<Value> result = script->Run();
        }
        catch (std::exception& e)
        {
            lx_error("Exception attempting run javascript script!");
            throw e;
        }
    }

}}}