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
#include <algorithm>

// OpenAL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

// Ogg Vorbis
#include <vorbis/vorbisfile.h>

// Lx
#include <lx0/lxengine.hpp>
#include <lx0/engine/engine.hpp>
#include <lx0/engine/document.hpp>
#include <lx0/engine/element.hpp>
#include <lx0/engine/mesh.hpp>
#include <lx0/util/misc/util.hpp>

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
        ~SoundEngine();

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
                SoundBufferElem(ElementPtr spElem);
                ~SoundBufferElem();

        virtual void    onAttributeChange   (ElementPtr spElem, std::string name, lxvar value);

        ALuint  buffer          (void) const { return mBuffer; }
    
    protected:
        void    _releaseBuffer  (void);
        void    _reset          (ElementPtr spElem);
        void    _loadWav        (std::string filename);
        void    _loadOgg        (std::string filename);

        ALuint  mBuffer;
    };

    //-----------------------------------------------------------------------//
    //! A point in space emitting sound in the scene
    /*!
     */
    class SoundSourceElem : public Element::Component
    {
    public:
                SoundSourceElem(ElementPtr spElem);
                ~SoundSourceElem();

        virtual void    onAttributeChange   (ElementPtr spElem, std::string name, lxvar value);

    protected:
        void    _release        (void);
        void    _reset          (ElementPtr spElem);
        void    _changeState    (lxvar value);

        ALuint          mSource;
        glgeom::point3f mPosition;
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
        alutInit(0, NULL);

        // Clear the error bit
        alGetError();
        lx_check_error (alGetError() == AL_NO_ERROR);
    }

    SoundEngine::~SoundEngine()
    {
        alutExit();
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
            spElem->attachComponent("SoundBuffer", new SoundBufferElem(spElem));
        else if (tag == "SoundSource")
            spElem->attachComponent("SoundSource", new SoundSourceElem(spElem));
    }

    //===========================================================================//
    // SoundBufferElem
    //===========================================================================//

    SoundBufferElem::SoundBufferElem(ElementPtr spElem)
        : mBuffer (0)
    {
        _reset(spElem);
    }

    SoundBufferElem::~SoundBufferElem()
    {
        _releaseBuffer();
    }

    void
    SoundBufferElem::_releaseBuffer()
    {
        if (mBuffer != 0)
        {
            alDeleteBuffers(1, &mBuffer);
            mBuffer = 0;
        }
    }

    void
    SoundBufferElem::onAttributeChange (ElementPtr spElem, std::string name, lxvar value) 
    {
        // Reset everything on an attribute change.  This is a bit heavy-weight, but
        // simpler until there's a use case for SoundBuffer elements being changed
        // with frequency.
        //
        _reset(spElem);
    }

    void
    SoundBufferElem::_loadWav (std::string filename)
    {
        ALenum format;
        ALsizei size;
        ALvoid* data;
        ALsizei freq;
        ALboolean loop;
        ALbyte* pFilename = (ALbyte*)filename.c_str();
        alutLoadWAVFile(pFilename, &format, &data, &size, &freq, &loop);
        alBufferData(mBuffer, format, data, size, freq);
        alutUnloadWAV(format, data, size, freq);
    }
    
    /*!
        Developer Notes:

        This needs to be converted to handle large files as streams to
        reduce the unnecessary memory footprint:
        http://www.devmaster.net/articles/openal-tutorials/lesson8.php
     */
    void
    SoundBufferElem::_loadOgg (std::string filename)
    {
        lx_debug("Loading ogg file '%s'", filename.c_str());

        // Open for binary reading
        FILE* fp = fopen(filename.c_str(), "rb");
        if (fp)
        {
            OggVorbis_File oggFile;
            ov_open(fp, &oggFile, NULL, 0);

            // Read the header info
            vorbis_info* pInfo = ov_info(&oggFile, -1);
            ALenum format = (pInfo->channels == 1)
                    ? AL_FORMAT_MONO16
                    : AL_FORMAT_STEREO16;
            ALsizei freq = pInfo->rate;

            // Read the data
            std::vector<char> buffer;
            long bytesRead;
            do 
            {
                const size_t kChunk = 4096 * 4;
                buffer.resize(buffer.size() + kChunk);

                const int kEndian = 0;
                const int kWordSize = 2;    // Always use 16-bit samples
                const int kSigned = 1;      // true / false for signed data
                int bitStream;
                bytesRead = ov_read(&oggFile, 
                                    &buffer[buffer.size() - kChunk], kChunk, 
                                    kEndian, kWordSize, kSigned, 
                                    &bitStream);

                // Keep the buffer size in sync with what was read
                buffer.resize( buffer.size() - (kChunk - bytesRead) );

            } while (bytesRead > 0);

            ov_clear(&oggFile);

            alBufferData(mBuffer, format, 
                         &buffer[0], static_cast<ALsizei>(buffer.size()), 
                         freq);
            lx_check_error (alGetError() == AL_NO_ERROR);
        }
    }

    void    
    SoundBufferElem::_reset (ElementPtr spElem)
    {
        _releaseBuffer();

        std::string filename = spElem->attr("src").query("");
        std::string type = spElem->attr("type").query("");

        if (!filename.empty())
        {
            if (type.empty())
            {
                std::string ext = filename.substr(filename.length() - 4, 4);
                std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

                if (ext == ".wav")
                    type = "wav";
                else if (ext == ".ogg")
                    type = "ogg";
                else
                    lx_error("No type specified and could not determine type from extension for '%s'", filename.c_str());
            }

            lx_check_error (alGetError() == AL_NO_ERROR);

            alGenBuffers(1, &mBuffer);
            lx_check_error (alGetError() == AL_NO_ERROR);

            if (type == "wav")
                _loadWav(filename);
            else if (type == "ogg")
                _loadOgg(filename);
            else
                lx_error("Unrecognized type: '%s'", type.c_str());
        }
    }


    //===========================================================================//
    // SoundSourceElem
    //===========================================================================//

    SoundSourceElem::SoundSourceElem(ElementPtr spElem)
        : mSource   (0)
    {
        _reset(spElem);
    }

    SoundSourceElem::~SoundSourceElem()
    {
        _release();
    }

    void
    SoundSourceElem::_release ()
    {
        if (mSource != 0)
        {
            alDeleteSources(1, &mSource);
            mSource = 0;
        }
    }


    void
    SoundSourceElem::onAttributeChange (ElementPtr spElem, std::string name, lxvar value) 
    {
        if (name == "sound_state")
            _changeState(value);
        else
            _reset(spElem);
    }

    void
    SoundSourceElem::_reset (ElementPtr spElem)
    {
        lx_check_error(alGetError() == AL_NO_ERROR);

        _release();

        ALuint buffer = 0;

        std::string ref = spElem->attr("ref").query("");
        if (!ref.empty())
        {
            ElementPtr spSource = spElem->document()->getElementById(ref);
            if (spSource.get())
            {
                auto spBuffer = spSource->getComponent<SoundBufferElem>("SoundBuffer");
                buffer = spBuffer->buffer();
            }
        }

        std::string state = spElem->attr("state").query("stopped");

        float volume = spElem->attr("volume").query(1.0f);
        bool  bLoop = spElem->attr("loop").query(0) ? true : false;

        if (buffer)
        {
            alGenSources(1, &mSource);
            lx_check_error(alGetError() == AL_NO_ERROR);

            glgeom::vector3f velocity(0, 0, 0);
            alSourcei (mSource, AL_BUFFER,   buffer);
            alSourcef (mSource, AL_PITCH,    1.0f );
            alSourcef (mSource, AL_GAIN,     volume);
            alSourcefv(mSource, AL_POSITION, &mPosition.x);
            alSourcefv(mSource, AL_VELOCITY, &velocity.x);
            alSourcei (mSource, AL_LOOPING,  bLoop );

            lx_check_error(alGetError() == AL_NO_ERROR);

             _changeState(spElem->attr("sound_state"));
        }
    }

    void
    SoundSourceElem::_changeState (lxvar value)
    {
        if (mSource)
        {
            int state;
            if (value.isString())
            {
                std::string s = *value;
                if (s == "playing")
                    state = 1;
                else if (s == "stopped")
                    state = 0;
                else if (s == "paused")
                    state = 2;
            }
            else 
                state = 0;

            lx_check_error(alGetError() == AL_NO_ERROR);
            switch (state)
            {
            case 0:
                alSourceStop(mSource);
                break;
            case 1:
                alSourcePlay(mSource);
                break;
            case 2:
                alSourcePause(mSource);
                break;
            }   
            lx_check_error(alGetError() == AL_NO_ERROR);
        }
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
