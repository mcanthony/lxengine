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

#include <lx0/core.hpp>
#include <lx0/util.hpp>

namespace lx0 { namespace core {

    slot<void (const char*)> slotFatal;
    slot<void (const char*)> slotError;
    slot<void (const char*)> slotWarn;
    slot<void (const char*)> slotLog;
    slot<void (const char*)> slotAssert;
    slot<void (const char*)> slotDebug;

    void 
    lx_assert (bool condition)
    {
        if (!condition)
            *(char*)(void*)(0x0) = 'a';
    }

    void 
    lx_assert (bool condition, const char* format, ...)
    {
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
        if (!condition)
            lx_error("Error condition encountered!");
    }

    void 
    lx_check_error (bool condition, const char* format, ...)
    {
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
        if (!condition)
            lx_fatal("Error condition encountered!");
    }

    void
    lx_fatal (const char* format, ...)
    {
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
        char buffer[512] = "";
        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotWarn(buffer);
    }

    void 
    lx_log (const char* format, ...)
    {
        char buffer[512] = "";
        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotLog(buffer);
    }

    void 
    lx_debug (const char* format, ...)
    {
#ifndef NDEBUG
        char buffer[512] = "";
        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotDebug(buffer);
#endif
    }

}}
