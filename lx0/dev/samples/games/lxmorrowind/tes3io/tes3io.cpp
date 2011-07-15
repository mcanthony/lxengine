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

#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <lx0/lxengine.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glgeom/prototype/std_lights.hpp>

#include <niflib/niflib.h>
#include <niflib/obj/NiObject.h>
#include <niflib/obj/NiNode.h>
#include <niflib/obj/NiGeometry.h>
#include <niflib/obj/NiTriShape.h>
#include <niflib/obj/NiTriShapeData.h>
#include <niflib/obj/NiMaterialProperty.h>
#include <niflib/obj/NiTexture.h>
#include <niflib/obj/NiTextureProperty.h>
#include <niflib/obj/NiTexturingProperty.h>
#include <niflib/obj/NiTextureModeProperty.h>
#include <niflib/obj/NiImage.h>
#include <niflib/obj/NiRawImageData.h>
#include <niflib/gen/TexDesc.h>
#include <niflib/gen/TexSource.h>
#include <niflib/obj/NiPixelData.h>
#include <niflib/obj/NiSourceTexture.h>

#include "tes3io.hpp"
#include "esmiterator.hpp"

namespace bfs = boost::filesystem;

//===========================================================================//
//   I M P L E M E N T A T I O N
//===========================================================================//

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

std::shared_ptr<scene_group>
processNifObject (Niflib::NiObjectRef spObject)
{
    std::shared_ptr<scene_group> spGroup( new scene_group );

    if (Niflib::NiNodeRef spNode = Niflib::DynamicCast<Niflib::NiNode>(spObject))
    {
        auto children = spNode->GetChildren();
        for (auto it = children.begin(); it != children.end(); ++it)
        {
            if (Niflib::NiTriShapeRef spTriShape = Niflib::DynamicCast<Niflib::NiTriShape>(*it))
            {
                if (!spTriShape->GetVisibility())
                    continue;

                int material = spTriShape->GetActiveMaterial();
                std::string textureFilename;

                auto properties = spTriShape->GetProperties();
                for (auto jt = properties.begin(); jt != properties.end(); ++jt)
                {
                    if (Niflib::NiMaterialPropertyRef spMaterial = Niflib::DynamicCast<Niflib::NiMaterialProperty>(*jt))
                    {
                    }
                    else if (Niflib::NiTexturingPropertyRef spInfo = Niflib::DynamicCast<Niflib::NiTexturingProperty>(*jt))
                    {
                        int count = spInfo->GetTextureCount();
                        for (int i = 0; i < count; ++i)
                        {
                            Niflib::TexDesc desc = spInfo->GetTexture(0);
                            Niflib::NiSourceTextureRef spSource = desc.source;

                            if (!spSource->IsTextureExternal())
                            {
                                Niflib::NiPixelDataRef spData = spSource->GetPixelData();
                                int width = spData->GetWidth();
                                int height = spData->GetHeight();
                                auto texels = spData->GetColors();
                            }
                            else
                            {
                                textureFilename = spSource->GetTextureFileName();
                            }
                        }
                    }
                    else if (Niflib::NiTexturePropertyRef spTexture = Niflib::DynamicCast<Niflib::NiTextureProperty>(*jt))
                    {
                        Niflib::NiImageRef spImage = spTexture->GetImage();
                        bool bExternal = spImage->IsTextureExternal();
                        if (!bExternal)
                        {
                            Niflib::NiRawImageDataRef spData = spImage->GetRawImageData();
                            spData->GetIDString();
                        }
                    }
                }
                
                if (Niflib::NiTriBasedGeomDataRef spData = Niflib::DynamicCast<Niflib::NiTriBasedGeomData>(spTriShape->GetData()))
                {
                    spGroup->instances.resize( spGroup->instances.size() + 1 );
                    auto& primitive = spGroup->instances.back().primitive;
                    auto& transform = spGroup->instances.back().transform;

                    //
                    // A mesh is being processed, so record the texture filename found earlier.
                    //
                    spGroup->instances.back().material.handle = textureFilename;
                
                    //
                    // Let NifLib compute the full transform up to the parent
                    //
                    auto world = spTriShape->GetWorldTransform();
                    for (int i = 0; i < 16; ++i)
                        transform[i%4][i/4] = world[i%4][i/4];

                    //
                    // Copy the data from Niflib form to GLGeom form
                    //
                    auto indices = spData->GetTriangles();
                    primitive.indices.reserve(indices.size() * 3);
                    for (auto it = indices.begin(); it != indices.end(); ++it)
                    {
                        primitive.indices.push_back( it->v1 );
                        primitive.indices.push_back( it->v2 );
                        primitive.indices.push_back( it->v3 );
                    }
                
                    auto vertices = spData->GetVertices();
                    primitive.vertex.positions.reserve(vertices.size());
                    for (auto it = vertices.begin(); it != vertices.end(); ++it)
                    {
                        glgeom::point3f p(it->x, it->y, it->z);
                        primitive.vertex.positions.push_back(p);
                        primitive.bbox.merge(p);
                    }

                    auto normals = spData->GetNormals();
                    primitive.vertex.normals.reserve(vertices.size());
                    for (auto it = normals.begin(); it != normals.end(); ++it)
                        primitive.vertex.normals.push_back( glgeom::vector3f(it->x, it->y, it->z) );

                    auto colors = spData->GetColors();
                    primitive.vertex.colors.reserve(colors.size());
                    for (auto it = colors.begin(); it != colors.end(); ++it)
                    {
                        // Ignore alpha for now
                        primitive.vertex.colors.push_back( glgeom::color3f(it->r, it->g, it->b) );
                    }

                    auto channels = spData->GetUVSetCount();
                    primitive.vertex.uv.resize(channels);
                    for (short i = 0; i < channels; ++i)
                    {
                        auto uv = spData->GetUVSet(i);
                        primitive.vertex.uv[i].reserve(uv.size());
                        for (auto it = uv.begin(); it != uv.end(); ++it)
                            primitive.vertex.uv[i].push_back( glgeom::point2f(it->u, it->v) );
                    }

                    auto center = spData->GetCenter();
                    primitive.bsphere.center = glgeom::point3f(center.x, center.y, center.z);
                    primitive.bsphere.radius = spData->GetRadius();
                }
            }
        }
    }
    return spGroup;
}


