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

#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <lx0/lxengine.hpp>

#include <niflib/niflib.h>
#include <niflib/obj/NiObject.h>

#include "tes3io.hpp"
#include "esmiterator.hpp"

namespace bfs = boost::filesystem;

std::string readName (Stream& stream)
{
    // Read the length. Cap it in case something has gone wrong.
    lx0::uint32 length = stream.read();
    length = std::min(length, 1024u);

    return stream.read_string2(length);
}

void readIndexList (Stream& stream)
{
    lx0::uint32 count = stream.read();
    for (lx0::uint32 j = 0; j < count; ++j)
    {
        int index = stream.read();
    }
}

void readNode (Stream& stream)
{
    std::string name = readName(stream);

    lx0::uint32 index0 = stream.read();
    lx0::uint32 index1 = stream.read();

    lx0::uint16 flags = stream.read();

    glm::vec3 position = stream.read();
    glm::mat3 rotation = stream.read();
    float     scale    = stream.read();
    glm::vec3 velocity = stream.read();

    readIndexList(stream);  // properties

    lx0::uint32 bounds = stream.read();
    if (bounds != 0)
    {
        lx0::uint32 unused   = stream.read();
        glm::vec3 center     = stream.read();
        glm::mat3 rotation   = stream.read();
        glm::vec3 halfLength = stream.read();
    }
}

void readShape (Stream& stream, glgeom::primitive_buffer& primitive)
{
    lx0::int16 numVertices = stream.read();

    int hasPositionData = stream.read();
    if (hasPositionData)
    {
        primitive.vertex.positions.resize(numVertices);
        stream.read(&primitive.vertex.positions[0].x, numVertices* 3);
    }

    int hasNormalData = stream.read();
    if (hasNormalData)
    {
        primitive.vertex.normals.resize(numVertices);
        stream.read(&primitive.vertex.normals[0].x, numVertices* 3);
    }

    primitive.bsphere.center = stream.read();
    primitive.bsphere.radius = stream.read();

    if ((int)stream.read())
    {
        std::vector<glm::vec4> colors;
        colors.resize(numVertices);
        stream.read(&colors[0].r, numVertices * 4);
    }

    lx0::int16 uvCount = stream.read();
    uvCount &= 0x3F;

    if (int(stream.read()))
    {
        std::vector<float> unused;
        unused.resize( uvCount * numVertices * 2);
        stream.read(&unused[0], unused.size());
    }

    //
    // Build a bounding box for the shape
    //
    for (auto it = primitive.vertex.positions.begin(); it != primitive.vertex.positions.end(); ++it)
        primitive.bbox.merge(*it);
}

void readNiTriShapeData (Stream& stream, glgeom::primitive_buffer& primitive)
{
    readShape(stream, primitive);

    //
    // Read the triangles
    //
    lx0::uint16 triangleCount = stream.read();
    lx0::uint32 indexCount = stream.read();
    if (indexCount > 0)
    {
        primitive.indices.resize(indexCount);
        stream.read(&primitive.indices[0], primitive.indices.size());
    }

    lx0::int16 unused = stream.read();
    for (int i = 0; i < unused; ++i)
    {
        lx0::int16 count = stream.read();
        stream.skip(count * 2);
    }
}

/*
    For now, load up the first mesh encountered.   Can add more properties
    later.
 */
