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

#include <string>
#include <lx0/core/lxvar/lxvar.hpp>

namespace lx0 { namespace util { namespace misc {

            void                lx_break_if_debugging   (void);

            bool                lx_file_exists          (std::string filename);
            bool                lx_file_is_open         (std::string filename);
            std::string         string_from_file        (std::string filename);
            lx0::lxvar          lxvar_from_file         (std::string filename);

            std::string         lx_itoa                 (size_t i);
            std::string         lx_ctime                (void);

            unsigned int        lx_milliseconds         (void);
            void                lx_message_box          (std::string caption, std::string message);

            void                _lx_reposition_console  (void);

            void                lx_operating_system_info (lxvar& map);


            class Timer
            {
            public:
                            Timer() : mCount(0), mTotal(0) {}

            void            start       (void)      { mStart = _ticks(); }
            void            stop        (void)      { mTotal += _ticks() - mStart; mCount++; mStart = 0; }

            int             count       (void) const { return mCount; }
            double          averageMs   (void) const { return (lx0::uint32)(mTotal * 1000 / (_ticksPerSec() * mCount)); } 
            lx0::uint32     totalMs     (void) const { return (lx0::uint32)(mTotal * 1000 / _ticksPerSec()); }

            protected:
                static  lx0::int64  _ticks();
                static  lx0::int64  _ticksPerSec();

                int         mCount;
                lx0::int64  mStart;
                lx0::int64  mTotal;
            };

        }
    }
    using namespace lx0::util::misc;
}
