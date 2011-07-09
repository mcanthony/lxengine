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
#include <fstream>

#include <lx0/core/init/init.hpp>
#include <lx0/core/log/log.hpp>
#include <lx0/core/slot/slot.hpp>
#include <lx0/util/misc/util.hpp>

namespace lx0 { namespace core { namespace log_ns {

    lx0::slot<void (const char*)> slotFatal;
    lx0::slot<void (const char*)> slotError;
    lx0::slot<void (const char*)> slotWarn;
    lx0::slot<void (const char*)> slotLog;
    lx0::slot<void (const char*)> slotAssert;
    lx0::slot<void (const char*)> slotDebug;

    std::ofstream s_log;
    int s_log_count = 0;

    static struct _autoclose
    {
        ~_autoclose() 
        { 
            s_log << "</ul></body></html>" << std::endl;
            s_log.close();
        }
    } __autoclose;

    // Internal function to ensure initialization has occured.
    /*
        The intended behavior is:

        (a) do nothing in production code - the developer is intended to properly
        initialize the system.

        (b) in debug code, cause an ignorable error.  This is done by initilizing
        for the developer (so they can continue to debug the application), but 
        causing an error so that the initilization problem will be fixed.
     */
    static inline void
    _lx_check_init()
    {
#ifdef _DEBUG
        if (!lx0::_lx_init_called())
        {
            lx_init();
            lx_error("lx_init() has not been called!");
        }
#endif
    }

    /*!
        Check that a condition is true.   If it is not, cause an error.
     */
    void 
    lx_assert (bool condition)
    {
        _lx_check_init();

        if (!condition)
            lx_break_if_debugging();
    }

    /*!
        Check that a condition is true.   If it is not, cause an error.
        Provides an string message with additional information as well.
     */
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

            lx_break_if_debugging();
        }
    }

    /*!
        Similar to lx_assert() but will cause lx_error() to be called if the
        condition fails in *either* production or debug code.
     */
    void 
    lx_check_error (bool condition)
    {
        _lx_check_init();

        if (!condition)
            lx_error("Error condition encountered!");
    }

    /*!
        Overload of lx_check_error that provides an additional string of
        information.
     */
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

    /*!
        Similar to lx_check_error() but will cause lx_fatal() to be called if the
        condition fails in *either* production or debug code.
     */
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

    /*!
        Reserved for unrecoverable errors.  Throws a lx0::fatal_exception.
        Subsystems that catch the exception should do minimal work to 
        attempt to save critical user data and then re-throw the exception.
        The application should shutdown immediately in response to a 
        fatal exception.
     */
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

        char buffer[4096] = "";
        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotError(buffer);

#if 0 // !defined(NDEBUG) && defined(_MSC_VER)      
        lx0::lx_message_box("LxEngine Error", buffer);
        *(int*)0 = 0;
#endif
            
        std::string err("lx_error (re-throw if error is non-recoverable).\n");
        throw lx0::error_exception("Generic", (err + buffer).c_str());
    }

    void
    lx_error2 (const char* type, const char* format, ...)
    {
        _lx_check_init();

        char buffer[4096] = "";
        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotError(buffer);

        throw lx0::error_exception(type, buffer);
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
#ifndef NDEBUG
        _lx_check_init();

        char buffer[512] = "";
        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotDebug(buffer);
#endif
    }

    void lx_debug  (const std::string& s)
    {
        slotDebug(s.c_str());
    }

}}}
