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
                error_exception (const char* file, int line);
                error_exception (const char* file, int line, const char* s);

                template <typename T0>
                error_exception (const char* file, int line, const char* format, T0 a0) : detail::_exception_base(file, line) { detail(format, a0); }
                
                template <typename T0, typename T1>  
                error_exception (const char* file, int line, const char* format, T0 a0, T1 a1) : detail::_exception_base(file, line) { detail(format, a0, a1); }
                    
                template <typename T0, typename T1, typename T2>  
                error_exception (const char* file, int line, const char* format, T0 a0, T1 a1, T2 a2) : detail::_exception_base(file, line) { detail(format, a0, a1, a2); }
                    
                template <typename T0, typename T1, typename T2, typename T3>  
                error_exception (const char* file, int line, const char* format, T0 a0, T1 a1, T2 a2, T3 a3) : detail::_exception_base(file, line) { detail(format, a0, a1, a2, a3); }
                    
                template <typename T0, typename T1, typename T2, typename T3, typename T4>  
                error_exception (const char* file, int line, const char* format, T0 a0, T1 a1, T2 a2, T3 ae, T4 a4) : detail::_exception_base(file, line) { detail(format, a0, a1, a2, a3, a4); }
            };

            /*!
                \ingroup lx0_core_log
             */
            class fatal_exception : public detail::_exception_base
            {
            public:
                fatal_exception (const char* file, int line) : detail::_exception_base(file, line) {}

                fatal_exception (const char* file, int line, const char* s) : detail::_exception_base(file, line) { detail("%1%", s); }

                template <typename T0>
                fatal_exception (const char* file, int line, const char* format, T0 a0) : detail::_exception_base(file, line) { detail(format, a0); }
                
                template <typename T0, typename T1>  
                fatal_exception (const char* file, int line, const char* format, T0 a0, T1 a1) : detail::_exception_base(file, line) { detail(format, a0, a1); }
                    
                template <typename T0, typename T1, typename T2>  
                fatal_exception (const char* file, int line, const char* format, T0 a0, T1 a1, T2 a2) : detail::_exception_base(file, line) { detail(format, a0, a1, a2); }
                    
                template <typename T0, typename T1, typename T2, typename T3>  
                fatal_exception (const char* file, int line, const char* format, T0 a0, T1 a1, T2 a2, T3 a3) : detail::_exception_base(file, line) { detail(format, a0, a1, a2, a3); }
                    
                template <typename T0, typename T1, typename T2, typename T3, typename T4>  
                fatal_exception (const char* file, int line, const char* format, T0 a0, T1 a1, T2 a2, T3 ae, T4 a4) : detail::_exception_base(file, line) { detail(format, a0, a1, a2, a3, a4); }
            };

            //===========================================================================//
            // defines
            //===========================================================================//

            #define lx_message(FMT,...)             lx0::_lx_message_imp(__FILE__,__LINE__,_lx_format(FMT,__VA_ARGS__))
            #define lx_debug(FMT,...)               lx0::_lx_debug_imp(__FILE__,__LINE__,_lx_format(FMT,__VA_ARGS__))
            #define lx_log(FMT,...)                 lx0::_lx_log_imp(__FILE__,__LINE__,_lx_format(FMT,__VA_ARGS__))
            #define lx_warn(FMT,...)                lx0::_lx_warn_imp(__FILE__,__LINE__,_lx_format(FMT,__VA_ARGS__))
            #define lx_error_exception(FMT,...)     lx0::error_exception(__FILE__, __LINE__,FMT,__VA_ARGS__)
            #define lx_fatal_exception(FMT,...)     lx0::fatal_exception(__FILE__, __LINE__,FMT,__VA_ARGS__)
                        
            #define lx_assert(CONDITION,...)        if (!(CONDITION)) { lx_warn("Assert failed: '%s' at %s:%d", #CONDITION, __FILE__, __LINE__); lx0::_lx_warn_imp(__FILE__,__LINE__,_lx_format(__VA_ARGS__)); lx_break_if_debugging(); }
            #define lx_check_warn(CONDITION,...)    if (!(CONDITION)) { lx_warn("Check failed: '%s'", #CONDITION); lx0::_lx_warn_imp(__FILE__,__LINE__,_lx_format(__VA_ARGS__)); }
            #define lx_check_error(CONDITION,...)   if (!(CONDITION)) { lx0::error_exception e(__FILE__, __LINE__); e.detail("Error check failed: '%s'", #CONDITION); e.detail(__VA_ARGS__); throw e; }
            #define lx_check_fatal(CONDITION,...)   if (!(CONDITION)) { lx0::fatal_exception e(__FILE__, __LINE__); e.detail("Error check failed: '%s'", #CONDITION); e.detail(__VA_ARGS__); throw e; }

            #define lx_message_once(FMT,...)        do { static bool once = false; if(!once) { lx_message(FMT,__VA_ARGS__); once = true; } } while (0)
            #define lx_debug_once(FMT,...)          do { static bool once = false; if(!once) { lx_debug(FMT,__VA_ARGS__); once = true; } } while (0)
            #define lx_log_once(FMT,...)            do { static bool once = false; if(!once) { lx_log(FMT,__VA_ARGS__); once = true; } } while (0            
            #define lx_warn_once(FMT,...)           do { static bool once = false; if(!once) { lx_warn(FMT,__VA_ARGS__); once = true; } } while (0)

            #ifndef _DEBUG
                #undef lx_assert
                #define lx_assert(FMT,...)
                #undef lx_debug
                #define lx_debug(FMT,...)
            #endif

            //===========================================================================//
            // functions
            //===========================================================================//

            inline std::string _lx_format      (void) { return std::string(); }
            inline std::string _lx_format      (const char* format) { return std::string(format); }
            template <typename T0>  
            std::string _lx_format      (const char* format, T0 a0) {  return boost::str( boost::format(format) % a0 ); }
            template <typename T0, typename T1>  
            std::string _lx_format      (const char* format, T0 a0, T1 a1) {  return boost::str( boost::format(format) % a0 % a1 ); }
            template <typename T0, typename T1, typename T2>  
            std::string _lx_format      (const char* format, T0 a0, T1 a1, T2 a2) { return boost::str( boost::format(format) % a0 % a1 % a2 ); }
            template <typename T0, typename T1, typename T2, typename T3>  
            std::string _lx_format      (const char* format, T0 a0, T1 a1, T2 a2, T3 a3) { return boost::str( boost::format(format) % a0 % a1 % a2 % a3 ); }
            template <typename T0, typename T1, typename T2, typename T3, typename T4>  
            std::string _lx_format      (const char* format, T0 a0, T1 a1, T2 a2, T3 a3, T4 a4) { return boost::str( boost::format(format) % a0 % a1 % a2 % a3 % a4 ); }

            void        _lx_message_imp (const char* file, int line, const std::string& s);
            void        _lx_debug_imp   (const char* file, int line, const std::string& s);
            void        _lx_log_imp     (const char* file, int line, const std::string& s);
            void        _lx_warn_imp    (const char* file, int line, const std::string& s);

            inline void _lx_message_imp (const char* file, int line) {}
            inline void _lx_debug_imp   (const char* file, int line) {}
            inline void _lx_log_imp     (const char* file, int line) {}
            inline void _lx_warn_imp    (const char* file, int line) {}

            void        _lx_write_to_log(const char* css, const char* prefix, const char* s);
        }
    }
    using namespace lx0::core::log_ns;
}

