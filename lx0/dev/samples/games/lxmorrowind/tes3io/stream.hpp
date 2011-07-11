//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

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

#include <glgeom/glgeom.hpp>
#include <lx0/_detail/forward_decls.hpp>
#include <fstream>
#include <string>

//===========================================================================//

class Stream
{
public:
    Stream () : mFin(*(new std::ifstream)) { mpOwned = &mFin; }
    Stream (std::ifstream& stream) : mpOwned (nullptr), mFin(stream) {}
    ~Stream() { delete mpOwned; }

    void open (const std::string& filename)
    {
        mFin.open(filename, std::ios::binary | std::ios::in);
    }

    std::ifstream&  stream  (void)              { return mFin; }

    void            close   (void)              { mFin.close(); }
    bool            eof     (void)              { return mFin.eof(); }
    bool            good    (void) const        { return mFin.good(); }
    lx0::uint32     tellg   (void)              { return mFin.tellg(); }
    void            seekg   (lx0::uint32 pos)   { mFin.seekg(pos); assert(!mFin.fail()); }

    void skip (size_t bytes)
    {
        mFin.seekg( bytes, std::ios_base::cur);
    }

    void read (char* data, size_t count)
    {
        mFin.read(data, count);
    }

    void read5 (char* data)
    {
        mFin.read(data, 4);
        data[4] = '\0';
    }

    void read (float* data, size_t count = 1)
    {
        mFin.read((char*)data, sizeof(float) * count);
    }

    void read (lx0::uint16* data, size_t count = 1)
    {
        mFin.read((char*)data, 2 * count);
    }

    void read (lx0::uint32* data, size_t count = 1)
    {
        mFin.read((char*)data, 4 * count);
    }

    std::string read_string(size_t size)
    {
        std::string s;
        if (size > 1)
        {
            s.resize(size - 1);
            read(&s[0], s.size());
        }
        char terminator = read();
        return s;
    }
    
    std::string read_string2(size_t size)
    {
        std::string s;
        if (size > 0)
        {
            s.resize(size);
            read(&s[0], s.size());
        }
        return s;
    }

    struct auto_cast
    {
        auto_cast(Stream& stream) : mStream (stream) {}
        
        operator int         () { int v; mStream.read((char *)&v, 4); return v; }
        operator lx0::uint8  () { lx0::uint8 v; mStream.read((char *)&v, 1); return v; }
        operator lx0::int16  () { lx0::int16 v; mStream.read((char *)&v, 2); return v; }
        operator lx0::uint16 () { lx0::uint16 v; mStream.read((char *)&v, 2); return v; }
        operator lx0::uint32 () { lx0::uint32 v; mStream.read(&v); return v; }
        operator float       () { float v; mStream.read(&v); return v; }
        operator char        () { char c; mStream.read(&c, 1); return c; }
        operator glgeom::point3f () {
            glgeom::point3f p;
            p.x = (*this);
            p.y = (*this);
            p.z = (*this);
            return p;
        }
        operator glm::vec3 () {
            glm::vec3 p;
            p.x = (*this);
            p.y = (*this);
            p.z = (*this);
            return p;
        }
        operator glm::vec4 () {
            glm::vec4 p;
            p.x = (*this);
            p.y = (*this);
            p.z = (*this);
            p.w = (*this);
            return p;
        }
        operator glm::mat3 () {
            glm::mat3 m;
            m[0] = (*this);
            m[1] = (*this);
            m[2] = (*this);
            return m;
        }
        Stream& mStream;
    };

    inline auto_cast read()
    {
        return auto_cast(*this);
    }


protected:
    std::ifstream* mpOwned;
    std::ifstream& mFin;
};

