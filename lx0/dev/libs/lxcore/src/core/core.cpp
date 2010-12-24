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

#include <iostream>
#include <cstdio> 
#include <cstdarg>
#include <exception>
#include <string>

#include <lx0/core/core.hpp>
#include <lx0/core/util/util.hpp>

namespace lx0 { namespace core {

    slot<void (const char*)> slotFatal;
    slot<void (const char*)> slotError;
    slot<void (const char*)> slotWarn;
    slot<void (const char*)> slotLog;
    slot<void (const char*)> slotAssert;
    slot<void (const char*)> slotDebug;


    static bool s_lx_init_called = false;

    void 
    lx_init()
    {
        if (!s_lx_init_called)
        {
            // Define a helper lambda function that returns a function (this effectively 
            // acts as runtime template function).
            auto prefix_print = [](std::string prefix) -> std::function<void(const char*)> {
                return [prefix](const char* s) { std::cout << prefix << s << std::endl; };
            };
            slotDebug   = prefix_print("DBG: ");
            slotLog     = prefix_print("LOG: ");
            slotWarn    = prefix_print("WARN: ");
            slotError   = prefix_print("ERROR: ");
            slotFatal   = prefix_print("FATAL: ");

            s_lx_init_called = true;
        }
    }

    static inline void
    _lx_check_init()
    {
#ifdef _DEBUG
        if (!s_lx_init_called)
        {
            lx_init();
            lx_error("lx_init() has not been called!");
        }
#endif
    }

    void 
    lx_assert (bool condition)
    {
        _lx_check_init();

        if (!condition)
            *(char*)(void*)(0x0) = 'a';
    }

    void 
    lx_assert (bool condition, const char* format, ...)
    {
        _lx_check_init();

        if (!condition)
        {
            char buffer[512] = "";
            va_list args;
            va_start(args, format);
            vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

            *(char*)(void*)(0x0) = 'a';
        }
    }

    void 
    lx_check_error (bool condition)
    {
        _lx_check_init();

        if (!condition)
            lx_error("Error condition encountered!");
    }

    void 
    lx_check_error (bool condition, const char* format, ...)
    {
        _lx_check_init();

        if (!condition)
        {
            char buffer[512] = "";
            va_list args;
            va_start(args, format);
            vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

            lx_error("%s", buffer);
        }
    }

    void 
    lx_check_fatal (bool condition)
    {
        _lx_check_init();

        if (!condition)
            lx_fatal("Error condition encountered!");
    }

    void 
    lx_fatal  (void)
    {
        lx_fatal("Unknown fatal error!");
    }

    void
    lx_fatal (const char* format, ...)
    {
        _lx_check_init();

        char buffer[512] = "";
        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotFatal(buffer);

        std::string err("lx_fatal (save all data and exit).\n");
        throw std::exception((err + buffer).c_str());
    }

    void
    lx_error (const char* format, ...)
    {
        _lx_check_init();

        char buffer[512] = "";
        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotError(buffer);

#if !defined(NDEBUG) && defined(_MSC_VER)      
        lx0::util::lx_message_box("LxEngine Error", buffer);
        *(int*)0 = 0;
#endif

        std::string err("lx_error (re-throw if error is non-recoverable).\n");
        throw std::exception((err + buffer).c_str());
    }

    void
    lx_warn (const char* format, ...)
    {
        _lx_check_init();

        char buffer[512] = "";
        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotWarn(buffer);
    }

    void 
    lx_log (const char* format, ...)
    {
        _lx_check_init();

        char buffer[512] = "";
        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotLog(buffer);
    }

    void 
    lx_debug (const char* format, ...)
    {
        _lx_check_init();

#ifndef NDEBUG
        char buffer[512] = "";
        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotDebug(buffer);
#endif
    }

}}
