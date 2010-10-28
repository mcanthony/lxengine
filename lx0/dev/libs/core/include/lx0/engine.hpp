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

#pragma once

#include <memory>
#include <deque>
#include <string>
#include <vector>
#include <map>

#include <lx0/detail/forward_decls.hpp>

namespace lx0 { namespace core {

    namespace detail
    {
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
    };

    //===========================================================================//
    //!
    /*!
     */
    class Engine
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
        static EnginePtr    acquire         () { return detail::acquireSingleton<Engine>(s_wpEngine); }
        
        void                shutdown        (void);

        Environment&        environment     (void)                      { return mEnvironment; }

        DocumentPtr         loadDocument    (std::string filename);
        void                connect         (DocumentPtr spDocument);

        void                sendMessage     (const char* message);
        int	                run             (void);

        // Stats
        void                incObjectCount (std::string name);
        void                decObjectCount (std::string name);

    protected:
        template <typename T> friend std::shared_ptr<T> detail::acquireSingleton (std::weak_ptr<T>&);
        static std::weak_ptr<Engine> s_wpEngine;

        Engine();
        ~Engine(); 

        ElementPtr  _loadDocumentRoot   (DocumentPtr spDocument, std::string filename);
 
        void        _attachPhysics      (DocumentPtr spDocument);
        void        _runJavascript      (DocumentPtr spDocument, std::string source);
        
        Environment                 mEnvironment;
        std::vector<DocumentPtr>    m_documents;
        std::deque<std::string>     m_messageQueue;

        std::map<std::string, detail::ObjectCount>   m_objectCounts;
    };

}}
