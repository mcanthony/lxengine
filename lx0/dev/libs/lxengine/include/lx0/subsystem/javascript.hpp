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

#include <boost/any.hpp>
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
            /*!
                Primary interface to the Javascript subsystem on a particular Document.
             */
            class IJavascriptDoc : public lx0::Document::Component
            {
            public:
                static  const char* s_name  (void)       { return "javascript"; }
                virtual const char* name    (void) const { return s_name(); }
                
                template <typename T>
                void                addObject (const char* name, std::shared_ptr<T>& spObject)
                {
                    auto pspObject = new std::shared_ptr<T>(spObject);
                    auto dtor = [&]() -> void { delete pspObject; };
                    _addObject(name, typeid(T).hash_code(), spObject.get(), dtor);
                }
                virtual void        addObject (const char* name, void* pointerToHandleToObject) = 0;

                //! Executes a string of Javascript code in the context of the Document
                virtual lx0::lxvar  run     (const std::string& source) = 0;

                virtual void        runInContext        (std::function<void(void)> func) = 0;

                template <typename T>
                std::function<T> acquireFunction (const char* functionName)
                {
                    std::function<T> func;
                    boost::any t(func);
                    _acquireFunction(functionName, t);
                    return boost::any_cast< std::function<T> >(t);
                }

            protected:
                virtual void        _addObject           (const char* objectName,  size_t type_hash, void* pObject, std::function<void()> dtor) = 0;
                virtual void        _acquireFunction     (const char* functionName, boost::any& func) = 0;


            };

            Engine::Component* createJavascriptSubsystem();
        }
    }

    using namespace lx0::subsystem::javascript_ns;
}
