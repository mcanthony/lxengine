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

// Standard headers
#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

// OpenAL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

// Lx
#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/mesh.hpp>
#include <lx0/util.hpp>
#include <lx0/cast.hpp>

using namespace lx0::core;

namespace lx0 { namespace core { namespace detail {

    //===========================================================================//
    // Declarations
    //===========================================================================//

    //-----------------------------------------------------------------------//
    //! 
    /*!
     */
    class SoundEngineBootstrap : public Engine::Component
    {
    public:
        virtual void        onDocumentCreated   (EnginePtr spEngine, DocumentPtr spDocument);
    };

    //-----------------------------------------------------------------------//
    //! 
    /*!
     */
    class SoundEngine : public Engine::Component
    {
    public:
        SoundEngine();

        virtual void        onDocumentCreated   (EnginePtr spEngine, DocumentPtr spDocument);

    protected:
        ALfloat     mListenerPosition[3];
        ALfloat     mListenerVelocity[3];
        ALfloat     mListenerOrientation[6];    // Look vector followed by Up vector
    };

    //-----------------------------------------------------------------------//
    //! 
    /*!
     */
    class SoundDoc : public Document::Component
    {
    public:
        virtual void    onElementAdded      (DocumentPtr spDocument, ElementPtr spElem);
    };

    //-----------------------------------------------------------------------//
    //! Sound data as used by sound sources 
    /*!
     */
    class SoundBufferElem : public Element::Component
    {
    public:
                SoundBufferElem();
                ~SoundBufferElem();
    protected:
        ALuint  mBuffer;
    };

    //-----------------------------------------------------------------------//
    //! A point in space emitting sound in the scene
    /*!
     */
    class SoundSourceElem : public Element::Component
    {
    public:
                SoundSourceElem();
    protected:
        ALuint      mSource;
        point3      mPosition;
        vector3     mVelocity;
    };


    //===========================================================================//
    // SoundEngineBootStrap
    //===========================================================================//

    void 
    SoundEngineBootstrap::onDocumentCreated (EnginePtr spEngine, DocumentPtr spDocument)
    {
        // Now that a document has been created, remove the bootstrap and replace
        // it with the full sound engine, and forward the event.
        //
        auto spThis = shared_from_this();
        spEngine->removeComponent("soundBootstrap");

        auto pSound = new SoundEngine;
        spEngine->attachComponent("sound", pSound);
        pSound->onDocumentCreated(spEngine, spDocument);
    }

    //===========================================================================//
    // SoundEngine
    //===========================================================================//

    SoundEngine::SoundEngine()
    {
        lx_debug("SoundEngine ctor");

        mListenerPosition[0] = 0.0f;
        mListenerPosition[1] = 0.0f;
        mListenerPosition[2] = 0.0f;

        mListenerVelocity[0] = 0.0f;
        mListenerVelocity[1] = 0.0f;
        mListenerVelocity[2] = 0.0f;

        mListenerOrientation[0] = -1.0f;     // Forward
        mListenerOrientation[1] = 0.0f;
        mListenerOrientation[2] = 0.0f;
        mListenerOrientation[3] = 0.0f;     // Up
        mListenerOrientation[4] = 0.0f;
        mListenerOrientation[5] = 1.0f;


        // Initialize ALUT
        //alutInit(0, NULL);

        // Clear the error bit
        alGetError();
        lx_check_error (alGetError() == AL_NO_ERROR);
    }

    void 
    SoundEngine::onDocumentCreated (EnginePtr spEngine, DocumentPtr spDocument)
    {
        spDocument->attachComponent("sound", new SoundDoc);
    }

    //===========================================================================//
    // SoundDoc
    //===========================================================================//

    void
    SoundDoc::onElementAdded (DocumentPtr spDocument, ElementPtr spElem)
    {
        std::string tag = spElem->tagName();

        if (tag == "SoundBuffer")
            spElem->attachComponent("sound", new SoundBufferElem);
    }

    //===========================================================================//
    // SoundBufferElem
    //===========================================================================//

    SoundBufferElem::SoundBufferElem()
        : mBuffer (0)
    {
        alGenBuffers(1, &mBuffer);
        lx_check_error (alGetError() == AL_NO_ERROR);
    }

    SoundBufferElem::~SoundBufferElem()
    {
        if (mBuffer != 0)
        {
            alDeleteBuffers(1, &mBuffer);
        }
    }

    //===========================================================================//
    // SoundSourceElem
    //===========================================================================//

    SoundSourceElem::SoundSourceElem()
        : mPosition (0, 0, 0)
        , mVelocity (0, 0, 0)
    {
    }

}}}

namespace lx0 { namespace core { 
    
    using namespace detail;

    void        
    Engine::_attachSound (void)
    {
        attachComponent("soundBootstrap", new SoundEngineBootstrap);
    }
}}
