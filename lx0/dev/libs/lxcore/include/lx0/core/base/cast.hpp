//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

namespace lx0 { namespace core {

    namespace detail
    {
        template <typename To, typename From>
        struct cast_is_safe
        {
            enum { value = 0 };
        };
         
        template <typename To, typename From, bool Valid>
        struct cast_imp
        {
        };
        
        template <typename To, typename From>
        struct cast_imp<To,From,false>
        {
            static To cast (From& f)
            {
                static_assert(false, "Not a valid lx::core::cast()");
                throw std::exception();
            }
        };
        
        template <typename To, typename From>
        struct cast_imp<To,From,true>
        {
            static To cast (From& f)
            {
                static_assert(sizeof(To) == sizeof(From), "lx::core::cast between different sized types is not allowed");
                return reinterpret_cast<To>(f);
            }
        };
    }
    
    template <typename To,typename From>
    To cast (From& f)
    {
        return detail::cast_imp<To,From, 
            detail::cast_is_safe<To,From>::value >::cast(f);
    } 


    namespace detail
    {
        template <typename Base, typename To>
        struct lx_cast_imp
        {
        };

        template<typename B>
        struct lx_cast_worker
        {
            lx_cast_worker (B& base) : mBase(base) {}
            B& mBase;

            template <typename T>
            operator T () const 
            { 
                return lx_cast_imp<B,T>().cast(mBase); 
            }
        };
    }

    template <typename B>
    detail::lx_cast_worker<B> 
    lx_cast (B& b) 
    { 
        return detail::lx_cast_worker<B>(b); 
    }

}};

#define _ENABLE_LX_CAST(From,To)                                \
    namespace lx0 { namespace core { namespace detail {         \
        template <>                                             \
        struct lx_cast_imp<From, To>                            \
        {                                                       \
            To& cast(From& base)                                \
            {                                                   \
                return reinterpret_cast<To&>(base);             \
            }                                                   \
        };                                                      \
    }}}
