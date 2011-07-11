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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

#include <iostream>
#include <algorithm>
#include <lx0/lxengine.hpp>

#include <fstream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

lx0::View::Component*   create_renderer();

//===========================================================================//

namespace bfs = boost::filesystem;

void loadBsaFile (const std::string& filename)
{
    std::cout << boost::format("Loading file '%s'...\n") % filename;
}

void loadBsaDirectory(const char* path)
{
    lx0::for_files_in_directory(path, "bsa", [](std::string file) {
        loadBsaFile(file);
    });
}

class Stream
{
public:
    void open (const std::string& filename)
    {
        mFin.open(filename, std::ios::binary | std::ios::in);
    }

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

    struct auto_cast
    {
        auto_cast(Stream& stream) : mStream (stream) {}
        
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
        Stream& mStream;
    };

    inline auto_cast read()
    {
        return auto_cast(*this);
    }


protected:
    std::ifstream mFin;
};

struct SubRecord
{
    SubRecord (Stream& stream)
    {
        _read(stream);
    }

    bool test (Stream& stream, const char* expectedName)
    {
        if (strncmp(name, expectedName, 4) == 0)
            return true;
        else
        {
            stream.seekg(offset);
            return false;
        }
    }

    int         offset; 
    char        name[5];
    lx0::uint32 size;

private:
    void _read (Stream& stream)
    {
        offset = stream.tellg();
        stream.read5(name);
        stream.read(&size);
    }
};

struct RecordHeader : public SubRecord
{
    RecordHeader(Stream& stream)
        : SubRecord (stream)
    {
        stream.read(&header);
        stream.read(&flags);
    }

    lx0::uint32 header;
    lx0::uint32 flags;
};

void advanceToData (Stream& fin, const RecordHeader& record)
{
    fin.seekg( record.offset + 16);
}

void advanceToData (Stream& fin, const SubRecord& record)
{
    fin.seekg( record.offset + 8);
}

struct RecordIterator
{
    RecordIterator (Stream& stream)
        : mStream  (stream)
        , mCurrent (stream)
    {
    }

    bool is_name (const char* s)
    {
        return strncmp(s, mCurrent.name, 4) == 0;
    }

    void next()
    {
        mStream.seekg(mCurrent.offset + mCurrent.size + 16);
        new (&mCurrent) RecordHeader(mStream);
    }

    const RecordHeader& operator* () const { return mCurrent; }

    RecordHeader  mCurrent;
    Stream& mStream;
};

struct SubRecordIterator
{
    SubRecordIterator (Stream& stream) 
        : mStream  (stream) 
        , mCurrent (stream)
    {
    }

    bool        is_name(const char* s) { return strncmp(s, mCurrent.name, 4) == 0; }
    lx0::uint32 size() { return mCurrent.size; }
    void next() { mStream.seekg( mCurrent.offset + mCurrent.size + 8); new (&mCurrent) SubRecord(mStream); }

    SubRecord   mCurrent;
    Stream&     mStream;
};

struct Index
{
    Index (Stream& stream)
    {
        //
        // Read the initial header
        //
        // Use this to pre-allocate and prepare for reading the 
        // full index.
        //
        lx0::uint32 recordCount;

        RecordIterator recordIter (stream);
        assert(recordIter.is_name("TES3"));

        SubRecordIterator sub(stream);
        if (sub.is_name("HEDR"))
        {
            float version = stream.read();
            stream.skip(4 + 32 + 256);
            recordCount = stream.read();
            headers.reserve(recordCount);
            recordCount--;
        }
        headers.push_back(*recordIter);
        recordIter.next();

        while (recordCount-- > 0)
        {
            headers.push_back(*recordIter);
            recordIter.next();
        }
    }

    std::vector<RecordHeader>               headers;
    std::vector<std::pair<std::string, RecordHeader*>> cells;
};

void loadCell (Stream& stream, Index& index, const char* name)
{
    struct Sort
    {
        bool operator() (const std::pair<std::string, RecordHeader*>& a,
                         const std::pair<std::string, RecordHeader*>& b) const
        {
            return a.first < b.first;
        }
    };
    auto it = std::lower_bound(index.cells.begin(), index.cells.end(), std::make_pair(std::string(name), (RecordHeader*)nullptr));
    if (it != index.cells.end())
    {
    }
    else
    {
        assert(0);
    }
}

