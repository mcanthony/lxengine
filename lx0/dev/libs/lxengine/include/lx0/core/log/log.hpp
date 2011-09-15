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


#pragma once

#include <boost/format.hpp>

#include <lx0/core/slot/slot.hpp>

namespace lx0 
{ 
    namespace core 
    {
        namespace log_ns
        {
            //===========================================================================//
            // types
            //===========================================================================//

            namespace detail
            {
                class _exception_base : public std::exception 
                {
                public:
                                    _exception_base (const char* file, int line);

                    void                location (const char* file, int line);
                    _exception_base&    detail   (void) { return *this; }
                    _exception_base&    detail   (const char* msg);

                    template <typename T0>  
                    _exception_base& detail (const char* format, T0 a0) {  return detail( boost::str( _fmt(format) % a0 ).c_str() ); }
                    
                    template <typename T0, typename T1>  
                    _exception_base& detail (const char* format, T0 a0, T1 a1) {  return detail( boost::str( _fmt(format) % a0 % a1 ).c_str() ); }
                    
                    template <typename T0, typename T1, typename T2>  
                    _exception_base& detail (const char* format, T0 a0, T1 a1, T2 a2) { return detail( boost::str( _fmt(format) % a0 % a1 % a2 ).c_str() ); }
                    
                    template <typename T0, typename T1, typename T2, typename T3>  
                    _exception_base& detail (const char* format, T0 a0, T1 a1, T2 a2, T3 a3) { return detail( boost::str( _fmt(format) % a0 % a1 % a2 % a3 ).c_str() ); }
                    
                    template <typename T0, typename T1, typename T2, typename T3, typename T4>  
                    _exception_base& detail (const char* format, T0 a0, T1 a1, T2 a2, T3 ae, T4 a4) { return detail( boost::str( _fmt(format) % a0 % a1 % a2 % a3 % a4 ).c_str() ); }

                    virtual const char* what() const throw();

                protected:
                    boost::format _fmt(const char* format)
                    {
                        // Disable exceptions within boost::format, since nested exceptions can not be handled
                        // properly
                        boost::format f(format);
                        f.exceptions(boost::io::no_error_bits);
                        return f;
                    }

                    std::string       mWhat;
                };
            }


            /*!
                \ingroup lx0_core_log
             */
            class error_exception : public detail::_exception_base
            {
            public:
                    error_exception (const char* file, int line) : detail::_exception_base(file, line) {}
                    
                    template <typename T0>
                    error_exception (const char* file, int line, const char* format, T0 a0) : detail::_exception_base(file, line) { detail(format, a0); }
            };

            /*!
                \ingroup lx0_core_log
             */
            class fatal_exception : public detail::_exception_base
            {
            public:
                    fatal_exception (const char* file, int line) : detail::_exception_base(file, line) {}
            };

            //===========================================================================//
            // defines
            //===========================================================================//

            #define lx_error_exception(F,...) \
                lx0::error_exception(__FILE__, __LINE__,F,__VA_ARGS__)
            #define lx_check_error(CONDITION,...) \
                if (!(CONDITION)) { lx0::error_exception e(__FILE__, __LINE__); e.detail("Error check failed: '%s'", #CONDITION); e.detail(__VA_ARGS__); throw e; }

            //===========================================================================//
            // functions
            //===========================================================================//

            void        lx_assert       (bool condition);
            void        lx_assert       (bool condition, const char* format, ...);

            void        lx_debug        (const char* format, ...);
            void        lx_debug        (const std::string& s);
            void        lx_log          (const char* format, ...);
            void        lx_warn         (const char* format, ...);
            void        lx_error        (const char* format, ...);
            void        lx_fatal        (void);
            void        lx_fatal        (const char* format, ...);
            
    
            void        lx_check_fatal (bool condition);

            #define     lx_warn_once(FORMAT,...)  do { static bool once = false;  if(!once) { lx_warn(FORMAT,__VA_ARGS__); once = true; } } while (0)


            extern slot<void (const char*)> slotFatal;
            extern slot<void (const char*)> slotError;
            extern slot<void (const char*)> slotWarn;
            extern slot<void (const char*)> slotLog;
            extern slot<void (const char*)> slotAssert;
            extern slot<void (const char*)> slotDebug;
    



            /*!
                \ingroup lx0_core_log
             */
            void lx_error2 (const char* name, const char* detailsFormat, ...);
            
            /*!
                \ingroup lx0_core_log
             */
            inline void lx_error2 (const char* name) { lx_error2(name, ""); }



            
        }
    }
    using namespace lx0::core::log_ns;
}

