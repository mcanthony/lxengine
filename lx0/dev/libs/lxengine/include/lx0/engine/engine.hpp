//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2012 athile@athile.net (http://www.athile.net)

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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

// Standard headers
#include <memory>
#include <deque>
#include <string>
#include <vector>
#include <map>

#include <boost/thread.hpp>

// Lx headers
#include <lx0/_detail/forward_decls.hpp>
#include <lx0/core/init/version.hpp>
#include <lx0/engine/dom_base.hpp>
#include <lx0/engine/profilemonitor.hpp>
#include <lx0/core/lxvar/lxvar.hpp>
#include <lx0/core/log/log.hpp>

namespace lx0 
{ 
    namespace engine 
    { 
        namespace dom_ns
        {
            namespace detail
            {
                using lx0::lxvar;

                //===========================================================================//
                //!
                /*!
                 */
                class AttributeParser
                {
                public:
                    virtual             ~AttributeParser (void) {}
                    virtual     lxvar   parse               (std::string s) = 0;
                };

                _LX_FORWARD_DECL_PTRS(AttributeParser);


                //===========================================================================//
                //!
                /*!
                 */
                class ObjectCount
                {
                public:
                            ObjectCount ();
                            ObjectCount (size_t current);

                    void    inc      (void);
                    void    dec      (void);
            
                    size_t  current  (void) const { return mCurrent; }
                    size_t  total    (void) const { return mTotal; }

                protected:
                    size_t mCurrent;
                    size_t mTotal;
                };

                //===========================================================================//
                //!
                /*!
                 */
                class EngineComponent : public detail::_ComponentBase
                {
                public:
                    virtual void        onAttached          (EnginePtr spEngine) {}

                    virtual void        onDocumentCreated   (EnginePtr spEngine, DocumentPtr spDocument) {}
                };

                //===========================================================================//
                //!
                /*!
                    \ingroup lx0_engine_dom
                 */
                class WorkerThread
                {
                public:
                            WorkerThread();
                            ~WorkerThread();

                    void    addTask (std::function<void()>);
                    

                protected:
                    void        _run     (void);

                    boost::thread*                    mpThread;
                    boost::condition_variable         mCondition;
                    boost::mutex                      mMutex;
                    std::deque<std::function<void()>> mQueue;
                    volatile bool                     mDone;                    
                };


                struct Profile;
            }

            //===========================================================================//

            typedef std::shared_ptr<std::function<int()>>   EventHandle;


