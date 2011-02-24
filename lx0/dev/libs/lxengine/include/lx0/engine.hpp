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
#include <lx0/core/detail/forward_decls.hpp>
#include <lx0/core/detail/dom_base.hpp>
#include <lx0/core/data/lxvar.hpp>

namespace lx0 { namespace core {

    namespace detail
    {
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


        //! Acquire the Singleton for the Engine
        static EnginePtr    acquire             (void) { return detail::acquireSingleton<Engine>(s_wpEngine); }
        
        void                shutdown            (void);

        int                 versionMajor        (void)                      { return 0; }
        int                 versionMinor        (void)                      { return 0; }

        Environment&        environment         (void)                      { return mEnvironment; }

        DocumentPtr         loadDocument        (std::string filename);

        void                sendMessage         (const char* message);
        int	                run                 (void);

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
        
        typedef std::map<std::string, std::pair<std::string,std::function<ElementComponent*()>> > ElemCompList;
        void                addElementComponent  (std::string tag, std::string name, std::function<ElementComponent*()> ctor);
        const ElemCompList& elementComponents     (void) const  { return mElementComponents; }
        ///@}

        // Stats
        void                incObjectCount      (std::string name);
        void                decObjectCount      (std::string name);

    protected:
        template <typename T> friend std::shared_ptr<T> detail::acquireSingleton (std::weak_ptr<T>&);
        static std::weak_ptr<Engine> s_wpEngine;

        Engine();
        ~Engine(); 

        ElementPtr  _loadDocumentRoot       (DocumentPtr spDocument, std::string filename);

        void        _notifyDocumentCreated  (DocumentPtr spDocument);
 
        void        _attachSound            (void);
        void        _attachPhysics          (DocumentPtr spDocument);
        void        _attachJavascript       (void);
        void        _runJavascript          (DocumentPtr spDocument, std::string source);
        
        void        _processDocumentHeader  (DocumentPtr spDocument);

        bool        _handlePlatformMessages (void);

        Environment                 mEnvironment;
        std::vector<DocumentPtr>    m_documents;
        std::deque<std::string>     m_messageQueue;

        std::map<std::string, detail::ObjectCount>   m_objectCounts;

        std::map<std::string, std::function<ViewImp*(View*)>>                   mViewImps;
        std::map<std::string, std::function<DocumentComponent* ()>>             mDocumentComponents;
        ElemCompList                                                            mElementComponents;

        std::map<std::string, std::vector<std::function<bool(std::string)>>>    m_psuedoAttributes;
        std::map<std::string, std::vector<std::function<lxvar(std::string)>>>   m_attributeParsers;
    };

}}
