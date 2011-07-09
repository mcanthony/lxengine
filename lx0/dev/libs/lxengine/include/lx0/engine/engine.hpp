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

// Lx headers
#include <lx0/_detail/forward_decls.hpp>
#include <lx0/core/init/version.hpp>
#include <lx0/engine/dom_base.hpp>
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
                    virtual void        onDocumentCreated   (EnginePtr spEngine, DocumentPtr spDocument) {}
                };

            }

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
                static EnginePtr    acquire             (void) { return lx0::detail::acquireSingleton<Engine>(s_wpEngine); }
        
                void                shutdown            (void);

                lxvar               getSystemInfo       (void);
                bool                parseCommandLine    (int argc, char** argv, const char* defArgumentName = nullptr);

                Environment&        environment         (void)                      { return mEnvironment; }

                DocumentPtr         createDocument      (void);
                DocumentPtr         loadDocument        (std::string filename);
                void                closeDocument       (DocumentPtr& spDocument);
        
                const std::vector<DocumentPtr>& documents (void) { return mDocuments; }

                void                sendEvent           (const char* evt);
                int	                run                 (void);

                lxvar&              globals             (void) { return mGlobals; }

                ///@name Attribute Parsing
                ///@{
                void                addAttributeParser  (std::string attr, std::function<lxvar(std::string)> parser);
                void                addPsuedoAttribute  (std::string attr, std::function<bool(std::string)> parser);

                lxvar               parseAttribute      (std::string name, std::string value);
                ///@}

                ///@name Engine plug-ins
                ///@{
                void                addViewPlugin       (std::string name, std::function<ViewImp*(View*)> ctor);
                ViewImp*            _createViewImp      (std::string name, View* pView);
        
                void                addDocumentComponent (std::string name, std::function<DocumentComponent* ()> ctor);
        
                typedef std::function<ElementComponent*(ElementPtr spElem)>      ElementComponentCtor;
                typedef std::pair<std::string,ElementComponentCtor>              ElementComponentPair;
                typedef std::map<std::string, std::vector<ElementComponentPair>> ElementComponentMap;

                void                        addElementComponent     (std::string tag, std::string name, ElementComponentCtor ctor);
                const ElementComponentMap&  elementComponents       (void) const  { return mElementComponents; }
                ///@}

                void                notifyAttached      (ComponentPtr spComponent) { /*! \todo */ } 

                ///@name Counters and Statistics
                ///@{
                void                incObjectCount          (std::string name);
                void                decObjectCount          (std::string name);
                const detail::ObjectCount& objectCount      (std::string name) { return m_objectCounts[name]; }
                void                incPerformanceCounter   (std::string name, lx0::uint64 t);
                ///@}

                void                postponeException       (lx0::error_exception& e);

                // Development Scaffolding Methods
                void                workaround_runJavascript(DocumentPtr spDoc, const std::string& source);

            protected:
                template <typename T> friend std::shared_ptr<T> lx0::detail::acquireSingleton (std::weak_ptr<T>&);
                static std::weak_ptr<Engine> s_wpEngine;

                Engine();
                ~Engine(); 

                DocumentPtr _loadDocument           (bool bCreate, std::string filename);
                ElementPtr  _loadDocumentRoot       (DocumentPtr spDocument, std::string filename);

                void        _registerBuiltInPlugins (void);
                void        _attachJavascript       (void);
                void        _runJavascript          (DocumentPtr spDocument, std::string source);
        
                void        _throwPostponedException (void);
                bool        _handlePlatformMessages  (void);

                
                lxvar                       mSystemInfo;
                
                typedef std::map<std::string,std::function<void(lx0::lxvar)>> FunctionMap;
                lxvar                       mGlobals;
                FunctionMap                 mFunctions;

                Environment                 mEnvironment;
                std::vector<DocumentPtr>    mDocuments;
                std::deque<std::string>     m_messageQueue;

                lx0::uint32                 mFrameNum;
                lx0::uint32                 mFrameStartMs;

                std::deque<lx0::error_exception>            m_postponedExceptions;
                std::map<std::string, detail::ObjectCount>  m_objectCounts;
                
                struct PerfCounter
                {
                    PerfCounter(lx0::uint64 e, lx0::uint64 t) : events(e), total (t) {}
                    lx0::uint64 events;
                    lx0::uint64 total;
                };
                std::map<std::string, PerfCounter>          m_perfCounters;

                std::map<std::string, std::function<ViewImp*(View*)>>                   mViewImps;
                std::map<std::string, std::function<DocumentComponent* ()>>             mDocumentComponents;
                ElementComponentMap                                                     mElementComponents;

                std::map<std::string, std::vector<std::function<bool(std::string)>>>    m_psuedoAttributes;
                std::map<std::string, std::vector<std::function<lxvar(std::string)>>>   m_attributeParsers;
            };

        }
    }
    using namespace lx0::engine::dom_ns;
}
