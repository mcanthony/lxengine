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
#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include <lx0/lxengine.hpp>
#include <lx0/engine/engine.hpp>
#include <lx0/engine/document.hpp>
#include <lx0/engine/element.hpp>
#include <lx0/util/misc/util.hpp>
#include <lx0/elements/core.hpp>

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


        WorkerThread::WorkerThread()
            : mDone (false)
        {
            mpThread = new boost::thread([&]() { _run(); });
        }

        WorkerThread::~WorkerThread (void)
        {
            addTask([&]() { mDone = true; });
            mpThread->join();
            delete mpThread;
        }          

        void 
        WorkerThread::addTask (std::function<void()> f)
        {
            boost::lock_guard<boost::mutex> lock(mMutex);
            mQueue.push_back(f);
            mCondition.notify_one();
        }
    
        void
        WorkerThread::_run (void)
        {
            lx_current_thread_priority_below_normal();

            while (!mDone)
            {
                boost::unique_lock<boost::mutex> lock(mMutex);
                while (mQueue.empty())
                    mCondition.wait(lock);

                auto f = mQueue.front();
                mQueue.pop_front();

                f();
            }
        }  
    }

    using namespace detail;

    Engine::Environment::Environment ()
        : mTimeScale (1.0f)
    {
    }

    void
    Engine::Environment::setTimeScale (float s)
    {
        if (s < 0.0f)
            throw lx_error_exception("Time scale cannot be negative.  '%f' is not a valid value.", s);
        else
            mTimeScale = s;
    }

    EnginePtr 
    Engine::acquire (void) 
    {
        //
        // Get the pointer to the singleton using the helper _lx_get_engine_singleton() so that
        // singleton is unique across the entire *process* rather than unique across each 
        // module.  (See http://athile.net/library/blog/?p=1346).
        //
        // Keep a per-module copy of pointer simply for efficiency.  Engine::acquire() is called
        // with relative frequency.
        //        
        static std::weak_ptr<Engine>* pwpEngine = NULL;
        if (!pwpEngine)
            pwpEngine = _lx_get_engine_singleton();
        
        return lx0::detail::acquireSingleton<Engine>(*pwpEngine); 
    }

    Engine::Engine()
        : mGlobals            (lxvar::decorated_map())
        , mIdCounter          (0)
        , mbShutdownRequested (false)
    {
        lx_init();
        lx_log("lx::core::Engine ctor");

        //
        // This seems like a reasonable idea - eventually.  A configuration file should be
        // able to determine what subsystems are loaded (and their order) and the globals
        // table may be the way to communicate that information.
        //
        mGlobals.add("load_builtins", 0, validate_readonly(), lxvar::decorated_map());
        mGlobals["load_builtins"].add("sound",      0, validate_bool(), true);
        mGlobals["load_builtins"].add("javascript", 0, validate_bool(), true);
        mGlobals["load_builtins"].add("Canvas",     0, validate_bool(), true);
        mGlobals["load_builtins"].add("Ogre",       0, validate_bool(), false);

        lxvar info = getSystemInfo();
        lx_debug("%s", lx0::format_tabbed(info).c_str());       
    }

    /*!
        
     */
    void
    Engine::_registerBuiltInPlugins (void)
    {
        // Forward declarations inlined here to avoid pulling in 
        // complex headers that don't really belong in engine.cpp
        //
        // Obviously, this needs future clean-up...
        //
        lx0::ViewImp*           _hidden_createCanvasViewImp   (lx0::View* pView);

        auto& var = mGlobals["load_builtins"];

        if (var["sound"].as<bool>())
            loadPlugin("SoundAL");
        if (var["Canvas"].as<bool>())
            addViewPlugin("Canvas", _hidden_createCanvasViewImp);
        if (var["Ogre"])
            loadPlugin("OgreView");        
    }

    void 
    Engine::loadPlugin (std::string name)
    {
        lx_load_plugin(name.c_str());
    }

    void
    Engine::notifyAttached (ComponentPtr spComponent) 
    { 
        spComponent->onAttached(shared_from_this()); 
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

    void
    Engine::initialize()
    {
        // Convenience code to reset the working path automatically.
        if (lx_in_debugger())
            _lx_change_current_path_to_lx_root();

        _registerBuiltInPlugins();
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
        mComponents.clear();

        // All Documents should be gone by shutdown...
        lx_assert( objectCount("Document").current() == 0 );
    }

    Engine::~Engine()
    {
       lx_log("lx::core::Engine dtor");

       // Don't throw exceptions in a destructor!
       if (!mDocuments.empty())
           lx_log("Engine.Shutdown: Documents not cleaned up correctly.");

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

    lx0::uint32
    Engine::generateId (void)
    {
        return ++mIdCounter;
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
    Engine::decObjectCount (std::string name)
    {
        auto it = m_objectCounts.find(name);
        
        if (it != m_objectCounts.end())
        {
            if (!(it->second.current() >= 1))
                lx_warn("Object count for '%s' is unexpectedly less than 1!", name.c_str());

            it->second.dec();
        }
        else
            lx_warn("Decrementing object count on '%s' but no entry for that name.", name.c_str());
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
        m_postponedExceptions.push_back(e);
    }

    lxvar
    Engine::getSystemInfo (void)
    {
        // Cache the system info the first time it is computed
        if (!mSystemInfo.is_defined())
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
            info["sizes"]["std::vector"] =     (int)sizeof(std::vector<int>);
            info["sizes"]["Document"] =        (int)sizeof(Document);
            info["sizes"]["Element"] =         (int)sizeof(Element);
            info["sizes"]["lxvar"] =           (int)sizeof(lxvar);
            info["sizes"]["lx0::slot"] =       (int)sizeof(lx0::slot<void()>);

            info["system"] = lxvar::ordered_map();
            info["system"]["current_time"] = lx_ctime();
            info["system"]["debugger_active"] = lx_in_debugger() ? 1 : 0;
            info["system"]["endian"] = lx_little_endian() ? "little" : "big";

            info["system"]["operating_system"] = lxvar::ordered_map();
            lx_operating_system_info(info["system"]["operating_system"]);
            lx_display_info(info["system"]["display"]);
            
            info["lxengine"] = lxvar::ordered_map();
            info["lxengine"]["version"] = boost::str( boost::format("%d.%d.%d") % versionMajor() % versionMinor() % versionRevision() ); 
            
            info["build"] = lxvar::ordered_map();
            info["build"]["date"] = boost::str( boost::format("%s %s") % __DATE__ % __TIME__ ); 
            info["build"]["compiler_version"] = boost::str( boost::format("0x%04x") % _MSC_VER ); 
        }
        return mSystemInfo.clone();
    }


    /*!
        Populates the command-line with all the Engine globals.
     */
    bool 
    Engine::parseCommandLine (int argc, char** argv, const char* defaultArgument)
    {
        //
        // See http://www.boost.org/doc/libs/1_44_0/doc/html/program_options/tutorial.html
        //
        using namespace boost::program_options;

        //
        // Set up the command-line options data structure
        //  
        std::string caption = boost::str( boost::format("Syntax: %1% [options] <file>.\nOptions") % argv[0] );
        options_description desc (caption);
    
        auto& adder = desc.add_options();
        adder("help", "Print usage information and exit.");
    
        for (auto it = mGlobals.begin(); it != mGlobals.end(); ++it)
        {
            std::string name = it.key();
            const lx0::uint32 flags = mGlobals.flags(name.c_str());

            if (flags & eAcceptsString)
                adder(name.c_str(), value<std::string>());
            else if (flags & eAcceptsInt)
                adder(name.c_str(), value<int>());
        }

        //
        // Support for a default argument, e.g.:
        //
        // ./myapp --filename=somefile.txt
        //
        // ./myapp somefile.txt
        //
        positional_options_description pos;
        if (defaultArgument)
            pos.add(defaultArgument, -1);

        //
        // Parse the options
        //
        bool bFailed = false;
        variables_map vars;
        try
        {
            store(command_line_parser(argc, argv).options(desc).positional(pos).run(), vars);
        }
        catch (...)
        {
            bFailed = true;
        }

        //
        // Now check and set the options
        //
        if (bFailed || vars.count("help"))
        {
            std::cout << desc << std::endl;
            return false;
        }

        for (auto it = mGlobals.begin(); it != mGlobals.end(); ++it)
        {
            std::string       nameStr  = it.key();              // Keep a copy so the c_str() doesn't expire!
            const char*       name     = nameStr.c_str();
            const lx0::uint32 flags    = mGlobals.flags(name);

            if (vars.count(name) > 0)
            {
                if (flags & eAcceptsString)
                    mGlobals[name] = vars[name].as<std::string>();
                else if (flags & eAcceptsInt)
                    mGlobals[name] = vars[name].as<int>();
            }
        }

        return true;
    }
   
    DocumentPtr
    Engine::createDocument (void)
    {
        lx_log("Creating new document.");

        return _loadDocument(true, std::string());
    }

    DocumentPtr
    Engine::loadDocument (std::string filename)
    {
        lx_log("Loading document '%s'", filename.c_str());

        return _loadDocument(false, filename);
    }

    /*!
        \todo Design choice: do documents need to be explicitly closed?  Can they be implicitly closed
            when the Engine is released?
     */
    void
    Engine::closeDocument (DocumentPtr& spDocument)
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
            throw lx_error_exception("Document being closed but not in Engine Document list! Was this Document already closed?");

        // Free the incoming pointer
        auto count = spDocument.use_count();
        spDocument.reset();
    }

    DocumentPtr
    Engine::_loadDocument (bool bCreate, std::string filename)
    {
        //
        // Create the empty document and send out notification to all Engine Components
        //
        DocumentPtr spDocument(new Document);
        mDocuments.push_back(spDocument);

        _foreach([&] (ComponentPtr spComponent) {
            spComponent->onDocumentCreated(shared_from_this(), spDocument);
        });

        //
        // Attach all registered per-document Components
        //
        for (auto it = mDocumentComponents.begin(); it != mDocumentComponents.end(); ++it)
        {
            auto pComponent = (it->second)();
            spDocument->attachComponent(pComponent);
        }

        //
        // Load the document data
        //
        ElementPtr spRoot;
        if (!bCreate)
        {
            try 
            {
                spRoot = _loadDocumentRoot(spDocument, filename);
            } 
            catch (lx0::error_exception& e)
            {
                // Clean-up the local changes, then pass the exception along
                mDocuments.erase( std::find(mDocuments.begin(), mDocuments.end(), spDocument) );
                spDocument.reset();
                throw e;
            }

            if (!spRoot)
            {
                throw lx_error_exception("Could not load document.  Does file '%s' exist?", filename.c_str());
            }
        }
        else
            spRoot = spDocument->createElement("Document");

        spDocument->root(spRoot);

        return spDocument;
    }

	void   
	Engine::sendEvent (std::string message)
    {
        if ("quit" == message)
            lx_debug("Message 'quit' sent to Engine");

        Event evt;
        evt.message = message;
        m_eventQueue.push_back(evt);
    }

	void   
	Engine::sendTask (std::function<void()> f)
    {
        Event evt;
        evt.task = f;
        m_eventQueue.push_back(evt);
    }

    void 
    Engine::sendWorkerTask (std::function<void()> f)
    {
        mWorkerThreads[0]->addTask(f);
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

    bool
    Engine::isShuttingDown (void) const
    {
        return mbShutdownRequested;
    }

	int
	Engine::run()
	{
        slotRunBegin();

        const lx0::uint64 start = lx0::lx_milliseconds();
        mFrameNum = 0;

        _lx_reposition_console();

        //
        // Launch the worker thread
        //
        mWorkerThreads.push_back( new detail::WorkerThread );

        //
        // Signal to the Document components that the main loop is about
        // to begin
        //
        for(auto it = mDocuments.begin(); it != mDocuments.end(); ++it)
            (*it)->beginRun();

        bool bDone = false;
        do
        {
            mFrameStartMs = lx0::lx_milliseconds();

            bool bIdle = true;

            _throwPostponedException();

            while (!m_eventQueue.empty())
            {
                bIdle = false;

                Event evt = m_eventQueue.front();
                m_eventQueue.pop_front();

                int validity = (evt.message.empty() ? 0 : 1) + (evt.task ? 1 : 0);
                switch (validity)
                {
                case 0:
                    lx_warn("Ignoring empty event (neither a message or task is set)");
                    break;
                case 2:
                    throw lx_error_exception("Overspecified event! Both a message and a task are set on the event.");
                    break;

                case 1:
                    {
                        if (evt.message == "quit")
                            bDone = true;
                        if (evt.task)
                            evt.task();
                    }
                    break;
                }
            }

            _handlePlatformMessages(bDone, bIdle);
            _throwPostponedException();

            if (bIdle)
                slotIdle();

            ///@todo Devise a better way to hand time-slices from the main loop to the individual documents
            /// for updates.  Also consider multi-threading.
            {
                const auto startLoop = lx0::lx_milliseconds();

                for(auto it = mDocuments.begin(); it != mDocuments.end(); ++it)
                    (*it)->updateRun();

                incPerformanceCounter("Engine>run>doc update", lx0::lx_milliseconds() - startLoop);
            }

            incPerformanceCounter("Engine>run>loop", lx0::lx_milliseconds() - mFrameStartMs);

            //
            // Engine's notion of a frame is one full cycle through the main loop.
            // Views may have different notions is there is not a 1:1 on the redraw
            // and main loop.
            //
            mFrameNum++;

        } while (!bDone);

        mbShutdownRequested = true;

        for(auto it = mDocuments.begin(); it != mDocuments.end(); ++it)
            (*it)->endRun();

        incPerformanceCounter("Engine>run", lx0::lx_milliseconds() - start);

        slotRunEnd();

        //
        // Finish all worker threads.  The destructor will *wait* for any pending
        // tasks to run before returning.
        //
        for (auto it = mWorkerThreads.begin(); it != mWorkerThreads.end(); ++it)
            delete (*it);

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
            throw lx_error_exception("Registering View plugin '%s': there is already a plugin with that name.", name.c_str()); 
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
            throw lx_error_exception("No View plug-in with name '%s' found.", name.c_str()); 
            return nullptr;
        }
    }

    void 
    Engine::registerViewComponent (std::string name, std::function<lx0::ViewComponent*()> ctor)
    {
        mViewComponents.insert( std::make_pair(name, ctor) );
    }

    lx0::ViewComponent*
    Engine::createViewComponent (std::string name)
    {
        auto it = mViewComponents.find(name);
        auto pComponent = (it->second)();
        return pComponent;
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
