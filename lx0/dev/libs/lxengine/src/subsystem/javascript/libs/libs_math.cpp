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
#include <boost/math/constants/constants.hpp>

#include <v8/v8.h>

#include <glgeom/ext/patterns.hpp>

#include <lx0/engine/engine.hpp>
#include <lx0/engine/document.hpp>
#include <lx0/engine/element.hpp>
#include <lx0/engine/view.hpp>
#include <lx0/engine/mesh.hpp>
#include <lx0/util/misc/util.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>
#include <lx0/subsystem/javascript.hpp>
#include <lx0/subsystem/javascript/v8bind.hpp>

using namespace v8;
using namespace lx0::core;
using namespace lx0::core::v8bind;
using namespace lx0::util;
using namespace lx0;

namespace { 
    
    namespace MathWrappers
    {
        int sign (float a)
        {
            if (a < 0.0f) 
                return -1;
            else if (a > 0.0f)
                return 1;
            else
                return 0;
        }

        Handle<Value> modf (const v8::Arguments& args)
        {
            float v = _marshal(args[0]);
            float i;
            float f = std::modf(v, &i);

            auto arr = v8::Array::New(2);
            arr->Set(0, _marshal(i));
            arr->Set(1, _marshal(f));
            return arr;
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

        float fract (float v)
        {
            float unused;
            return ::modf(v, &unused);
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

        if ((int)generated.size() <= slot) 
            generated.resize(slot + 1); 
        generated[slot] = func;
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


    template <typename RTYPE>
    std::function< Handle<Value>(const Arguments&) >
    method (RTYPE (*fp)(float))
    {
        return [fp](const Arguments& args) -> Handle<Value> {
            return _marshal( (*fp)( _marshal(args[0]) ) );
        };
    }

    std::function< Handle<Value>(const Arguments&) >
    method (float (*fp)(float, float))
    {
        return [fp](const Arguments& args) -> Handle<Value> {
            return _marshal( (*fp)( _marshal(args[0]), _marshal(args[1]) ) );
        };
    }

    template <typename RTYPE, typename T0>
    std::function< Handle<Value>(const Arguments&) >
    method (RTYPE (*fp)(T0, const glm::vec2&))
    {
        return [fp](const Arguments& args) -> Handle<Value> {
            T0 a0 = _marshal(args[0]);
            glm::vec2 a1 = _marshal(args[1]);
            return _marshal( (*fp)(a0, a1) );
        };
    }

    template <typename RTYPE>
    std::function< Handle<Value>(const Arguments&) >
    method (RTYPE (*fp)(const glm::vec2& uv))
    {
        return [fp](const Arguments& args) -> Handle<Value> {
            glm::vec2 v( (float)_marshal(args[0]), (float)_marshal(args[1]) );
            return _marshal( (*fp)(v) );
        };
    }

    template <typename RTYPE>
    std::function< Handle<Value>(const Arguments&) >
    method_allow_array (RTYPE (*fp)(float))
    {
        return [fp](const Arguments& args) -> Handle<Value> 
        {
            if (args[0]->IsArray())
            {
                auto vec = Handle<Array>::Cast(args[0]);
                auto arr = v8::Array::New(vec->Length());
                for (uint32_t index = 0; index < vec->Length(); ++index)
                    arr->Set( index, _marshal( (*fp)(_marshal(vec->Get(index))) ) );
                return arr;
            }
            else
                return _marshal( (*fp)( _marshal(args[0]) ) );
        };
    } 
}

template <int M, int N>
class FunctionTableImp : public FunctionTableImp<M, N-1>
{
public:
    FunctionTableImp()
    {
        mPointers[N-1] = func;
    }

    static v8::Handle<Value> func(const Arguments& args)
    {
        return mFunctions[N-1](args);
    }
};

template<int M>
class FunctionTableImp<M,0>
{
public:
    static v8::InvocationCallback                           mPointers[M];
    static std::function< Handle<Value>(const Arguments&) > mFunctions[M];
};

template <int M> v8::InvocationCallback FunctionTableImp<M,0>::mPointers[M];
template <int M> std::function< Handle<Value>(const Arguments&) > FunctionTableImp<M,0>::mFunctions[M];


template <int N>
class FunctionTable : public FunctionTableImp<N,N>
{
};

template <int N>
class Builder
{
public:
    Builder (Handle<Template>& proto, FunctionTable<N>& table)
        : mProto (&proto)
        , mTable (table)
        , mCount (0)
    {
    }

    void method ( const char* name, std::function<Handle<Value>(const Arguments&)> func )
    {
        lx_check_error(mCount < N);

        mTable.mFunctions[mCount] = func;
        (*mProto)->Set(name, FunctionTemplate::New(mTable.mPointers[mCount]) );
        mCount++;
    }

    Handle<Template>* mProto;
    FunctionTable<N>& mTable;
    int               mCount;
};

Handle<v8::Object> create_javascript_math()
{
    namespace W = MathWrappers;

    // Create the "Math" function template and add it's methods.
    //
    Handle<FunctionTemplate>    templ( FunctionTemplate::New() );
    Handle<ObjectTemplate>      objInst( templ->InstanceTemplate() );
    objInst->SetInternalFieldCount(2);
    //obj->SetInternalField(0, External::New(new Math));

    Handle<Template> proto_t( templ->PrototypeTemplate() );

    FunctionTable<64> table2;
    Builder<64> build (proto_t, table2);

    // Constants
    constant<0>(objInst,    "E",            boost::math::constants::e<float>() );
    constant<1>(objInst,    "PI",           glgeom::pi().value);
    constant<2>(objInst,    "TWO_PI",       2.0f * glgeom::pi().value);
    constant<3>(objInst,    "ROOT_PI",      boost::math::constants::root_pi<float>() );
    constant<4>(objInst,    "ROOT_HALF_PI", boost::math::constants::root_half_pi<float>() );
    constant<5>(objInst,    "ROOT_TWO_PI",  boost::math::constants::root_two_pi<float>() );
    constant<6>(objInst,    "SQRT2",        1.4142135623730951f);
    constant<7>(objInst,    "SQRT1_2",      0.7071067811865476f);
    constant<8>(objInst,    "LN2",          0.6931471805599453f);
    constant<9>(objInst,    "LN10",         2.302585092994046f);
    constant<10>(objInst,   "LOG2E",        1.4426950408889634f);
    constant<11>(objInst,   "LOG10E",       0.4342944819032518f);

    // Trig functions
    build.method("cos",         method_allow_array(std::cosf) );
    build.method("sin",         method_allow_array(std::sinf) );
    build.method("tan",         method_allow_array(std::tanf) );
    build.method("acos",        method_allow_array(std::acosf) );
    build.method("asin",        method_allow_array(std::asinf) );
    build.method("atan",        method_allow_array(std::atanf) );

    // Hyperbolic functions
    build.method("cosh",        method_allow_array(std::coshf) );
    build.method("sinh",        method_allow_array(std::sinhf) );
    build.method("tanh",        method_allow_array(std::tanhf) );

    // Exponential and logarithmic functions
    build.method("exp",          method(std::expf));
    // frexp
    // ldexp
    build.method("log",          method_allow_array(std::logf));
    build.method("log10",        method_allow_array(std::log10f));
    build.method("modf",         W::modf);

    // Power functions
    build.method("pow",          method(&std::powf) );
    build.method("sqrt",         method_allow_array(&std::sqrtf) );

    // Rounding, absolute value, remainder functions
    build.method("abs",          method_allow_array(&std::abs) );
    build.method("sign",         method_allow_array(&W::sign) );
    build.method("ceil",         method_allow_array(&std::ceilf) );
    build.method("floor",        method_allow_array(&std::floorf) );
    build.method("fract",        method_allow_array(W::fract) );

    // Miscellaneous
    build.method("min",          W::min);
    build.method("max",          W::max);
    build.method("noise3d",      W::noise3d);
    build.method("mix",          W::mix);
    build.method("random",       W::random);

    // Patterns
    build.method("checker",      method( glgeom::pattern_checker<float> ) );
    build.method("checker_dim",  method( glgeom::pattern_checker_dim<float> ) );
    build.method("spot_dim",     method( glgeom::pattern_spot_dim<float> ) );
      
    //
    // Instaniate the object
    //
    return templ->GetFunction()->NewInstance();
}
