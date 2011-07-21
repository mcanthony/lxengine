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

#include <glm/gtx/euler_angles.hpp>
#include <glgeom/prototype/std_lights.hpp>
#include <lx0/lxengine.hpp>

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

#include "../tes3loader.hpp"
#include "esmiterator.hpp"
#include "esm_ids.hpp"

namespace bfs = boost::filesystem;

//===========================================================================//
//   I M P L E M E N T A T I O N
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

    typedef std::map<std::string, Entry> EntryMap;

    std::string     mFilename;
    EntryMap        mEntries;
};

class BsaCollection
{
public:
    void initialize(const char* path);

    std::pair<bool,BsaFile::EntryMap::iterator>     getEntry    (std::string name);

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


std::pair<bool,BsaFile::EntryMap::iterator>
BsaCollection::getEntry (std::string name)
{
    for (auto jt = mBsas.begin(); jt != mBsas.end(); ++jt)
    {
        boost::to_lower(name);
        auto kt = jt->mEntries.find(name);
        if (kt != jt->mEntries.end())
        {
            return std::make_pair(true, kt);
        }
    }
    return std::make_pair(false, BsaFile::EntryMap::iterator());
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
    lx_warn("Texture stream for '%s' not found", name.c_str());
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


struct LandscapeTexture
{
    LandscapeTexture (ESMIterator& iter)
    {
        while (!iter.sub_done())
        {
            switch (iter.sub_id())
            {
            case kId_NAME:
                name = iter.read_string();
                break;
            case kId_INTV:
                value = iter.read();
                break;
            case kId_DATA:
                // The texture name is null terminated.  We don't want to add the
                // null character to the end of the std::string or else it will be
                // included as part of the string itself (causing problems later on).
                //
                texture.resize( iter.sub_size() - 1);
                iter.read(&texture[0], texture.size());
                break;
            }
            iter.next_sub();
        }
    }

    std::string name;
    std::string texture;
    int         value;      //?
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

            switch (iter.record_id())
            {
            //
            // Index the record types that start with a name field
            //
            case kId_ACTI:
            case kId_APPA:
            case kId_ARMO:
            case kId_BOOK:
            case kId_CELL:
            case kId_CLOT:
            case kId_CONT:
            case kId_CREA:
            case kId_DOOR:
            case kId_INGR:
            case kId_LEVC:
            case kId_LEVI:      
            case kId_LIGH:
            case kId_LOCK:
            //case kId_LTEX:
            case kId_MISC:
            case kId_NPC_:
            case kId_PROB:
            case kId_REGN:
            case kId_REPA:
            case kId_SOUN:
            case kId_STAT:
            case kId_WEAP:
                names.insert(std::make_pair(iter.read_string(), &headers.back()));
                break;

            //
            // Special case LTEX while the landscape texturing is still being figured out...
            //
            case kId_LTEX:
                {
                    LandscapeTexture tex(iter);
                    auto name = tex.name;
                    names.insert(std::make_pair(name, &headers.back()));
                    landscapeTextures.push_back(&headers.back());
                }
                break;

            //
            // Indexed by location, not name
            //
            case kId_LAND:
                {
                    int gridX = iter.read();
                    int gridY = iter.read();
                    landscapes.insert( std::make_pair(std::make_pair(gridX, gridY), &headers.back()) );
                }
                break;

            //
            // Keep this code for debugging all the fields in a sub-record.  Swap 0xFFFFFFFF
            // with a real record id.
            //
            case 0xFFFFFFFF:
                {
                    while (!iter.sub_done())
                    {
                        std::cout << boost::format("%s %d bytes\n") % iter.sub_name() % iter.sub_size();
                        size_t size = iter.sub_size();
                        std::string name = iter.sub_name();
                        std::vector<char> buffer;
                        buffer.resize(iter.sub_size());
                        iter.read(&buffer[0], buffer.size());
                        char* data = &buffer[0];
                        iter.next_sub();
                    }
                }
                break;

            //
            // Known but not indexed
            //
            case kId_ALCH:      // Alchemy?
            case kId_BODY:      // Body-part?
            case kId_BSGN:      // Birthsign
            case kId_CLAS:      // Class definition
            case kId_DIAL:      // Dialogue
            case kId_ENCH:      // Enchantment
            case kId_FACT:      // Faction
            case kId_GLOB:      // ?
            case kId_GMST:      // ?
            case kId_INFO:      // Dialogue-related data?
            case kId_MGEF:      // Magic Effect
            case kId_PGRD:      // Path Grid?
            case kId_RACE:      // Race
            case kId_SCPT:      // Script
            case kId_SKIL:      // Skill
            case kId_SNDG:      // Sound Generator?
            case kId_SPEL:      // Spell
                break;

            //
            // Unknown
            //
            default:
                lx_error("Unrecognized record id '%s'", iter.record_name().c_str()); 
                break;
            }
            iter.next_record();
        }
    } 