            //===========================================================================//
            //!
            /*!
                \ingroup lx0_engine_dom
             */
            class Engine 
                : public std::enable_shared_from_this<Engine>
                , public detail::_EnableComponentList<Engine, detail::EngineComponent>
            {
            public:               
                class Environment
                {
                public:
                                    Environment     (void);

                    float           timeScale       (void) const    { return mTimeScale;}
                    void            setTimeScale    (float s);

                protected:
                    float       mTimeScale;
                };

                ///@name Version
                ///@{
                int                 versionMajor        (void) const                { return lx0::LXENGINE_VERSION_MAJOR; }
                int                 versionMinor        (void) const                { return lx0::LXENGINE_VERSION_MINOR; }
                int                 versionRevision     (void) const                { return lx0::LXENGINE_VERSION_REVISION; }
                ///@}

                //! Acquire the Singleton for the Engine
                static EnginePtr    acquire             (void);
             
                void                loadPlugin          (std::string name);
        
                void                initialize          (void);
                void                shutdown            (void);
                bool                isShuttingDown      (void) const;

                lx0::uint32         generateId          (void);

                lxvar               getSystemInfo       (void);
                bool                parseCommandLine    (int argc, char** argv, const char* defArgumentName = nullptr);

                void                addResourceDirectory(std::string path);
                std::string         findResource        (std::string name);

                lxvar&              globals             (void)                      { return mGlobals; }
                Environment&        environment         (void)                      { return mEnvironment; }

                DocumentPtr         createDocument      (void);
                DocumentPtr         loadDocument        (std::string filename);
                void                closeDocument       (DocumentPtr& spDocument);
                const std::vector<DocumentPtr>& documents (void) { return mDocuments; }

                void                sendEvent           (std::string evt);
                void                sendEvent           (const char* evt)           { sendEvent(std::string(evt)); }
                void                sendEvent           (std::function<int()> f);
                void                sendEvent           (std::function<int()> f, EventHandle& handle);
                void                sendTask            (std::function<void()> f);
                void                sendTask            (unsigned int delay, std::function<void()> f);
                void                sendWorkerTask      (std::function<void()> f);

                int	                run                 (void);

                ///@name Attribute Parsing
                ///@{
                void                addAttributeParser  (std::string attr, std::function<lxvar(std::string)> parser);
                void                addPsuedoAttribute  (std::string attr, std::function<bool(std::string)> parser);

                lxvar               parseAttribute      (std::string name, std::string value);
                ///@}

                ///@name Engine plug-ins
                ///@{
                void                addViewPlugin           (std::string name, std::function<ViewImp*(View*)> ctor);
                ViewImp*            _createViewImp          (std::string name, View* pView);
                
                void                registerViewComponent   (std::string name, std::function<lx0::ViewComponent*()> ctor);
                lx0::ViewComponent* createViewComponent     (std::string name);
        
                void                addDocumentComponent (std::string name, std::function<DocumentComponent* ()> ctor);
        
                typedef std::function<ElementComponent*(ElementPtr spElem)>      ElementComponentCtor;
                typedef std::pair<std::string,ElementComponentCtor>              ElementComponentPair;
                typedef std::map<std::string, std::vector<ElementComponentPair>> ElementComponentMap;

                void                        addElementComponent     (std::string tag, std::string name, ElementComponentCtor ctor);
                const ElementComponentMap&  elementComponents       (void) const  { return mElementComponents; }
                ///@}

                void                notifyAttached      (ComponentPtr spComponent);

                lx0::slot<void()>   slotRunBegin;
                lx0::slot<void()>   slotRunEnd;
                lx0::slot<void()>   slotIdle;

                ///@name Counters and Statistics
                ///@{
                void                incObjectCount          (std::string name);
                void                decObjectCount          (std::string name);
                const detail::ObjectCount& objectCount      (std::string name) { return mObjectCounts[name]; }
                void                registerProfileCounter  (const char* name, int* pId);
                void                addProfileRelationship  (const char* parentName, const char* childName);
                ///@}



            protected:
                template <typename T> friend std::shared_ptr<T> lx0::detail::acquireSingleton (std::weak_ptr<T>&);

                typedef std::map<std::string,std::function<void(lx0::lxvar)>> FunctionMap;

                struct Event
                {
                    std::string                         message;
                    std::function<void()>               task;
                    std::function<int()>                func;
                    std::weak_ptr<std::function<int()>> wpFunc;
                };

                Engine();
                ~Engine(); 

                void                        _registerBuiltInPlugins     (void);

                DocumentPtr                 _loadDocument               (bool bCreate, std::string filename);
                ElementPtr                  _loadDocumentRoot           (DocumentPtr spDocument, std::string filename);
        
                void                        _handlePlatformMessages     (bool& bDone, bool& bIdle);

                lxvar                               mSystemInfo;
                lxvar                               mGlobals;
                Environment                         mEnvironment;
                lx0::uint32                         mIdCounter;

                // Filesystem
                std::vector<std::string>            mResourceDirectories;
                
                bool                                mbShutdownRequested;
                std::vector<DocumentPtr>            mDocuments;

                boost::mutex                        mEventQueueMutex;
                std::deque<Event>                   mEventQueue;
                std::map<unsigned int,Event>        mDelayedEventQueue;
                std::deque<Event>                   mFrameEventQueue;
                unsigned int                        mFrameDuration;
                unsigned int                        mFrameStart;
                unsigned int                        mFrameTime;
                std::vector<detail::WorkerThread*>  mWorkerThreads;

                FunctionMap                         mFunctions;

                lx0::uint32                         mFrameNum;
                lx0::uint32                         mLoopStartMs;

                std::map<std::string, std::function<ViewImp*(View*)>>                   mViewImps;
                std::map<std::string, std::function<DocumentComponent* ()>>             mDocumentComponents;
                ElementComponentMap                                                     mElementComponents;
                std::map<std::string, std::function<lx0::ViewComponent*()>>             mViewComponents;

                std::map<std::string, std::vector<std::function<bool(std::string)>>>    m_psuedoAttributes;
                std::map<std::string, std::vector<std::function<lxvar(std::string)>>>   m_attributeParsers;

                
                ProfileMonitor                              mProfileMonitor;                
                struct detail::Profile*                     mpProfile;
                std::map<std::string, detail::ObjectCount>  mObjectCounts;
            };
        }
    }
    using namespace lx0::engine::dom_ns;
}
