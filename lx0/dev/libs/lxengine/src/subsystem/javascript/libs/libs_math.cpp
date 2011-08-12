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
#include "../v8bind.hpp"

using namespace v8;
using namespace lx0::core;
using namespace lx0::core::v8bind;
using namespace lx0::util;
using namespace lx0;

namespace { 
    
    namespace MathWrappers
    {
        Handle<Value> sin (const v8::Arguments& args)
        {
            float a = _marshal(args[0]);
            return _marshal( ::sin(a) );
        }

        Handle<Value> cos (const v8::Arguments& args)
        {
            float a = _marshal(args[0]);
            return _marshal( ::cos(a) );
        }

        Handle<Value> abs (const v8::Arguments& args)
        {
            float a = _marshal(args[0]);
            return _marshal( fabs(a) );
        }

        Handle<Value> sign (const v8::Arguments& args)
        {
            float a = _marshal(args[0]);
            int s;
            if (a < 0.0f) 
                s = -1;
            else if (a > 0.0f)
                s = 1;
            else
                s = 0;
            return _marshal(s);
        }

        Handle<Value> pow (const Arguments& args)
        {
            return _marshal( std::pow( (float)_marshal(args[0]), (float)_marshal(args[1]) ) );
        }

        Handle<Value> floor (const v8::Arguments& args)
        {
            return _marshal( std::floor( (float)_marshal(args[0]) ) );
        }

        Handle<Value> ceil (const v8::Arguments& args)
        {
            return _marshal( std::ceil( (float)_marshal(args[0])));
        }

        Handle<Value> noise3d (const v8::Arguments& args)
        {
            glm::vec3 v;
            if (args.Length() == 3)
            {
                v.x = _marshal(args[0]);
                v.y = _marshal(args[1]);
                v.z = _marshal(args[2]);
            }
            else
                v = _marshal(args[0]);

            return _marshal( lx0::noise3d_perlin(v) );
        }

        Handle<Value> random (const v8::Arguments& args)
        {
            float ret = 0.0f;
            if (args.Length() == 0)
            {
                ret = float(rand()) / float(RAND_MAX);
            }
            else if (args.Length() == 2)
            {
                float mn = _marshal(args[0]);
                float mx = _marshal(args[1]);
                ret = (mx - mn) * (float(rand()) / float(RAND_MAX)) + mn;
            }
            else
                lx_warn("Wrong number of arguments passed to Math.random().  Expects either 0 or 2 arguments.");

            return _marshal(ret);
        }

        Handle<Value> fract (const Arguments& args)
        {
            float v = _marshal(args[0]);
            
            float unused;
            float f = modf(v, &unused);
            
            return _marshal(f);
        }

        Handle<Value> min (const Arguments& args)
        {
            float m = _marshal(args[0]);
            for (int i = 1; i < args.Length(); ++i)
            {
                float n = _marshal(args[i]);
                m = std::min(m, n);
            }
            return _marshal(m);
        }

        Handle<Value> max (const Arguments& args)
        {
            float m = _marshal(args[0]);
            for (int i = 1; i < args.Length(); ++i)
            {
                float n = _marshal(args[i]);
                m = std::max(m, n);
            }
            return _marshal(m);
        }

        Handle<Value> mix (const Arguments& args)
        {
            if (args[0]->IsArray())
            {
                glm::vec3 a = _marshal(args[0]);
                glm::vec3 b = _marshal(args[1]);
                float t = _marshal(args[2]);
                return _marshal( glm::mix(a, b, t) );
            }
            else
            {
                float a = _marshal(args[0]);
                float b = _marshal(args[1]);
                float t = _marshal(args[2]);
                return _marshal( glm::mix(a, b, t) );
            }
        }

        Handle<Value> PI (Local< String > property, const AccessorInfo &info)
        {
            return _marshal( glgeom::pi().value );
        }
    }
}

// func object
// function pointer

namespace detail2
{
    typedef std::function<v8::Handle<Value> (Local<String>, const AccessorInfo&)> Function2;
    static std::deque<Function2> generated;
    static std::deque< std::function< Handle<Value>(const Arguments& args) > > methods;

