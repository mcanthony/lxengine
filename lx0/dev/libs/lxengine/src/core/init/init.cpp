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

#include <string>
#include <iostream>
#include <fstream>
#include <functional>

#include <lx0/core/init/init.hpp>
#include <lx0/core/slot/slot.hpp>

namespace lx0 { namespace core { namespace log_ns {
    extern std::ofstream s_log;
    extern int s_log_count;

    extern lx0::slot<void (const char*)> slotFatal;
    extern lx0::slot<void (const char*)> slotError;
    extern lx0::slot<void (const char*)> slotWarn;
    extern lx0::slot<void (const char*)> slotLog;
    extern lx0::slot<void (const char*)> slotAssert;
    extern lx0::slot<void (const char*)> slotDebug;
}}}

using namespace lx0::core::log_ns;

namespace lx0 { namespace core { namespace init_ns {

    static bool s_lx_init_called = false;

    bool
    _lx_init_called()
    {
        return s_lx_init_called;
    }

    //! Base initialization (automatically called by Engine constructor)
    /*!
        \ingroup lx0_core_init

        Initializes the LxEngine code.  This is a small, low-cost function.  It can be
        safely called multiple times.

        This function should only be called if the Engine class is not being used.
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
                << "  .multiline { white-space: pre; font-family: monospace; margin-left: 48px; padding: 4px; padding-bottom: 8px }" << std::endl
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
                    
                    // Quick and ugly HTML escaping
                    bool multiline = false;
                    size_t len = strlen(s);
                    std::string t;
                    t.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        switch (s[i])
                        {
                        case '<':   t += "&lt;";    break;
                        case '>':   t += "&gt;";    break;
                        case '\n':  t += "<br />";  multiline = true; break;
                        default:
                            t += s[i];
                        }
                    }
                    if (multiline)
                        s_log << "<li class='" << css << "'><span class='prefix'>" << prefix << "</span></li>" 
                            << "<div class='multiline " << css << "'>"<< t << "</div>"  
                            << std::endl; 
                    else
                        s_log << "<li class='" << css << "'><span class='prefix'>" << prefix << "</span>"<< t << "</li>" << std::endl;

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
}}}
