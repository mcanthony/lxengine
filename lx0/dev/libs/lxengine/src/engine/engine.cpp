//===========================================================================//
/*
                                   LxEngine

    LICENSE

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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

#include <iostream>
#include <string>

#include <lx0/lxengine.hpp>
#include <lx0/engine/engine.hpp>
#include <lx0/engine/document.hpp>
#include <lx0/engine/element.hpp>
#include <lx0/util/misc/util.hpp>

#include <boost/format.hpp>

using namespace lx0::util;

//===========================================================================//
//   I M P L E M E N T A T I O N 
//===========================================================================//

namespace lx0 { namespace engine { namespace dom_ns {

    namespace detail
    {
        ObjectCount::ObjectCount (void)
            : mCurrent (0)
            , mTotal   (0)
        {
        }

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

    Engine::Environment::Environment ()
        : mTimeScale (1.0f)
    {
    }

    void
    Engine::Environment::setTimeScale (float s)
    {
        if (s < 0.0f)
            lx_error("Time scale cannot be negative.  '%f' is not a valid value.", s);
        else
            mTimeScale = s;
    }


    Engine::Engine()
    {
        lx_init();
        lx_log("lx::core::Engine ctor");

        lxvar info = getSystemInfo();
        lx_debug("%s", lx0::format_tabbed(info).c_str());

        ///@todo These should not be included by default
        _attachSound();
        _attachJavascript();
    }

    void 
    Engine::incPerformanceCounter (std::string name, lx0::uint64 t) 
    { 
        auto it = m_perfCounters.find(name);
        if (it != m_perfCounters.end())
        {
            it->second.events++;
            it->second.total += t;
        }
        else
            m_perfCounters.insert(std::make_pair(name, PerfCounter(1, t))); 
    }

    /*!
        Subject to future change.

        An explicit shutdown method, in addition to the normal destructor, is currently required
        to ensure a proper order of events for object destruction.
     */
    void
    Engine::shutdown()
    {
        lx_log("Engine::shutdown()");

        for (auto it = m_perfCounters.begin(); it != m_perfCounters.end(); ++it)
        {
            lx_log("COUNTER: %s => %lf avg, %lf total %.0lf events", it->first.c_str(), 
                double(it->second.total) / double(it->second.events), 
                double(it->second.total), double(it->second.events));
        }
            
        // Explicitly free all references to shared objects so that memory leak checks will work
        mDocuments.clear();
    }

    Engine::~Engine()
    {
       lx_log("lx::core::Engine dtor");

       // Check for memory leaks of Engine-related objects
       bool bLeaksFound = false;
       for (auto it = m_objectCounts.begin(); it != m_objectCounts.end(); ++it)
       {
           if (it->second.current() != 0)
           {
               lx_warn("Leaked %u %s objects (%.1f%%)", it->second.current(), it->first.c_str(),
                   100.0f * float(it->second.current()) / float(it->second.total()));
               bLeaksFound = true;   
           }
           else
               lx_debug("Allocated %u %s objects.  0 leaked.", it->second.total(), it->first.c_str()); 
        }
        if (bLeaksFound)
        {
            lx_warn("Memory leaks detected!  All major Lx objects should be freed before the Engine "
                    "object is freed.  Was Engine::shutdown() called?");

            // Stop the debugger immediately if a memory leak is detected.
            // It usually is a lot less difficult to track down leaks as 
            // soon as they are introduced.
            lx0::lx_break_if_debugging();
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

    /*!
        Defers an exception such that it will be rethrown at the start of the next update.
        This is reserved for cases where the exception simply cannot be thrown all the
        way up the call stack to where it can be handled appropriately.

        For example, the Windows message loop code will consume any exception that 
        propogates through it; therefore, this method is used to get the exception back
        to the client code - albeit postponed until the next update - so that the 
        relevant failure is communicated to the client.

        This method should be used as rarely as possible.
     */ 
    void
    Engine::postponeException (lx0::error_exception& e)
    {
        lx_break_if_debugging();

        m_postponedExceptions.push_back(e);
    }

    lxvar
    Engine::getSystemInfo (void)
    {
        // Cache the system info the first time it is computed
        if (!mSystemInfo.isDefined())
        {
            auto& info = mSystemInfo;
            info = lxvar::ordered_map();

            info["sizes"] = lxvar::ordered_map();
            info["sizes"]["char"] =            (int)sizeof(char);
            info["sizes"]["short"] =           (int)sizeof(short);
            info["sizes"]["int"] =             (int)sizeof(int);
            info["sizes"]["long"] =            (int)sizeof(long);
            info["sizes"]["float"] =           (int)sizeof(long);
            info["sizes"]["double"] =          (int)sizeof(long);
            info["sizes"]["pointer"] =         (int)sizeof(void*);
            info["sizes"]["std::unique_ptr"] = (int)sizeof(std::shared_ptr<int>);
            info["sizes"]["std::shared_ptr"] = (int)sizeof(std::shared_ptr<int>);
            info["sizes"]["std::weak_ptr"] =   (int)sizeof(std::weak_ptr<int>);
            info["sizes"]["std::string"] =     (int)sizeof(std::string);
            info["sizes"]["Document"] =        (int)sizeof(Document);
            info["sizes"]["Element"] =         (int)sizeof(Element);

            info["system"] = lxvar::ordered_map();
            info["system"]["current_time"] = lx_ctime();

            // Credit to the Quake 2 source code (Swap_Init) for this test
            {
                lx0::uint8 bytes[2] = { 1, 0 };
                info["system"]["endian"] = (*(short*)bytes == 1) ? "little" : "big";
            }

            info["system"]["operating_system"] = lxvar::ordered_map();
            lx_operating_system_info(info["system"]["operating_system"]);
            
            info["lxengine"] = lxvar::ordered_map();
            info["lxengine"]["version"] = boost::str( boost::format("%d.%d.%d") % versionMajor() % versionMinor() % versionRevision() ); 
            
            info["build"] = lxvar::ordered_map();
            info["build"]["date"] = boost::str( boost::format("%s %s") % __DATE__ % __TIME__ ); 
            info["build"]["compiler_version"] = boost::str( boost::format("0x%04x") % _MSC_VER ); 
        }
        return mSystemInfo.clone();
    }

    void
    Engine::_notifyDocumentCreated (DocumentPtr spDocument)
    {
        _foreach([&] (ComponentPtr spComponent) {
            spComponent->onDocumentCreated(shared_from_this(), spDocument);
        });
    }

    
    DocumentPtr
    Engine::createDocument (void)
    {
        return _loadDocument(true, std::string());
    }

    DocumentPtr
    Engine::loadDocument (std::string filename)
    {
        return _loadDocument(false, filename);
    }

    /*!
        \todo Design choice: do documents need to be explicitly closed?  Can they be implicitly closed
            when the Engine is released?
     */
    void
    Engine::closeDocument (DocumentPtr spDocument)
    {
        // Find the document and remove it from the internal list
        auto it = mDocuments.begin();
        while (it != mDocuments.end())
        {
            if (it->get() == spDocument.get())
                break;
            ++it;
        }

        if (it != mDocuments.end())
            mDocuments.erase(it);
        else
            lx_error("Document being closed but not in Engine Document list! Was this Document already closed?");
    }

    DocumentPtr
    Engine::_loadDocument (bool bCreate, std::string filename)
    {
        DocumentPtr spDocument(new Document);

        // Attach all registered per-document Components
        //
        for (auto it = mDocumentComponents.begin(); it != mDocumentComponents.end(); ++it)
        {
            auto pComponent = (it->second)();
            spDocument->attachComponent(it->first, pComponent);
        }

        _notifyDocumentCreated(spDocument);
        _attachPhysics(spDocument);

        ElementPtr spRoot;
        if (!bCreate)
        {
            spRoot = _loadDocumentRoot(spDocument, filename);
            if (!spRoot)
            {
                lx_error("Could not load document.  Does file '%s' exist?", filename.c_str());
            }
        }
        else
        {
            spRoot = spDocument->createElement("Document");
        }

        spDocument->root(spRoot);


        mDocuments.push_back(spDocument);

        // This is probably not the exactly right place for the scripts to be run.
        // If nothing else, this is likely not consistent with HTML which runs
        // scripts as the document is being loaded.  Should the HTML behavior be
        // emulated or is this approach cleaner?
        _processDocumentHeader(spDocument);

        return spDocument;
    }

    void
    Engine::_processDocumentHeader (DocumentPtr spDocument)
    {
        std::vector<std::string> scripts;
        ElementPtr spRoot = spDocument->root();
        for (int i = 0; i < spRoot->childCount(); ++i)
        {
            ElementCPtr spChild = spRoot->child(i);
            if (spChild->tagName() == "Header")
            {
                for (int j = 0; j < spChild->childCount(); ++j)
                {
                    ElementCPtr spElem = spChild->child(j);
                    if (spElem->tagName() == "Script")
                    {
                        std::string language = spElem->attr("language").as<std::string>();
                        std::string content;
                        if (spElem->value().isDefined())
                        {
                            content = spElem->value().as<std::string>();
                        }
                        else
                        {
                            std::string filename = spElem->attr("src").as<std::string>();
                            content = lx0::lx_file_to_string(filename);
                        }

                        lx_check_error(language.empty() || language == "javascript");
                        _runJavascript(spDocument, content);
                    }
                }
            }
        }
    }

	void   
	Engine::sendEvent (const char* message)
    {
        if (strcmp("quit", message) == 0)
            lx_debug("Message 'quit' sent to Engine");

        m_messageQueue.push_back(message);
    }

    void        
    Engine::_throwPostponedException (void)
    {
        if (!m_postponedExceptions.empty())
        {
            auto e = m_postponedExceptions.front();
            m_postponedExceptions.pop_front();
            throw e;
        }
    }

	int
	Engine::run()
	{
        const lx0::uint64 start = lx0::lx_milliseconds();

        _lx_reposition_console();

        for(auto it = mDocuments.begin(); it != mDocuments.end(); ++it)
            (*it)->beginRun();

        bool bDone = false;
        do
        {
            const auto startLoop = lx0::lx_milliseconds();

            _throwPostponedException();

            while (!m_messageQueue.empty())
            {
                std::string msg = m_messageQueue.front();
                m_messageQueue.pop_front();

                if (msg == "quit")
                    bDone = true;
            }


            if (!bDone)
                bDone = _handlePlatformMessages();
            
            _throwPostponedException();

            ///@todo Devise a better way to hand time-slices from the main loop to the individual documents
            /// for updates.  Also consider multi-threading.
            {
                const auto startLoop = lx0::lx_milliseconds();

                for(auto it = mDocuments.begin(); it != mDocuments.end(); ++it)
                    (*it)->updateRun();

                incPerformanceCounter("Engine>run>doc update", lx0::lx_milliseconds() - startLoop);
            }

            incPerformanceCounter("Engine>run>loop", lx0::lx_milliseconds() - startLoop);

        } while (!bDone);

        for(auto it = mDocuments.begin(); it != mDocuments.end(); ++it)
            (*it)->endRun();

        incPerformanceCounter("Engine>run", lx0::lx_milliseconds() - start);

		return 0;
	}


    void
    Engine::addViewPlugin (std::string name, std::function<ViewImp*(View*)> ctor)
    {
        auto it = mViewImps.find(name);
        if (it == mViewImps.end())
        {
            mViewImps.insert(std::make_pair(name, ctor));
        }
        else
        {
            lx_error("Registering View plugin '%s': there is already a plugin with that name.", name.c_str()); 
        }
    }

    ViewImp*
    Engine::_createViewImp (std::string name, View* pView)
    {
        auto it = mViewImps.find(name);
        if (it != mViewImps.end())
        {
            return (it->second)(pView);
        }
        else
        {
            lx_error("No View plug-in with name '%s' found.", name.c_str()); 
            return nullptr;
        }
    }

    /*!
        Register a Document::Component that is added to all created / loaded documents.

        @todo Eventually, Documents should have a type, such that a Document::Component is
            added to only certain types (or a * selector for all types).
     */
    void
    Engine::addDocumentComponent (std::string name, std::function<Document::Component* ()> ctor)
    {
        mDocumentComponents.insert(std::make_pair(name, ctor));
    }

    void
    Engine::addElementComponent (std::string tag, std::string name, std::function<ElementComponent*(ElementPtr)> ctor)
    {
        mElementComponents[tag].push_back(std::make_pair(name, ctor));
    }

}}}
