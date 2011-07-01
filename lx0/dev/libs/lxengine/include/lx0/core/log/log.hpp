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

#include <lx0/core/slot/slot.hpp>

namespace lx0 
{ 
    namespace core 
    {
        namespace log_ns
        {

            /*!
                \ingroup lx0_core_log
             */
            void lx_assert (bool condition);
            
            /*!
                \ingroup lx0_core_log
             */
            void lx_assert (bool condition, const char* format, ...);

            /*!
                \ingroup lx0_core_log
             */
            void lx_fatal  (void);
            
            /*!
                \ingroup lx0_core_log
             */
            void lx_fatal  (const char* format, ...);
            
            /*!
                \ingroup lx0_core_log
             */
            void lx_error  (const char* format, ...);
            
            /*!
                \ingroup lx0_core_log
             */
            void lx_warn   (const char* format, ...);
            
            /*!
                \ingroup lx0_core_log
             */
            void lx_log    (const char* format, ...);
            
            /*!
                \ingroup lx0_core_log
             */
            void lx_debug  (const char* format, ...);

            /*!
                \ingroup lx0_core_log
             */
            void lx_debug  (const std::string& s);
            
            /*!
                \ingroup lx0_core_log
             */
            #define lx_warn_once(FORMAT,...) \
                do { static bool once = false;  if(!once) { lx_warn(FORMAT,__VA_ARGS__); once = true; } } while (0)
    
            /*!
                \ingroup lx0_core_log
             */
            void lx_check_fatal (bool condition);
            
            /*!
                \ingroup lx0_core_log
             */
            void lx_check_error (bool condition);
            
            /*!
                \ingroup lx0_core_log
             */
            void lx_check_error (bool condition, const char* format, ...);

            extern slot<void (const char*)> slotFatal;
            extern slot<void (const char*)> slotError;
            extern slot<void (const char*)> slotWarn;
            extern slot<void (const char*)> slotLog;
            extern slot<void (const char*)> slotAssert;
            extern slot<void (const char*)> slotDebug;
    
            /*!
                \ingroup lx0_core_log
             */
            class error_exception : public std::exception 
            {
            public:
                                error_exception (const char* t, const char* d)
                                    : mType     (t)
                                    , mDetails  (d)
                                { }

                std::string     type    (void) const    { return mType; }
                std::string     details (void) const    { return mDetails; }

            protected:
                std::string     mType;
                std::string     mDetails;
            };

            /*!
                \ingroup lx0_core_log
             */
            class fatal_exception : public std::exception 
            {
            public:
                    fatal_exception (const char* t, const char* d)
                        : mType     (t)
                        , mDetails  (d)
                    { }

                std::string     type    (void) const    { return mType; }
                std::string     details (void) const    { return mDetails; }

            protected:
                std::string     mType;
                std::string     mDetails;
            };


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