    std::vector<RecordHeader>                   headers;
    std::map<std::string, RecordHeader*>        names;
    std::vector<RecordHeader*>                  landscapeTextures;
    std::map<std::pair<int,int>, RecordHeader*> landscapes;
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

struct Landscape
{
    Landscape (ESMIterator& iter)
    {
        while (!iter.sub_done())
        {
            switch (iter.sub_id())
            {
            case kId_INTV:
                gridX = iter.read();
                gridY = iter.read();
                break;
            case kId_VHGT:
                {
                    // Offset, followed by 65x65 heights, then 3 bytes unknown
                    float offset = iter.read();

                    std::vector<lx0::int8> buffer(65 * 65);
                    iter.read(&buffer[0], 65 * 65);
                    
                    vertexHeight.reserve(65 * 65);

                    for (int y = 0; y < 65; ++y)
                    {
                        offset += buffer[y * 65];
                        float row = offset;
                        vertexHeight.push_back(row * 8);
                        for (int x = 1; x < 65; ++x)
                        {
                            row += buffer[y * 65 + x];
                            vertexHeight.push_back(row * 8);
                        }
                    }
                }
                break;
            case kId_VNML:
                {
                    vertexNormal.resize(65 * 65);
                    for (auto it = vertexNormal.begin(); it != vertexNormal.end(); ++it)
                    {
                        lx0::int8 x = iter.read();
                        lx0::int8 y = iter.read();
                        lx0::int8 z = iter.read();
                        *it = glgeom::normalize( glgeom::vector3f( x / 127.0f, y / 127.0f, z / 127.0f ) );
                    }
                }
                break;

            case kId_VTEX:
                {
                    // Still working on figuring out what these values mean
                    textureId.resize(16 * 16);
                    iter.read(&textureId[0], textureId.size());

                    // Are these IDs based on the order of the LTEX records?
                    // How do these map from id to LTEX?
                }
                break;

            case kId_DATA:
            case kId_VCLR:
            case kId_WNAM:
            default:
                {
                    std::string name = iter.sub_name();
                    std::vector<char> buffer (iter.sub_size());
                    iter.read(&buffer[0], buffer.size());
                    mData.push_back( std::make_pair(name, buffer) );
                }
            }
            iter.next_sub();
        }
    }

    int gridX;
    int gridY;
    std::vector<float>            vertexHeight;
    std::vector<glgeom::vector3f> vertexNormal;
    std::vector<lx0::uint16>      textureId;

    std::vector<char> vertexColor;
    std::vector<char> vertexUvs;
    std::vector<std::pair<std::string, std::vector<char>>> mData;
};

struct Door
{
    Door (ESMIterator& iter)
    {
        while (!iter.sub_done())
        {
            switch (iter.sub_id())
            {
            case kId_NAME:
                name = iter.read_string();
                break;
            case kId_MODL:
                model = iter.read_string();
                break;
            }
            iter.next_sub();
        }
    }
    std::string name;
    std::string model;
};

struct Armor
{
    Armor (ESMIterator& iter)
    {
        while (!iter.sub_done())
        {
            switch (iter.sub_id())
            {
            case kId_NAME:
                name = iter.read_string();
                break;
            case kId_MODL:
                model = iter.read_string();
                break;
            }
            iter.next_sub();
        }
    }
    std::string name;
    std::string model;
};

struct Container
{
    Container (ESMIterator& iter)
    {
        while (!iter.sub_done())
        {
            switch (iter.sub_id())
            {
            case kId_NAME:
                name = iter.read_string();
                break;
            case kId_MODL:
                model = iter.read_string();
                break;
            }
            iter.next_sub();
        }
    }
    std::string name;
    std::string model;
};

struct Miscellaneous
{
    Miscellaneous (ESMIterator& iter)
    {
        while (!iter.sub_done())
        {
            switch (iter.sub_id())
            {
            case kId_NAME:
                name = iter.read_string();
                break;
            case kId_MODL:
                model = iter.read_string();
                break;
            }
            iter.next_sub();
        }
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
            case kId_MODL:
                {
                    model = iter.read_string();
                }
                break;
            }
            iter.next_sub();
        }
    }

    std::string             name;
    std::string             model;
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
    bool                isInterior () { return !!(flags & 0x01); }

    std::string             name;
    lx0::uint32             flags;
    int                     grid[2];
    float                   waterHeight;
    std::string             region;
    std::vector<char>       nam5;           // ?
    std::vector<Reference>  references;
};

