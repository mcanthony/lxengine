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
#include <string>

#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/util.hpp>

using namespace lx0::util;

//===========================================================================//
//   I M P L E M E N T A T I O N 
//===========================================================================//

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

        {
            lx_debug("    current time: %s", lx_ctime().c_str());
            lx_debug("    version %d.%d", versionMajor(), versionMinor());
            lx_debug("    build date:  %s %s", __DATE__, __TIME__);
            lx_debug("    _MSC_VER = 0x%04x", _MSC_VER);
            lx_debug("    sizeof(char) = %u bytes", sizeof(char));
            lx_debug("    sizeof(int) = %u bytes", sizeof(int));
            lx_debug("    sizeof(long) = %u bytes", sizeof(long));
            lx_debug("    sizeof(float) = %u bytes", sizeof(float));
            lx_debug("    sizeof(void*) = %u bytes", sizeof(void*));
            lx_debug("    sizeof(std::unique_ptr<int>) = %u bytes", sizeof(std::unique_ptr<int>));
            lx_debug("    sizeof(std::shared_ptr<int>) = %u bytes", sizeof(std::shared_ptr<int>));
            lx_debug("    sizeof(std::weak_ptr<int>) = %u bytes", sizeof(std::weak_ptr<int>));
            lx_debug("    sizeof(std::string) = %u bytes", sizeof(std::string));
            lx_debug("    sizeof(Engine) = %u bytes", sizeof(Engine));
            lx_debug("    sizeof(Document) = %u bytes", sizeof(Document));
            lx_debug("    sizeof(Element) = %u bytes", sizeof(Element));
        }

        _attachSound();
        _attachJavascript();
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
                    "object is freed.");

            // Stop the debugger immediately if a memory leak is detected.
            // It usually is a lot less difficult to track down leaks as 
            // soon as they are introduced.
            lx0::util::lx_break_if_debugging();
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
    Engine::_notifyDocumentCreated (DocumentPtr spDocument)
    {
        _foreach([&] (ComponentPtr spComponent) {
            spComponent->onDocumentCreated(shared_from_this(), spDocument);
        });
    }

    
    DocumentPtr
    Engine::loadDocument (std::string filename)
    {
        DocumentPtr spDocument(new Document);

        _notifyDocumentCreated(spDocument);
        _attachPhysics(spDocument);

        ElementPtr spRoot = _loadDocumentRoot(spDocument, filename);
        if (!spRoot)
            lx_error("Could not load document.  Does file '%s' exist?", filename.c_str());

        spDocument->root(spRoot);


        m_documents.push_back(spDocument);

        // This is properly not the exactly right place for the scripts to be run.
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
                        std::string language = spElem->attr("language").asString();
                        std::string content;
                        if (spElem->value().isDefined())
                        {
                            content = spElem->value().asString();
                        }
                        else
                        {
                            std::string filename = spElem->attr("src").asString();
                            content = lx0::util::lx_file_to_string(filename);
                        }

                        lx_check_error(language.empty() || language == "javascript");
                        _runJavascript(spDocument, content);
                    }
                }
            }
        }
    }

	void   
	Engine::sendMessage (const char* message)
    {
        m_messageQueue.push_back(message);
    }

	int
	Engine::run()
	{
        for(auto it = m_documents.begin(); it != m_documents.end(); ++it)
            (*it)->beginRun();

        bool bDone = false;
        do
        {
            while (!m_messageQueue.empty())
            {
                std::string msg = m_messageQueue.front();
                m_messageQueue.pop_front();

                if (msg == "quit")
                    bDone = true;
            }

            ///@todo Devise a better way to hand time-slices from the main loop to the individual documents
            /// for updates.  Also consider multi-threading.
            for(auto it = m_documents.begin(); it != m_documents.end(); ++it)
                (*it)->updateRun();

        } while (!bDone);

        for(auto it = m_documents.begin(); it != m_documents.end(); ++it)
            (*it)->endRun();

		return 0;
	}

}}