void loadNif (std::ifstream& fin, glgeom::primitive_buffer& primitive)
{
    Stream stream(fin);

    char headerName[40];
    stream.read(headerName, 40);

    lx0::uint32 version = stream.read();
    lx0::uint32 records = stream.read();

    bool done = false;
    for (lx0::uint32 i = 0; i < records && !done; ++i)
    {
        std::string name = readName(stream);

        if (name == "NiNode")
        {
            readNode(stream);
            readIndexList(stream);  // children
            readIndexList(stream);  // effects
        }
        else if (name == "NiTriShape")
        {
            readNode(stream);
            int data = stream.read();
            int skin = stream.read();
        }
        else if (name == "NiTriShapeData")
        {
            readNiTriShapeData(stream, primitive);
            done = true;
        }
        else if (name == "NiTexturingProperty")
        {
            std::string name = readName(stream);
            int         extra = stream.read();
            int         control = stream.read();

            lx0::uint16 flags = stream.read();
            int         apply = stream.read();
            int         count = stream.read();

            for (int  i = 0; i < count; ++i)
            {
                int     use     = stream.read();
                if (use)
                {
                    int texId   = stream.read();
                    int clamp   = stream.read();
                    int filter  = stream.read();
                    int set     = stream.read();
                    stream.skip(6);

                    // Bump map special properties
                    if (i == 5)
                    {
                        float     lumScale  = stream.read();
                        float     lumOffset = stream.read();
                        glm::vec4 lumVector = stream.read();
                    }
                }
            }
        }
        else if (name == "NiSourceTexture")
        {
            std::string name = readName(stream);
            int         extra = stream.read();
            int         control = stream.read();

            char        external = stream.read();
            if (external)
            {
                std::string filename = readName(stream);
            }
            else
            {
                char unused = stream.read();
                int  index = stream.read();     // internal data ptr
            }

            int pixel = stream.read();
            int mipmap = stream.read();
            int alpha = stream.read();
            stream.skip(1);
        }
        else if (name == "NiMaterialProperty")
        {
            std::string name = readName(stream);
            int         extra = stream.read();
            int         control = stream.read();

            lx0::uint16 flags = stream.read();

            glm::vec3   ambient = stream.read();
            glm::vec3   diffuse = stream.read();
            glm::vec3   specular = stream.read();
            glm::vec3   emissive = stream.read();
            float       glossiness = stream.read();
            float       alpha   = stream.read();
        }
        else
            return;
    }
}


//===========================================================================//

struct BsaFile
{
    struct Entry
    {
        std::string     name;
        size_t          size;
        lx0::uint32     offset;
    };

    std::string                  mFilename;
    std::map<std::string, Entry> mEntries;
};

class BsaCollection
{
public:
    void initialize(const char* path);

    std::shared_ptr<glgeom::primitive_buffer> getModel (std::string type, std::string name);

protected:
    std::map< std::string, std::shared_ptr<glgeom::primitive_buffer> > mModelCache;
    std::vector<BsaFile> mBsas;
};

std::shared_ptr<glgeom::primitive_buffer>
BsaCollection::getModel (std::string type, std::string name)
{
    auto fullname = type + name;
    boost::to_lower(fullname);

    auto it = mModelCache.find(fullname);
    if (it == mModelCache.end())
    {
        std::shared_ptr<glgeom::primitive_buffer> spPrimitive;

        for (auto jt = mBsas.begin(); jt != mBsas.end(); ++jt)
        {
            auto kt = jt->mEntries.find(fullname);
            if (kt != jt->mEntries.end())
            {
                Stream stream;
                stream.open(jt->mFilename);
                stream.seekg(kt->second.offset);
                
                spPrimitive.reset(new glgeom::primitive_buffer);
                lx0::uint32 pos = stream.tellg();
                loadNif(stream.stream(), *spPrimitive);
                stream.seekg(pos);
                
                Niflib::NifInfo info;
                auto spNifRoot = Niflib::ReadNifTree(stream.stream(), &info);

                stream.close();

                return spPrimitive;
            }
        }
        return spPrimitive;
    }
    else
        return it->second;
}


void loadBsaFile (BsaFile& bsa, const std::string& filename)
{
    std::cout << boost::format("Loading file '%s'...\n") % filename;
    bsa.mFilename = filename;

    Stream stream;
    stream.open(filename);
    if (stream.good())
    {
        stream.skip(4);
        lx0::uint32 directorySize = stream.read();
        lx0::uint32 fileCount = stream.read();

        struct FileOffset
        {
            lx0::uint32 size;
            lx0::uint32 offset;
        };
        std::vector<FileOffset> fileOffsets;
        fileOffsets.resize(fileCount);
        stream.read((char *)&fileOffsets[0], fileOffsets.size() * 8);

        std::vector<lx0::uint32> nameOffsets;
        nameOffsets.resize(fileCount);
        stream.read(&nameOffsets[0], nameOffsets.size());

        std::vector<char> nameTable;
        nameTable.resize(directorySize - fileCount * 12);
        stream.read(&nameTable[0], nameTable.size());

        // Skip the hash table
        stream.skip(8 * fileCount);

        lx0::uint32 dataOffset = stream.tellg();

        for (lx0::uint32 i = 0; i < fileCount; ++i)
        {
            BsaFile::Entry entry;
            entry.name = &nameTable[ nameOffsets[i] ];
            entry.size = fileOffsets[i].size; 
            entry.offset = dataOffset + fileOffsets[i].offset; 

            bsa.mEntries.insert(std::make_pair(entry.name, entry));
        }
        stream.close();
    }

    //
    // Debugging tool: create an index of all the named entries
    //
    if (false)
    {
        std::ofstream out(boost::str(boost::format("%s_index.txt") % boost::filesystem::path(bsa.mFilename).filename()));
        lx_check_error(out.good());
        for (auto it = bsa.mEntries.begin(); it != bsa.mEntries.end(); ++it)
            out << it->first << std::endl;
        out.close();
    }
}