//===========================================================================//

struct BsaFile
{
    struct Entry
    {
        std::string     name;
        size_t          size;
        lx0::uint64     offset;
    };

    std::string                  mFilename;
    std::map<std::string, Entry> mEntries;
};

class BsaCollection
{
public:
    void initialize(const char* path);

    std::shared_ptr<scene_group>    getModel            (std::string type, std::string name);
    std::shared_ptr<std::istream>   getTextureStream    (std::string name);

protected:
    std::map< std::string, std::shared_ptr<glgeom::primitive_buffer> > mModelCache;
    std::vector<BsaFile> mBsas;
};

std::shared_ptr<scene_group>
BsaCollection::getModel (std::string type, std::string name)
{
    std::shared_ptr<scene_group> spGroup(new scene_group);

    auto fullname = type + name;
    boost::to_lower(fullname);

    auto it = mModelCache.find(fullname);
    if (it == mModelCache.end())
    {
        for (auto jt = mBsas.begin(); jt != mBsas.end(); ++jt)
        {
            auto kt = jt->mEntries.find(fullname);
            if (kt != jt->mEntries.end())
            {
                Stream stream;
                stream.open(jt->mFilename);
                stream.seekg(kt->second.offset);

                Niflib::NifInfo info;
                auto spNifRoot = Niflib::ReadNifTree(stream.stream(), &info);
                stream.close();
                
                return processNifObject(spNifRoot);
            }
        }
    }
    return spGroup;
}

std::shared_ptr<std::istream>   
BsaCollection::getTextureStream (std::string name)
{
    for (auto jt = mBsas.begin(); jt != mBsas.end(); ++jt)
    {
        boost::to_lower(name);
        auto kt = jt->mEntries.find(name);
        if (kt != jt->mEntries.end())
        {
            std::shared_ptr<std::ifstream> spStream(new std::ifstream);
            spStream->open(jt->mFilename, std::ios::binary);
            spStream->seekg(kt->second.offset);
            return spStream;
        }
    }
    return std::shared_ptr<std::istream>();
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

        lx0::uint64 dataOffset = stream.tellg();

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

#define _MAKE_ID_0 '0'
#define _MAKE_ID_1 '1'
#define _MAKE_ID_2 '2'
#define _MAKE_ID_3 '3'
#define _MAKE_ID_4 '4'
#define _MAKE_ID_5 '5'
#define _MAKE_ID_6 '6'
#define _MAKE_ID_7 '7'
#define _MAKE_ID_8 '8'
#define _MAKE_ID_9 '9'
#define _MAKE_ID_A 'A'
#define _MAKE_ID_B 'B'
#define _MAKE_ID_C 'C'
#define _MAKE_ID_D 'D'
#define _MAKE_ID_E 'E'
#define _MAKE_ID_F 'F'
#define _MAKE_ID_G 'G'
#define _MAKE_ID_H 'H'
#define _MAKE_ID_I 'I'
#define _MAKE_ID_J 'J'
#define _MAKE_ID_K 'K'
#define _MAKE_ID_L 'L'
#define _MAKE_ID_M 'M'
#define _MAKE_ID_N 'N'
#define _MAKE_ID_O 'O'
#define _MAKE_ID_P 'P'
#define _MAKE_ID_Q 'Q'
#define _MAKE_ID_R 'R'
#define _MAKE_ID_S 'S'
#define _MAKE_ID_T 'T'
#define _MAKE_ID_U 'U'
#define _MAKE_ID_V 'V'
#define _MAKE_ID_W 'W'
#define _MAKE_ID_X 'X'
#define _MAKE_ID_Y 'Y'
#define _MAKE_ID_Z 'Z'
#define _MAKE_ID(a,b,c,d) \
    kId_ ## a ## b ## c ## d = ( (_MAKE_ID_ ## d << 24) | (_MAKE_ID_ ## c << 16) | (_MAKE_ID_ ## b << 8) | (_MAKE_ID_ ## a) ) 
enum
{
    _MAKE_ID(F,N,A,M),  // kId_FNAM
    _MAKE_ID(H,E,D,R),
    _MAKE_ID(L,H,D,T),  // etc.
    _MAKE_ID(L,I,G,H),  
    _MAKE_ID(N,A,M,E),
    _MAKE_ID(S,T,A,T),
    _MAKE_ID(T,E,S,3),
};

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
        assert(iter.is_record(kId_TES3));

        if (iter.is_sub(kId_HEDR))
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
                || iter.is_record("STAT")
                || iter.is_record("LIGH"))
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

