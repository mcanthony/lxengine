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
    static std::ofstream s_log;
    static int s_log_count = 0;

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
        if (!s_lx_init_called)
        {
            lx_init();
            lx_error("lx_init() has not been called!");
        }
#endif
    }

    /*!
        Initializes the LxEngine code.  This is a small, low-cost function.  It can be
        safely called multiple times.
     */
    void 
    lx_init()
    {
        if (!s_lx_init_called)
        {
            // On a fatal error, this should be renamed to include the time & date so it
            // is not overwritten by the next run.
            s_log.open("lxengine_log.html");
            s_log 
                << "<html>" << std::endl
                << "<head>"
                << "  <title>LxEngine log</title>"
                << "  <style>"
                << "  body { font-family: sans-serif; font-size: 9pt; }" << std:: endl
                << "  li { white-space: pre; font-family: monospace; }" << std::endl
                << "  .prefix { font-size: 85%; font-variant: small-caps; float: left; width: 48px; padding-left: 26px; }" << std::endl
                << "  .debug { color: gray; font-size: 70%; } " << std::endl
                << "  .log { color: black; } " << std::endl
                << "  .warn { color: #f19527; } " << std::endl
                << "  .error { color: red; font-weight: bold; } " << std::endl
                << "  .fatal { color: red; background-color: yellow; font-weight: bold; } " << std::endl
                << "  </style>"
                << "</head>"
                << "<body>"
                << "<h1>Log</h1>"
                << "<ul style='padding-left: 12px;'>" 
                << std::endl;

            // Define a helper lambda function that returns a function (this effectively 
            // acts as runtime template function).
            auto prefix_print = [](std::string css, std::string prefix) -> std::function<void(const char*)> {
                return [css, prefix](const char* s) {
                    s_log << "<li class='" << css << "'><span class='prefix'>" << prefix << "</span>"<< s << "</li>" << std::endl; 
                    if (s_log_count++ == 100)
                    {
                        s_log_count = 0;
                        s_log.flush();
                    }
                };
            };
            slotDebug   = prefix_print("debug", "DBG");
            slotLog     = prefix_print("log",   "LOG");
            slotWarn    = prefix_print("warn",  "WARN");
            slotError   = prefix_print("error", "ERROR");
            slotFatal   = prefix_print("fatal", "FATAL: ");

            s_lx_init_called = true;
        }
    }

    /*!
        Check that a condition is true.   If it is not, cause an error.
     */
    void 
    lx_assert (bool condition)
    {
        _lx_check_init();

        if (!condition)
            *(char*)(void*)(0x0) = 'a';
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

            *(char*)(void*)(0x0) = 'a';
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

        char buffer[512] = "";
        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotError(buffer);

#if 0 // !defined(NDEBUG) && defined(_MSC_VER)      
        lx0::util::lx_message_box("LxEngine Error", buffer);
        *(int*)0 = 0;
#endif

        std::string err("lx_error (re-throw if error is non-recoverable).\n");
        throw std::exception((err + buffer).c_str());
    }

    void
    lx_error2 (const char* type, const char* format, ...)
    {
        _lx_check_init();

        char buffer[512] = "";
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