void BsaCollection::initialize (const char* path)
{
    lx0::for_files_in_directory(path, "bsa", [&](std::string file) {
        mBsas.push_back(BsaFile());
        loadBsaFile(mBsas.back(), file);
    });
}

//===========================================================================//


struct Index
{
public:

    void read (Stream& stream)
    {
        //
        // Read the initial header
        //
        // Use this to pre-allocate and prepare for reading the 
        // full index.
        //
        lx0::uint32 recordCount;

        ESMIterator iter (stream);
        assert(iter.is_record("TES3"));

        if (iter.is_sub("HEDR"))
        {
            float version = stream.read();
            iter.skip(4 + 32 + 256);
            recordCount = iter.read();
            headers.reserve(recordCount);
            recordCount--;
        }
        headers.push_back(iter.record_header());
        iter.next_record();

        while (recordCount-- > 0)
        {
            headers.push_back(iter.record_header());

            if (   iter.is_record("CELL")
                || iter.is_record("STAT"))
            {
                names.insert(std::make_pair(iter.read_string(), &headers.back()));
            }
            iter.next_record();
        }
    }

    std::vector<RecordHeader>               headers;
    std::map<std::string, RecordHeader*>    names;
};

struct StaticModel
{
    StaticModel (ESMIterator& iter)
    {
        name = iter.read_string();
        iter.next_sub();
        model = iter.read_string();
        iter.next_sub();
    }

    std::string name;
    std::string model;
};

struct Reference
{
    Reference()
        : scale (1.0f)
    {
    }

    std::string     name;
    glgeom::point3f position;
    glm::vec3       rotation;
    float           scale;
};

struct Cell
{
public:
    std::string             name;
    lx0::uint32             flags;
    int                     grid[2];
    std::vector<Reference>  references;
};

void loadObjectReferences (Cell& cell, ESMIterator& iter)
{
    while (iter.is_sub("FRMR"))
    {
        Reference ref;

        lx0::uint32 index = iter.read();
        iter.next_sub();

        if (iter.is_sub("NAME"))
        {
            ref.name = iter.read_string();
            iter.next_sub();
        }
        if (iter.is_sub("XSCL"))
        {
            ref.scale = iter.read();
            iter.next_sub();
        }
        if (iter.is_sub("FLTV"))
            iter.next_sub();
        if (iter.is_sub("ANAM"))
            iter.next_sub();
        if (iter.is_sub("BNAM"))
            iter.next_sub();
        if (iter.is_sub("XSOL"))
            iter.next_sub();
        if (iter.is_sub("CNAM"))
            iter.next_sub();
        if (iter.is_sub("INDX"))
            iter.next_sub();
        if (iter.is_sub("XCHG"))
            iter.next_sub();
        if (iter.is_sub("INTV"))
            iter.next_sub();
        if (iter.is_sub("NAM9"))
            iter.next_sub();
        if (iter.is_sub("KNAM"))   // Key NAMe
            iter.next_sub();
        if (iter.is_sub("TNAM"))   // Trap NAMe
            iter.next_sub();
        if (iter.is_sub("UNAM"))   
            iter.next_sub();
        if (iter.is_sub("FLTV"))   
            iter.next_sub();
        if (iter.is_sub("DODT"))
            iter.next_sub();
        if (iter.is_sub("DNAM"))
            iter.next_sub();
        if (iter.is_sub("DATA"))
        {
            ref.position = iter.read();
            ref.rotation = iter.read();
            iter.next_sub();
        }

        if (iter.is_sub("NAM0"))
            iter.next_sub();

        cell.references.push_back(ref);
    }
}