struct Light
{
    Light (ESMIterator& iter)
    {
        while (!iter.sub_done())
        {
            switch (iter.sub_id())
            {
            case kId_NAME:
                name = iter.read_string();
                break;
            case kId_LHDT:
                {
                    float       weight  = iter.read();
                    int         value   = iter.read();
                    int         time    = iter.read();
                    int         radius  = iter.read();
                    lx0::uint32 packed  = iter.read();
                    lx0::uint32 flags   = iter.read();

                    if (radius > 0)
                        light.radius = float(radius);

                    light.color = glgeom::unpack_rgbx<float>(packed);
                }
                break;
            }
            iter.next_sub();
        }
    }

    std::string             name;
    glgeom::point_light_f   light;
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

glm::mat4 _transform(Reference& ref)
{
    glm::mat4 mrot  = glm::gtx::euler_angles::eulerAngleYXZ(-ref.rotation.y, -ref.rotation.x, -ref.rotation.z);
    glm::mat4 mtran = glm::translate(glm::mat4(), ref.position.vec);
    glm::mat4 mscal = glm::scale(glm::mat4(), glm::vec3(ref.scale, ref.scale, ref.scale));
                          
    return mtran * mrot * mscal;
}

class TES3Loader : public ITES3Loader
{
public:
    virtual void    initialize  (const char* path)
    {
        mEsmFilename = std::string(path) + "/Morrowind.esm";

        loadEsmFile(mEsmFilename, mEsmIndex);
        mBsaSet.initialize(path);
    }
    
    virtual void    cell        (const char* id, scene_group& group)
    {
        std::cout << boost::format("Loading cell '%s'...\n") % id;

        Stream stream;
        stream.open(mEsmFilename);
        
        Cell cell;
        loadCell(cell, stream, mEsmIndex, id);

        std::cout << "References = " << cell.references.size() << std::endl;
        for (auto it = cell.references.begin(); it != cell.references.end(); ++it)
        {
            auto jt = mEsmIndex.names.find(it->name);
            if (jt != mEsmIndex.names.end())
            {
                ESMIterator iter(stream, *jt->second);

                switch (jt->second->name_id)
                {
                    default:
                        std::cout << "- " << it->name << " (type " << jt->second->name << " not handled)" << std::endl;
                    break;

                    case kId_STAT:
                    {
                        StaticModel model (iter);
                    
                        auto subgroup = mBsaSet.getModel("meshes\\", model.model);
                        auto transform = _transform(*it);
                        for (auto kt = subgroup->instances.begin(); kt != subgroup->instances.end(); ++kt)
                        {
                            // Account for the cell reference transform in addition to the native transform in 
                            // model itself.
                            kt->transform = transform * kt->transform;

                            if (!kt->material.handle.empty())
                            {
                                // Apparently Bethesda's level designers/artists used full res TGAs in the editor
                                // and the production system automatically mapped these files to compressed DDS equivalents
                                if (boost::ends_with(kt->material.handle, ".tga"))
                                    kt->material.handle = kt->material.handle.substr(0, kt->material.handle.length() - 4) + ".dds";

                                kt->material.handle = std::string("textures\\") + kt->material.handle;
                                kt->material.format = "DDS";
                                
                                std::string name = kt->material.handle;
                                kt->material.callback = [this, name]() {
                                    return mBsaSet.getTextureStream(name);
                                };
                            }
                        }
                        
                        
                        group.merge(*subgroup);

                        std::cout << "+ model " << model.model << "\n";
                    }
                    break;
                
                    case kId_LIGH:
                    {
                        Light esmLight (iter);
                        auto transform = _transform(*it);
                        esmLight.light.position = transform * glgeom::point3f(0,0,0);

                        //
                        // These adjustments to the light values are more or less arbitrary guesses
                        // since the actual mapping of NIF data to LxEngine parameters is not
                        // exactly known.
                        //
                        esmLight.light.radius *= 16.0f;
                        esmLight.light.attenuation = glm::vec3(0, 3, 0);
                        
                        group.lights.push_back(esmLight.light);

                        std::cout << "+ light " << esmLight.name << "\n";
                    }
                    break;

                }
            }
            else
                std::cout << "- " << it->name << " (not indexed)" << std::endl;
        }
        

        stream.close();
    }

    std::string     mEsmFilename;
    Index           mEsmIndex;
    BsaCollection   mBsaSet;
};

ITES3Loader* ITES3Loader::create()
{
    return new TES3Loader;
}
