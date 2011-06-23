//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

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

#include <lx0/engine/document.hpp>

namespace lx0 
{
    namespace subsystem
    {
         /*!
            \defgroup lx0_subsystem_javascript lx0_subsystem_javascript
            \ingroup Subsystem
         */
        namespace javascript_ns
        {
            class IJavascript : public lx0::Document::Component
            {
            public:
                virtual void run (const std::string& source) = 0;
            };

            IJavascript* createIJavascript();

            class Plugin
            {
            public:
                virtual ~Plugin() {}
            };

            class JavascriptPlugin : public Engine::Component
            {
            public:
                virtual void    onDocumentCreated   (EnginePtr spEngine, DocumentPtr spDocument);
            };
        }
    }

    using namespace lx0::subsystem::javascript_ns;
}