    void
    create_function (int slot, float value)
    {    
        //
        // Generate the std::function<> via a lambda using the input argument, and then
        // store it in fixed global offset in memory.  The fixed offset allows us
        // to generate a function pointer to the lambda at compile-time.
        //
        Function2 func = [value](Local<String>, const AccessorInfo&) -> Handle<Value> {
            return _marshal( value );
        };

        if (generated.size() <= slot) 
            generated.resize(slot + 1); 
        generated[slot] = func;
    }

    void
    create_method (int slot, float (*fp)(float))
    {
        auto func = [fp](const Arguments& args) -> Handle<Value> {
            return _marshal( (*fp)( _marshal(args[0]) ) );
        };

        if (methods.size() <= slot) 
            methods.resize(slot + 1); 
        methods[slot] = func;
    }

    //
    // Generate a static function that references the array of std::functions
    //
    template <int N>
    v8::Handle<Value> 
    template_function (Local<String> prop, const AccessorInfo& info)
    {
        return generated[N](prop, info);
    }

    template <int SLOT>
    v8::Handle<Value>
    template_method (const Arguments& args)
    {
        return methods[SLOT](args);
    }
}

template <int N>
v8::AccessorGetter generate_constant_function (float f)
{
    detail2::create_function(N, f);
    return &detail2::template_function<N>;
}

namespace
{
    template <int SLOT>
    void
    constant (Handle<ObjectTemplate>& objInst, const char* name, float value)
    {
        objInst->SetAccessor( String::New(name), generate_constant_function<SLOT>(value) );
    }

    template <int SLOT>
    void
    method2(Handle<Template>& proto_t, const char* name, float (*fp)(float) )
    {
        detail2::create_method(SLOT, fp);
        proto_t->Set(name, FunctionTemplate::New( &detail2::template_method<SLOT> ) );
    }
}

Handle<v8::Object> create_javascript_math()
{
    namespace W = MathWrappers;

    // Create the "Math" function template and add it's methods.
    //
    Handle<FunctionTemplate>    templ( FunctionTemplate::New() );
    Handle<ObjectTemplate>      objInst( templ->InstanceTemplate() );
    objInst->SetInternalFieldCount(1);

    Handle<Template> proto_t( templ->PrototypeTemplate() );
    auto method = [&proto_t] (const char* name, InvocationCallback cb) {
        proto_t->Set(name, FunctionTemplate::New(cb));
    };

    // Constants
    constant<0>(objInst,    "E",       2.7182818284590452f);
    constant<1>(objInst,    "PI",      glgeom::pi().value);
    constant<2>(objInst,    "SQRT2",   1.4142135623730951f);
    constant<3>(objInst,    "SQRT1_2", 0.7071067811865476f);
    constant<4>(objInst,    "LN2",     0.6931471805599453f);
    constant<5>(objInst,    "LN10",    2.302585092994046f);
    constant<6>(objInst,    "LOG2E",   1.4426950408889634f);
    constant<7>(objInst,    "LOG10E",  0.4342944819032518f);

    // Trig functions
    method2<0>(proto_t,     "cos",      &std::cosf );
    method2<1>(proto_t,     "sin",      &std::sinf );
    method2<2>(proto_t,     "tan",      &std::tanf );
    method2<3>(proto_t,     "acos",     &std::acosf );
    method2<4>(proto_t,     "asin",     &std::asinf );
    method2<5>(proto_t,     "atan",     &std::atanf );

    // Hyperbolic functions
    // cosh
    // sinh
    // tanh

    // Exponential and logarithmic functions
    // exp
    // frexp
    // ldexp
    // log
    // log10
    // modf

    // Power functions
    method("pow",       W::pow);
    //method("sqrt",    

    // Rounding, absolute value, remainder functions
    method("abs",       W::abs);
    method("sign",      W::sign);
    method("ceil",      W::ceil);
    method("floor",     W::floor);
    method("fract",     W::fract);

    // Miscellaneous
    method("min",       W::min);
    method("max",       W::max);
    method("noise3d",   W::noise3d);
    method("mix",       W::mix);
    method("random",    W::random);

    // Create the function and add it to the global object 
    //
    //@todo The native Math object is being leaked
    Handle<v8::Object> obj( templ->GetFunction()->NewInstance() );
    //obj->SetInternalField(0, External::New(new Math));

    return obj;
}