void loadCell (Cell& cell, Stream& stream, RecordHeader& recordHeader)
{
    ESMIterator iter(stream, recordHeader);

    if (iter.is_sub("NAME"))
    {
        cell.name = iter.read_string();
        iter.next_sub();
    }
    if (iter.is_sub("DATA"))
    {
        cell.flags = iter.read();
        cell.grid[0] = iter.read();
        cell.grid[1] = iter.read();
        iter.next_sub();
    }

    if (cell.flags & 0x01)   // Interior
    {
        if (iter.is_sub("INTV"))
            iter.next_sub();
        if (iter.is_sub("WHGT"))
        {
            lx0::uint32 waterHeight = iter.read();
            iter.next_sub();
        }
        if (iter.is_sub("AMBI"))
        {
            lx0::uint32 ambientColor = iter.read();
            lx0::uint32 sunlightColor = iter.read();
            lx0::uint32 fogColor = iter.read();
            float fogDensity = iter.read();
            iter.next_sub();
        }
    }
    else
    {
        if (iter.is_sub("NAM5"))
        {
            lx0::uint32 color = iter.read();
            iter.next_sub();
        }
    }

    if (iter.is_sub("NAME"))
        iter.next_sub();

    if (iter.is_sub("RGNN"))
        iter.next_sub();
    if (iter.is_sub("NAM0"))
        iter.next_sub();

    loadObjectReferences(cell, iter);
}

void loadCell (Cell& cell, Stream& stream, Index& index, const char* name)
{
    auto it = index.names.find(name);
    if (it != index.names.end())
        loadCell(cell, stream, *it->second);
}


/*
    The general strategy is to read through the entire file, building a 
    "table of contents" via the array of record headers and their offsets.

    Once that's built, the file can be accessed randomly to pull in pieces
    as they are needed.  A caching layer can pull in chunks of records in
    advance.

    Background thread prioritizes loading of elements
 */
void loadEsmFile (std::string filenameStr, Index& index)
{
    bfs::path filename (filenameStr);
    filename.normalize();

    Stream stream;
    stream.open(filename.string());

    if (stream.good())
    {
        std::cout << boost::format("Loading '%s'...\n") % filename.filename();
        index.read (stream);
        stream.close();
    }
}

class Tes3Imp
{
public:
    void    initialize  (const char* path)
    {
        mEsmFilename = std::string(path) + "/Morrowind.esm";

        loadEsmFile(mEsmFilename, mEsmIndex);
        mBsaSet.initialize(path);
    }
    
    void    cell        (const char* id, scene_group& group)
    {
        Stream stream;
        stream.open(mEsmFilename);
        
        Cell cell;
        loadCell(cell, stream, mEsmIndex, "Beshara");
        for (auto it = cell.references.begin(); it != cell.references.end(); ++it)
        {
            std::cout << "* " << it->name << std::endl;

            auto jt = mEsmIndex.names.find(it->name);
            if (jt != mEsmIndex.names.end())
            {
                if (strncmp(jt->second->name, "STAT", 4) == 0)
                {
                    StaticModel model (ESMIterator(stream, *jt->second));
                    std::cout << "\t" << model.model << "\n";
                    
                    auto spPrim = mBsaSet.getModel("meshes\\", model.model);
                    if (!spPrim->vertex.positions.empty())
                    {
                        instance inst;
                        inst.primitive = *spPrim;
                        inst.transform = glm::mat4();
                        group.instances.push_back(inst);
                    }
                }
            }
        }

        stream.close();
    }

    std::string     mEsmFilename;
    Index           mEsmIndex;
    BsaCollection   mBsaSet;
};

Tes3Io::Tes3Io() : mpImp(new Tes3Imp) {}
Tes3Io::~Tes3Io() { delete mpImp; }

void Tes3Io::initialize  (const char* path)
{           
    mpImp->initialize(path);
}

void Tes3Io::cell (const char* id, scene_group& group)
{
    mpImp->cell(id, group);
    std::cout << boost::format("Loaded cell '%s'.\n") % id;
}