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

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <lx0/core/init/init.hpp>
#include <lx0/core/log/log.hpp>
#include <lx0/core/slot/slot.hpp>
#include <lx0/util/misc/util.hpp>

namespace lx0 { namespace core { namespace log_ns {

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
            throw lx_error_exception("lx_init() has not been called!");
        }
#endif
    }

    detail::_exception_base::_exception_base (const char* file, int line)
    {
        lx_log("lx0::error_exception (%p) created ", this);
        mWhat.reserve(512);
        location(file, line);
    }

    void 
    detail::_exception_base::location (const char* file, int line)
    {
        lx_log("lx0::error_exception (%p) location: %s : %d", this, file, line);
        mWhat += boost::str(boost::format("\n>> %1%:%2%\n") % file % line);

    }

    detail::_exception_base&
    detail::_exception_base::detail (const char* msg)
    {
        lx0::lx_debugger_message(boost::str(boost::format("lx0:error_exception (%p) details:\n%s\n") % this % msg));
        lx_log("lx0::error_exception (%p) detail: %s", this, msg);
        mWhat += msg;
        mWhat += "\n";

        lx_break_if_debugging();

        return *this;
    }


    const char*
    detail::_exception_base::what() const 
    {
        return mWhat.c_str();
    }

    error_exception::error_exception (const char* file, int line) 
        : detail::_exception_base(file, line) 
    {        
    }

    error_exception::error_exception (const char* file, int line, const char* s) 
        : detail::_exception_base(file, line) 
    { 
        detail("%1%", s);         
    }

    void _lx_message_imp (const char* file, int line, const std::string& s)
    {
        _lx_write_to_log("message", "MSG", s.c_str());
        std::cout << s << std::endl;
    }

    void _lx_debug_imp   (const char* file, int line, const std::string& s)
    {
        _lx_write_to_log("debug", "DBG", s.c_str());

        boost::filesystem::path filePath(file);
        std::string filename = filePath.filename().string();
        if (filename.front() == '\"' && filename.back() == '\"')
            filename = filename.substr(1, filename.length() - 2);

        lx0::lx_debugger_message(boost::str(boost::format("DEBUG %s:%d: %s\n") % filename % line % s));;
    }

    void _lx_log_imp     (const char* file, int line, const std::string& s)
    {
        _lx_write_to_log("log", "LOG", s.c_str());
    }

    void _lx_warn_imp    (const char* file, int line, const std::string& s)
    {
        _lx_write_to_log("warn", "WARN", s.c_str());
        lx_debugger_message("LX WARNING: " + s + "\n");
        std::cerr << "WARNING: " << s << std::endl;
    }

}}}
