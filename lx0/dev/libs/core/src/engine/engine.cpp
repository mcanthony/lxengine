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

#include <iostream>
#include <string>

#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreSceneManager.h>

namespace lx0 { namespace core {

    namespace detail
    {
        ObjectCount::ObjectCount (size_t current)
            : mCurrent (current)
            , mTotal   (current)
        {
        }

        void   
        ObjectCount::inc (void)
        {
            mCurrent++;
            mTotal++;
        }

        void   
        ObjectCount::dec (void)
        {
            mCurrent--;
        }
    }

    using namespace detail;

    std::weak_ptr<Engine> Engine::s_wpEngine;

    Engine::Engine()
    {
        // Define a helper lambda function that returns a function (this effectively 
        // acts as runtime template function).
        auto prefix_print = [](std::string prefix) -> std::function<void(const char*)> {
            return [prefix](const char* s) { std::cout << prefix << s << std::endl; };
        };
        slotDebug   = prefix_print("DBG: ");
        slotLog     = prefix_print("LOG: ");
        slotWarn    = prefix_print("WARN: ");
        slotError   = prefix_print("ERROR: ");
        slotFatal   = prefix_print("FATAL: ");

        lx_log("lx::core::Engine ctor");
    }

    /*!
        Subject to future change.

        An explicit shutdown method, in addition to the normal destructor, is currently required
        to ensure a proper order of events for object destruction.
     */
    void
    Engine::shutdown()
    {
        // Explicitly free all references to shared objects so that memory leak checks will work
       m_documents.clear();
    }

    Engine::~Engine()
    {
       lx_log("lx::core::Engine dtor");

       // Check for memory leaks of Engine-related objects
       for (auto it = m_objectCounts.begin(); it != m_objectCounts.end(); ++it)
       {
           if (it->second.current() != 0)
               lx_warn("Leaked %u %s objects", it->second.current(), it->first.c_str());
           else
               lx_debug("Allocated %u %s objects.  0 leaked.", it->second.total(), it->first.c_str()); 
       }
    }

    /*!
        @todo This method is inefficient; but it is simple.  Until 1.0 is complete, simplicity
            is favored over efficiency.
     */
    void
    Engine::incObjectCount  (std::string name)
    {
        auto it = m_objectCounts.find(name);
        if (it == m_objectCounts.end())
            m_objectCounts.insert(std::make_pair(name, ObjectCount(1)));
        else
            it->second.inc();
    }

    void 
    Engine::decObjectCount  (std::string name)
    {
        auto it = m_objectCounts.find(name);
        lx_check_error (it != m_objectCounts.end());
        lx_check_fatal(it->second.current() >= 1);

        it->second.dec();
    }

    void    
    Engine::connect (DocumentPtr spDocument)
    {
        m_documents.push_back(spDocument);
    }

	void   
	Engine::sendMessage (const char* message)
    {
        m_messageQueue.push_back(message);
    }

	int
	Engine::run()
	{
        while (!m_messageQueue.empty())
        {
            std::string msg = m_messageQueue.front();
            m_messageQueue.pop_front();
        }

        ///@todo Devise a better way to hand time-slices from the main loop to the individual documents
        /// for updates.  Also consider multi-threading.
        for(auto it = m_documents.begin(); it != m_documents.end(); ++it)
            (*it)->run();

		return 0;
	}

}}