void loadObjectReferences (Cell& cell, ESMIterator& iter)
{
    while (iter.is_sub(kId_FRMR))
    {
        Reference ref;

        lx0::uint32 index = iter.read();
        iter.next_sub();

        while (!iter.sub_done() && !iter.is_sub(kId_FRMR))
        {
            switch (iter.sub_id())
            {
            case kId_NAME:
                ref.name = iter.read_string();
                break;
            case kId_XSCL:
                ref.scale = iter.read();
                break;
            case kId_DATA:
                ref.position = iter.read();
                ref.rotation = iter.read();
                break;
            }
            iter.next_sub();
        }
        cell.references.push_back(ref);
    }
}

void loadCell (Cell& cell, Stream& stream, RecordHeader& recordHeader)
{
    ESMIterator iter(stream, recordHeader);

    cell.waterHeight = 0.0f;

    bool done = false;
    while (!done && !iter.sub_done()) 
    { 
        switch (iter.sub_id())
        {
        case kId_DATA:
            cell.flags = iter.read();
            cell.grid[0] = iter.read();
            cell.grid[1] = iter.read();
            break;
        case kId_FRMR:
            // Object reference list to follow
            done = true;
            break;
        case kId_NAME:
            cell.name = iter.read_string();
            break;
        case kId_RGNN:
            cell.region = iter.read_string();
            break;
        case kId_WHGT:
            cell.waterHeight = iter.read();
            break;
        case kId_NAM5:
            cell.nam5.resize( iter.sub_size() );
            iter.read(&cell.nam5[0], cell.nam5.size() );
            break;
        default:
            std::cout << "cell skip: " << iter.sub_name() << std::endl;
            break;
        }
        if (!done)
            iter.next_sub();
    }

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

void _resolveTextureName (material_handle& material)
{
    // Apparently Bethesda's level designers/artists used full res TGAs in the editor
    // and the production system automatically mapped these files to compressed DDS equivalents.
    // And apparently a BMP or two as well...
    //
    // Also, it appears all look-ups are case insensative given that ESM and BSA names
    // do not always have consistent casing.
    //
    boost::to_lower(material.handle);
    if (boost::ends_with(material.handle, ".tga") || boost::ends_with(material.handle, ".bmp"))
        material.handle = material.handle.substr(0, material.handle.length() - 4) + ".dds";

    material.handle = std::string("textures\\") + material.handle;
    material.format = "DDS";
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

    void _resolveMaterial (material_handle& material)
    {
        if (!material.handle.empty())
        {
            _resolveTextureName(material);
                                
            std::string name = material.handle;
            if (mBsaSet.getEntry(name).first)
            {
                material.callback = [this, name]() {
                    return mBsaSet.getTextureStream(name);
                };
            }
            else
                lx_warn("Model references texture that doesn't exist '%s'", name.c_str());
        }
    }

    std::shared_ptr<scene_group> _getModel(Reference& ref, std::string modelName)
    {
        auto subgroup = mBsaSet.getModel("meshes\\", modelName);
        
        auto transform = _transform(ref);
        for (auto kt = subgroup->instances.begin(); kt != subgroup->instances.end(); ++kt)
        {
            // Account for the cell reference transform in addition to the native transform in 
            // model itself.
            kt->transform = transform * kt->transform;
            _resolveMaterial(kt->material);

        }

        return subgroup;
    }
    
    virtual void    cell        (const char* id, scene_group& group)
    {
        std::cout << boost::format("Loading cell '%s'...\n") % id;

        Stream stream;
        stream.open(mEsmFilename);
        
        Cell cell;
        loadCell(cell, stream, mEsmIndex, id);

        std::cout << "Interior = " << (cell.isInterior() ? "yes" : "no") << std::endl;
        std::cout << "Region = " << cell.region << std::endl;
        std::cout << boost::format("Grid = %d, %d\n") % cell.grid[0] % cell.grid[1];
        std::cout << "References = " << cell.references.size() << std::endl;

        if (!cell.isInterior())
        {
            auto it = mEsmIndex.landscapes.find(std::make_pair(cell.grid[0], cell.grid[1]));
            if (it != mEsmIndex.landscapes.end())
            {
                Landscape landscape(ESMIterator(stream, *it->second));
                std::cout << "Landscape found at grid location\n";

                //
                // The landscape is composed of 4x4 blocks that share the same texture.
                // The landscape as a whole has 16x16 of these (64x64 total).  So build
                // 256 primitives to represent this, but then merge the ones that share
                // the same texture in order to reduce the number of objects that need
                // to be drawn (more objects = slower).
                //
                // There are more efficient means of dealing with landscapes: (1) Use a shared
                // 65x65 vertex buffer with 256 (or less) index buffers all referencing that shared
                // buffer. (2) Retain the fact that this is heightmap when passing it into 
                // bullet and pass it in as a single primitive. (3) Other I'm sure.  We'll save
                // this for a later version of LxEngine.
                //
                for (int cy = 0; cy < 16; ++cy)
                {
                    for (int cx = 0; cx < 16; ++cx)
                    {
                        std::shared_ptr<glgeom::primitive_buffer> spPrimitive (new glgeom::primitive_buffer);
                        spPrimitive->vertex.positions.resize(5 * 5);
                        spPrimitive->vertex.normals.resize(5 * 5);
                        spPrimitive->vertex.uv.resize(1);
                        spPrimitive->vertex.uv[0].resize(5 * 5);

                        for (int gy = 0; gy < 5; ++gy)
                        {
                            for (int gx = 0; gx < 5; ++gx)
                            {
                                // OffsetL into the 65x65 landscape grid
                                // OffsetP into the 5x5 grid cell
                                int xL = (cx * 4 + gx);
                                int yL = (cy * 4 + gy);
                                int offsetL = yL * 65 + xL;
                                int offsetP = gy * 5 + gx;

                                auto& p = spPrimitive->vertex.positions[offsetP];
                                p.x = xL * 128;
                                p.y = yL * 128;
                                p.z = landscape.vertexHeight[offsetL];

                                spPrimitive->vertex.normals[offsetP] = landscape.vertexNormal[offsetL];
                                spPrimitive->vertex.uv[0][offsetP].vec = glm::vec2(gx / 4.0f, gy / 4.0f);
                            }
                        }

                        spPrimitive->indices.resize(5 * 5 * 6);     // 2 triangles per cell (3 x 2)
                        for (int gy = 0; gy < 4; ++gy)
                        {
                            for (int gx = 0; gx < 4; ++gx)
                            {
                                spPrimitive->indices.push_back( (gy  + 0) * 5 + (gx + 0) ); 
                                spPrimitive->indices.push_back( (gy  + 1) * 5 + (gx + 0) ); 
                                spPrimitive->indices.push_back( (gy  + 1) * 5 + (gx + 1) ); 

                                spPrimitive->indices.push_back( (gy  + 1) * 5 + (gx + 1) ); 
                                spPrimitive->indices.push_back( (gy  + 0) * 5 + (gx + 1) ); 
                                spPrimitive->indices.push_back( (gy  + 0) * 5 + (gx + 0) ); 
                            }
                        }

                        size_t textureId = landscape.textureId[cy * 16 + cx];
                        size_t neighborId[4];
                        neighborId[0] = landscape.textureId[((cy > 0) ? (cy - 1) : cy) * 16 + cx];
                        neighborId[1] = landscape.textureId[cy * 16 + ((cx < 15) ? (cx + 1) : cx)];
                        neighborId[2] = landscape.textureId[((cy < 15) ? (cy + 1) : cy) * 16 + cx];
                        neighborId[3] = landscape.textureId[cy * 16 + ((cx > 0) ? (cx - 1) : cx)];



                        instance inst;
                        inst.primitive = *spPrimitive;
                        inst.transform = glm::translate(glm::mat4(), glm::vec3(landscape.gridX * 8192, landscape.gridY * 8192, 0.0f));
                
                        lx0::lxvar graph;
                        graph["_type"] = "phong";
                        graph["diffuse"]["_type"] = "texture2d_blend5";
                        graph["diffuse"]["uv"] = "fragUV";

                        LandscapeTexture ltex( ESMIterator(stream, *mEsmIndex.landscapeTextures[textureId]) );
                        inst.material.handle = ltex.texture;
                        _resolveMaterial(inst.material);

                        graph["diffuse"]["texture0"] = lx0::lxvar::wrap(std::shared_ptr<material_handle>(new material_handle(inst.material)));
                        for (size_t i = 0; i < 4; ++i)
                        {
                            material_handle material;
                            int texId  = neighborId[i];
                            LandscapeTexture ltex( ESMIterator(stream, *mEsmIndex.landscapeTextures[texId]) );
                            material.handle = ltex.texture;
                            _resolveMaterial(material);

                            std::string name = boost::str(boost::format("texture%d") % (i + 1));
                            graph["diffuse"][name] = lx0::lxvar::wrap(std::shared_ptr<material_handle>(new material_handle(material)));

                        }
                        inst.material.graph = graph;
                
                        group.instances.push_back(inst);
                    }
                }


                //
                // Add water
                //
                {
                    instance inst;
                    
                    inst.primitive.vertex.positions.resize(4);
                    inst.primitive.vertex.positions[0] = glgeom::point3f(0, 0, 0);
                    inst.primitive.vertex.positions[1] = glgeom::point3f(0, 8192, 0);
                    inst.primitive.vertex.positions[2] = glgeom::point3f(8192, 8192, 0);
                    inst.primitive.vertex.positions[3] = glgeom::point3f(8192, 0, 0);
                    inst.primitive.vertex.normals.resize(4);
                    inst.primitive.vertex.normals[0] = glgeom::vector3f(0, 0, 1);                   
                    inst.primitive.vertex.normals[1] = glgeom::vector3f(0, 0, 1);                   
                    inst.primitive.vertex.normals[2] = glgeom::vector3f(0, 0, 1);                   
                    inst.primitive.vertex.normals[3] = glgeom::vector3f(0, 0, 1);                   
                    inst.primitive.indices.resize(6);
                    inst.primitive.indices[0] = 0;
                    inst.primitive.indices[1] = 1;
                    inst.primitive.indices[2] = 2;
                    inst.primitive.indices[3] = 0;
                    inst.primitive.indices[4] = 2;
                    inst.primitive.indices[5] = 3;

                    // Is water height fixed for exteriors?
                    inst.transform = glm::translate(glm::mat4(), glm::vec3(landscape.gridX * 8192, landscape.gridY * 8192, 0));

                    // TEMP: special-case handle
                    inst.material.handle = "WATER";

                    group.instances.push_back(inst);
                }
            }
            else
                std::cout << "Could not find landscape at grid location\n";
        }

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
                        group.merge( *_getModel(*it, model.model) );
                        std::cout << "+ model " << model.model << "\n";
                    }
                    break;

                    case kId_ARMO:
                    {
                        Armor armor(iter);
                        group.merge( *_getModel(*it, armor.model) );
                        std::cout << "+ armor " << armor.model << "\n";
                    }
                    break;

                    case kId_CONT:
                    {
                        Container container(iter);
                        group.merge( *_getModel(*it, container.model) );
                        std::cout << "+ container " << container.model << "\n";
                    }
                    break;

                    case kId_DOOR:
                    {
                        Door door (iter);
                        group.merge( *_getModel(*it, door.model) );
                        std::cout << "+ door " << door.model << "\n";
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
                        esmLight.light.radius *= 70.0f;
                        esmLight.light.attenuation = glm::vec3(0, 3 * 70, 0);

                        //
                        // Add a glow effect.  This is not native to Morrowind.
                        //
                        esmLight.light.glow.radius = 120.0f;
                        esmLight.light.glow.multiplier = 0.5f;
                        esmLight.light.glow.exponent = 32.0f;

                        group.lights.push_back(esmLight.light);

                        if (!esmLight.model.empty())
                            group.merge( *_getModel(*it, esmLight.model) );

                        std::cout << "+ light " << esmLight.name << "\n";
                    }
                    break;

                    case kId_MISC:
                    {
                        Miscellaneous misc(iter);
                        group.merge( *_getModel(*it, misc.model) );
                        std::cout << "+ misc " << misc.model << "\n";
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