void readCells (Stream& stream, Index& index)
{
    std::vector<std::pair<std::string, RecordHeader*>> cells;
    cells.reserve(1024);

    for (auto it = index.headers.begin(); it != index.headers.end(); ++it)
    {
        if (strncmp(it->name, "CELL", 4) == 0)
        {
            std::string cellName;
            lx0::uint32 flags;

            advanceToData(stream, *it);
            SubRecordIterator sub (stream);
            if (sub.is_name("NAME"))
            {
                cellName = stream.read_string(sub.size());
                sub.next();
            }

            if (!cellName.empty())
                cells.push_back(std::make_pair(cellName, &(*it)));
            continue;

            if (sub.is_name("DATA"))
            {
                flags = stream.read();
                lx0::uint32 gridX = stream.read();
                lx0::uint32 gridY = stream.read();
                sub.next();
            }

            if (flags & 0x01)   // Interior
            {
                if (sub.is_name("WHGT"))
                {
                    lx0::uint32 waterHeight = stream.read();
                    sub.next();
                }
                if (sub.is_name("AMBI"))
                {
                    lx0::uint32 ambientColor = stream.read();
                    lx0::uint32 sunlightColor = stream.read();
                    lx0::uint32 fogColor = stream.read();
                    float fogDensity = stream.read();
                    sub.next();
                }
            }
            else
            {
                if (sub.is_name("NAM5"))
                {
                    lx0::uint32 color = stream.read();
                    sub.next();
                }
            }

            if (sub.is_name("NAME"))
                sub.next();

            if (sub.is_name("RGNN"))
                sub.next();
            if (sub.is_name("NAM0"))
                sub.next();

            while (sub.is_name("FRMR"))
            {
                lx0::uint32 index = stream.read();
                sub.next();

                std::string name;
                glgeom::point3f position;
                glm::vec3       rotation;

                if (sub.is_name("NAME"))
                {
                    name = stream.read_string(sub.size());
                    sub.next();
                }
                if (sub.is_name("XSCL"))
                    sub.next();
                if (sub.is_name("FLTV"))
                    sub.next();
                if (sub.is_name("ANAM"))
                    sub.next();
                if (sub.is_name("BNAM"))
                    sub.next();
                if (sub.is_name("XSOL"))
                    sub.next();
                if (sub.is_name("CNAM"))
                    sub.next();
                if (sub.is_name("INDX"))
                    sub.next();
                if (sub.is_name("XCHG"))
                    sub.next();
                if (sub.is_name("INTV"))
                    sub.next();
                if (sub.is_name("NAM9"))
                    sub.next();
                if (sub.is_name("KNAM"))   // Key NAMe
                    sub.next();
                if (sub.is_name("TNAM"))   // Trap NAMe
                    sub.next();
                if (sub.is_name("UNAM"))   
                    sub.next();
                if (sub.is_name("FLTV"))   
                    sub.next();
                if (sub.is_name("DODT"))
                    sub.next();
                if (sub.is_name("DNAM"))
                    sub.next();
                if (sub.is_name("DATA"))
                {
                    position = stream.read();
                    rotation = stream.read();
                    sub.next();
                }

                if (sub.is_name("NAM0"))
                    sub.next();

                /*std::cout << boost::format("model: %s (%.1f, %.1f, %.1f)\n") % name
                    % position.x % position.y % position.z;*/
            }
        }
    }
    
    struct Sort
    {
        bool operator() (const std::pair<std::string, RecordHeader*>& a,
                                const std::pair<std::string, RecordHeader*>& b) const
        {
            return a.first < b.first;
        }
    };
    std::sort(cells.begin(), cells.end(), Sort());
    cells.swap(index.cells);
}

/*
    The general strategy is to read through the entire file, building a 
    "table of contents" via the array of record headers and their offsets.

    Once that's built, the file can be accessed randomly to pull in pieces
    as they are needed.  A caching layer can pull in chunks of records in
    advance.

    Background thread prioritizes loading of elements
 */
void loadEsmFile (std::string filenameStr)
{
    bfs::path filename (filenameStr);
    filename.normalize();

    std::string fileonly = filename.filename();

    Stream stream;
    stream.open(filename.string());

    if (stream.good())
    {
        std::cout << "Creating header index..." << std::endl;
        Index index (stream);
        std::cout << "Creating cell index..." << std::endl;
        readCells(stream, index);

        loadCell(stream, index, "Beshara");

        std::cout << "Done." << std::endl;
        stream.close();
    }
}


//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    using namespace lx0;

    int exitCode = -1;
    try
    {
        EnginePtr   spEngine   = Engine::acquire();
        
        DocumentPtr spDocument = spEngine->createDocument();
        
        loadEsmFile("mwdata/Morrowind.esm");
        loadBsaDirectory("mwdata");

        
        ViewPtr spView = spDocument->createView("Canvas", "view", create_renderer() );

        lxvar options;
        options.insert("title", "LxMorrowind");
        options.insert("width", 800);
        options.insert("height", 400);
        spView->show(options);

        spEngine->sendEvent("quit");
        exitCode = spEngine->run();

        spView.reset();
        spEngine->closeDocument(spDocument);
        spEngine->shutdown();
    }
    catch (lx0::error_exception& e)
    {
        std::cout << "Error: " << e.details().c_str() << std::endl
                    << "Code: " << e.type() << std::endl
                    << std::endl;
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled std::exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
