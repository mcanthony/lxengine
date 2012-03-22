//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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
#include <lx0/util/misc.hpp>

#include "nif.hpp"
#include "../tes3loader.hpp"
#include "esmiterator.hpp"
#include "esm_ids.hpp"

namespace bfs = boost::filesystem;


std::string _resolveTextureName (std::string texture)
{
    // Apparently Bethesda's level designers/artists used full res TGAs in the editor
    // and the production system automatically mapped these files to compressed DDS equivalents.
    // And apparently at least one BMP as well...
    //
    // Also, it appears all look-ups are case insensitive given that ESM and BSA names
    // do not always have consistent casing.
    //
    boost::to_lower(texture);
    if (boost::ends_with(texture, ".tga") || boost::ends_with(texture, ".bmp"))
        texture = texture.substr(0, texture.length() - 4) + ".dds";

    return std::string("textures\\") + texture;
}

//===========================================================================//
// BsaFile
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

    void            set     (EntryMap& map);
    Entry*          find    (const std::string& name);

    void            _dumpIndex ();

    std::string     mFilename;

protected:
    EntryMap        mEntries;
};

void
BsaFile::set (EntryMap& map)
{
    mEntries.swap(map);
}

BsaFile::Entry* 
BsaFile::find (const std::string& name)
{
    auto it = mEntries.find(name);
    return (it != mEntries.end()) ? &it->second : nullptr;
}

void
BsaFile::_dumpIndex ()
{
    //
    // Debugging tool: create an index of all the named entries
    //
    std::ofstream out(boost::str(boost::format("%s_index.txt") % boost::filesystem::path(mFilename).filename()));
    lx_check_error(out.good());
    for (auto it = mEntries.begin(); it != mEntries.end(); ++it)
        out << it->first << std::endl;
    out.close();
}

//===========================================================================//
// BsaCollection
//===========================================================================//

/*
    Represents a set of loaded BSA files.  Morrowind MODs include new BSA
    files, so a particular resource could be coming from any of a set of 
    BSA files.
 */
class BsaCollection
{
public:
    void initialize(const char* path);

    std::pair<bool,BsaFile::Entry*> getEntry    (std::string name);

    std::shared_ptr<scene_group>    getModel            (std::string type, std::string name);
    std::shared_ptr<std::istream>   getTextureStream    (std::string name);

protected:
    std::map< std::string, std::shared_ptr<scene_group> > mModelCache;
    std::vector<BsaFile> mBsas;
};

std::shared_ptr<scene_group>
BsaCollection::getModel (std::string type, std::string name)
{
    auto fullname = type + name;
    boost::to_lower(fullname);

    //
    // Models are frequently reused in the cells (for example, a latern fixture).
    // Therefore, cache the loaded results and return the cache if possible.
    //
    auto it = mModelCache.find(fullname);
    if (it == mModelCache.end())
    {
        for (auto jt = mBsas.begin(); jt != mBsas.end(); ++jt)
        {
            auto pEntry = jt->find(fullname);
            if (pEntry)
            {
                Stream stream;
                stream.open(jt->mFilename);
                stream.seekg(pEntry->offset);
                auto spSceneFragment = readNifObject(stream.stream(), _resolveTextureName);
                stream.close();

                mModelCache.insert(std::make_pair(fullname, spSceneFragment));
                return spSceneFragment;
            }
        }

        return std::shared_ptr<scene_group>();
    }
    else
        return it->second;
}


std::pair<bool,BsaFile::Entry*>
BsaCollection::getEntry (std::string name)
{
    for (auto jt = mBsas.begin(); jt != mBsas.end(); ++jt)
    {
        boost::to_lower(name);
        auto pEntry = jt->find(name);
        if (pEntry)
            return std::make_pair(true, pEntry);
    }
    return std::make_pair(false, nullptr);
}

std::shared_ptr<std::istream>   
BsaCollection::getTextureStream (std::string name)
{
    for (auto jt = mBsas.begin(); jt != mBsas.end(); ++jt)
    {
        boost::to_lower(name);
        auto pEntry = jt->find(name);
        if (pEntry)
        {
            std::shared_ptr<std::ifstream> spStream(new std::ifstream);
            spStream->open(jt->mFilename, std::ios::binary);
            spStream->seekg(pEntry->offset);
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

        BsaFile::EntryMap map;
        for (lx0::uint32 i = 0; i < fileCount; ++i)
        {
            BsaFile::Entry entry;
            entry.name = &nameTable[ nameOffsets[i] ];
            entry.size = fileOffsets[i].size; 
            entry.offset = dataOffset + fileOffsets[i].offset; 

            map.insert(std::make_pair(entry.name, entry));
        }
        bsa.set(map);
        stream.close();
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
                throw lx_error_exception("Unrecognized record id '%s'", iter.record_name().c_str()); 
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
                {
                    // INTV = Integer Values
                    //
                    // The signed 32-bit integer values are the grid location of this landscape
                    // cell.  Each cell takes up a 8192x8129 unit region of world space (in
                    // Morrowind units).  
                    //
                    gridX = iter.read();
                    gridY = iter.read();
                }
                break;

            case kId_VHGT:
                {
                    // VHGT = Vertex Height

                    //
                    // Offset, followed by 65x65 heights, then 3 bytes unknown.
                    // The heights are signed chars (-128 to 127).
                    //
                    float offset = iter.read();
                    std::vector<lx0::int8> buffer(65 * 65);
                    iter.read(&buffer[0], 65 * 65);
                    vertexHeight.reserve(65 * 65);

                    //
                    // The heights are differential encoded: each height is relative
                    // to the preceding heights.  
                    //
                    // More specifically, each height is simply an offset from the prior 
                    // height in that row.  The first height in a row is relative to the
                    // to the *first* offset in the prior row.  The first height in the
                    // first row is relative to the "offset" variable that precedes the
                    // 65x65 map.
                    //
                    // (This is a pretty smart format, by the way, given the characteristics of
                    // a heightmap - which is to say low variation from point to point
                    // but potentially large variation over the whole map.)
                    //
                    // Finally, note the the heights are *signed* bytes and are all 
                    // scaled by a factor of 8.
                    //
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
                    // VNML = Vertex Normals
                    //
                    // 65x65 array of signed bytes.  The signed bytes are simply to compress the
                    // normals.  Dividing by 127 and normalizing gives a "standard" 3-tuple float
                    // representation for the normal.
                    //
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
                    // VTEX = Vertex Textures

                    //
                    // Still working on figuring out what these values mean exactly...
                    //
                    // The textures for the 64x64 cells in Morrowind seem to be divided
                    // into 4x4 chunks (i.e. every 4x4 chunk has the same texture), 
                    // meaning ther are only 16x16 texture ids per LAND record.
                    //  
                    // But what exactly do these 16x16 16-bit values represent?  Are they
                    // indices into the array of LTEX records?  How do we take the 16-bits
                    // for a particular 4x4 region can know which LTEX record that's
                    // referencing??
                    //
                    textureId.resize(16 * 16);
                    iter.read(&textureId[0], textureId.size());
                }
                break;

            case kId_DATA:
            case kId_VCLR:  // VCLR = Vertex Color - ignored for now, but technically not hard to implement
            case kId_WNAM:
            default:
                {
                    //
                    // If the ID isn't recognized, just read the bytes into a buffer
                    // so we can view them in the debugger to try to figure out what
                    // they are...
                    //
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
    Cell (ESMIterator& iter);
    bool                isInterior () { return !!(flags & 0x01); }

    std::string             name;
    lx0::uint32             flags;
    int                     grid[2];
    float                   waterHeight;
    std::string             region;
    std::vector<char>       nam5;           // ?
    std::vector<Reference>  references;
};

Cell::Cell (ESMIterator& iter)
{
    waterHeight = 0.0f;

    bool done = false;
    while (!done && !iter.sub_done()) 
    { 
        switch (iter.sub_id())
        {
        case kId_DATA:
            flags = iter.read();
            grid[0] = iter.read();
            grid[1] = iter.read();
            break;
        case kId_FRMR:
            // Object reference list to follow
            done = true;
            break;
        case kId_NAME:
            name = iter.read_string();
            break;
        case kId_RGNN:
            region = iter.read_string();
            break;
        case kId_WHGT:
            waterHeight = iter.read();
            break;
        case kId_NAM5:
            nam5.resize( iter.sub_size() );
            iter.read(&nam5[0], nam5.size() );
            break;
        default:
            std::cout << "cell skip: " << iter.sub_name() << std::endl;
            break;
        }
        if (!done)
            iter.next_sub();
    }

    //
    // The main cell definition is followed by 0-N FRMR sub-records
    //
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
        references.push_back(ref);
    }
}

std::shared_ptr<Cell> 
loadCell (Stream& stream, Index& index, const char* name)
{
    auto it = index.names.find(name);
    if (it != index.names.end())
        return std::shared_ptr<Cell>(new Cell( ESMIterator(stream, *it->second) ) );
    else
        return std::shared_ptr<Cell>();
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

    void _resolveTexture (texture_handle& handle)
    {
        if (!handle.name.empty())
        {
            handle.name = _resolveTextureName(handle.name);
                                
            std::string name = handle.name;
            if (mBsaSet.getEntry(name).first)
            {
                handle.callback = [this, name]() {
                    return mBsaSet.getTextureStream(name);
                };
            }
            else
                lx_warn("Model references texture that doesn't exist '%s'", name.c_str());
        }
    }

    std::shared_ptr<scene_group> _getModel(Reference& ref, std::string modelName)
    {
        auto group = mBsaSet.getModel("meshes\\", modelName);
        
        std::shared_ptr<scene_group> subgroup(new scene_group);
        subgroup->merge(*group);

        auto transform = _transform(ref);
        for (auto kt = subgroup->instances.begin(); kt != subgroup->instances.end(); ++kt)
        {
            // Account for the cell reference transform in addition to the native transform in 
            // model itself.
            kt->spTransform.reset( new glm::mat4(transform * (*kt->spTransform)) );
        }
        for (auto kt = subgroup->textures.begin(); kt != subgroup->textures.end(); ++kt)
        {
            _resolveTexture(*kt);
        }

        return subgroup;
    }
    
    virtual void    cell        (const char* id, scene_group& group)
    {
        lx0::Timer timer;

        std::cout << boost::format("Loading cell '%s'...\n") % id;
        timer.start();

        Stream stream;
        stream.open(mEsmFilename);
        
        auto spCell = loadCell(stream, mEsmIndex, id);

        std::cout << "Interior = " << (spCell->isInterior() ? "yes" : "no") << std::endl;
        std::cout << "Region = " << spCell->region << std::endl;
        std::cout << boost::format("Grid = %d, %d\n") % spCell->grid[0] % spCell->grid[1];
        std::cout << "References = " << spCell->references.size() << std::endl;

        if (!spCell->isInterior())
        {
            //
            // Each CELL has a specific grid x,y: this corresponds to the same grid x,y as the
            // associated LAND record for the landscape data.  (Not sure if/what the CELL's grid
            // x,y is used for in the case of an interior cell.)  So grab the LAND record based
            // on the grid location.
            //
            auto it = mEsmIndex.landscapes.find(std::make_pair(spCell->grid[0], spCell->grid[1]));
            
            if (it != mEsmIndex.landscapes.end())
            {
                //
                // Read the Landscape object out of the ESM
                //
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
                // bullet and pass it in as a single primitive. (3) Use a single buffer for the
                // physics representation since texture has effect on the physics. (4) Other I'm sure.  
                // We'll save these optimizations for "someday"...
                //
                for (int cy = 0; cy < 16; ++cy)
                {
                    for (int cx = 0; cx < 16; ++cx)
                    {
                        //
                        // Allocate the buffer representation (5x5 vertices representing the 4x4 quad cells)
                        //
                        std::shared_ptr<glgeom::primitive_buffer> spPrimitive (new glgeom::primitive_buffer);
                        spPrimitive->type = "triangles";
                        spPrimitive->vertex.positions.resize(5 * 5);
                        spPrimitive->vertex.normals.resize(5 * 5);
                        spPrimitive->vertex.uv.resize(1);
                        spPrimitive->vertex.uv[0].resize(5 * 5);

                        //
                        // Read the vertex information: height, normal, uv, etc.
                        //
                        for (int gy = 0; gy < 5; ++gy)
                        {
                            for (int gx = 0; gx < 5; ++gx)
                            {
                                //
                                // The LAND record is a big 65x65 array.  The geometry we're creating is
                                // a 5x5 array.  Therefore separate offsets into each array are needed
                                // based on which 4x4 chunk we're creating.
                                //
                                int xL = (cx * 4 + gx);         // x-offset into the LAND array
                                int yL = (cy * 4 + gy);         // y-offset into the LAND array
                                int offsetL = yL * 65 + xL;     // offset into the LAND array
                                int offsetP = gy * 5 + gx;      // offset into the geometry array

                                //
                                // The LAND cell is 8192x8192, so at 64x64 cells that's 128 units per cell.
                                // Since the cells are uniformly spaced, the x/y position of each vertex is
                                // simply 128 times the x,y.
                                //
                                // The height is already scaled to Morrowind units in the Landscape object.
                                //
                                auto& p = spPrimitive->vertex.positions[offsetP];
                                p.x = xL * 128.0f;
                                p.y = yL * 128.0f;
                                p.z = landscape.vertexHeight[offsetL];

                                //
                                // Keep track of the bounds while building this object
                                //
                                spPrimitive->bbox.merge(p);

                                //
                                // Vertex normals are directly copied from the Landscape object: no scaling
                                // or anything unusual needed.
                                //
                                spPrimitive->vertex.normals[offsetP] = landscape.vertexNormal[offsetL];

                                //
                                // The texturing in LxMorrowind is not yet correct, but I'm *assuming* the
                                // UVs are uniformly distributed over the 4x4 regions.  I.e. the specified
                                // texture map is stretched from 0,0 to 1,1 across that 4x4 cell.
                                //
                                spPrimitive->vertex.uv[0][offsetP].vec = glm::vec2(gx / 4.0f, gy / 4.0f);
                            }
                        }

                        //
                        // Build the triangle mesh now that the vertex data is ready
                        //
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

                        //
                        // Set the bounding sphere as well
                        //
                        spPrimitive->bsphere = bsphere3_from(spPrimitive->bbox);

                        //
                        // Here's some guesswork since the landscape textures don't seem to be right
                        // yet...
                        //
                        // Presumably texture 0 means "NONE" so add 1 before offesting into the 
                        // LTEX array?  Otherwise take the x,y of the 4x4 region we just created
                        // and use that to index the ESM's LTEX record list.
                        // 
                        // (This doesn't seem like it can be right: what happens if multiple MODs add new
                        // LTEX records?  How could that possibly work as the index would change depending
                        // on the load order of those MODs.)
                        //
                        // Anyway, this gets us *a* texture to use for testing for now.
                        //
                        size_t textureId = landscape.textureId[cy * 16 + cx] + 1;

                        //
                        // Now do the same for the local neighbors of the current cell.  Do this so 
                        // that we can blend the edges with the neighbors.  Morrowind apparently blends
                        // up to 2 textures.  We'll do 5 here, because we can. 
                        //
                        size_t neighborId[4];
                        neighborId[0] = landscape.textureId[((cy > 0) ? (cy - 1) : cy) * 16 + cx] + 1;
                        neighborId[1] = landscape.textureId[cy * 16 + ((cx < 15) ? (cx + 1) : cx)] + 1;
                        neighborId[2] = landscape.textureId[((cy < 15) ? (cy + 1) : cy) * 16 + cx] + 1;
                        neighborId[3] = landscape.textureId[cy * 16 + ((cx > 0) ? (cx - 1) : cx)] + 1;

                        //
                        // Now take the geometry and bundle it up into a geometry, transform, material
                        //
                        // The transform is always (8192 * grid x,y) since each cell is 8192x8192 Morrowind
                        // units.
                        //
                        instance inst;
                        inst.spPrimitive = spPrimitive;
                        inst.spTransform = std::shared_ptr<glm::mat4>(new glm::mat4( glm::translate(glm::mat4(), glm::vec3(landscape.gridX * 8192, landscape.gridY * 8192, 0.0f)) ));
                
                        //
                        // Set up the material.  This uses the LxEngine ShaderBuilder system, which
                        // isn't well documented at the moment.  Basically construct a Phong material
                        // that sets the diffuse color via a shader fragment named "texture2d_blend5",
                        // which is a little shader function more or less created just for LxMorrowind
                        // sample a simple texture but also blend with it's neighbor textures along
                        // the edges.
                        //
                        lx0::lxvar graph;
                        graph["_type"] = "phong";
                        graph["diffuse"]["_type"] = "texture2d_blend5";     // Use the texture2d_blend5 ShaderBuilder node
                        graph["diffuse"]["uv"] = "fragUV";                  // Use the UV set on the fragment

                        //
                        // This next chunk simply loads up each of the LTEX records in order to grab
                        // the name over each landscape texture (we computed numeric IDs above, not
                        // names).
                        //
                        // These names get throw into the the "texture0", "texture1", etc. arguments of 
                        // the texture2d_blend5 shader node.  We also push a texture_handle for each one
                        // so that LxEngine can then know how to load these textures the first time 
                        // it seems an unrecognized name.
                        // 
                        LandscapeTexture ltex( ESMIterator(stream, *mEsmIndex.landscapeTextures[textureId]) );
                        texture_handle texture;
                        texture.name = ltex.texture;
                        _resolveTexture(texture);
                        group.textures.push_back(texture);

                        graph["diffuse"]["texture0"] = texture.name;
                        for (size_t i = 0; i < 4; ++i)
                        {
                            int texId  = neighborId[i];
                            LandscapeTexture ltex( ESMIterator(stream, *mEsmIndex.landscapeTextures[texId]) );
                            texture_handle texture;
                            texture.name = ltex.texture;
                            _resolveTexture(texture);

                            std::string name = boost::str(boost::format("texture%d") % (i + 1));
                            graph["diffuse"][name] = texture.name;
                            group.textures.push_back(texture);
                        }
                        inst.material["graph"] = graph;
                
                        group.instances.push_back(inst);
                    }
                }


                //
                // Add water
                //
                // Create a big 8192x8129 quad for transparent blue for now.
                //
                {
                    instance inst;
                    
                    inst.spPrimitive->type = "triangles";
                    inst.spPrimitive->vertex.positions.resize(4);
                    inst.spPrimitive->vertex.positions[0] = glgeom::point3f(0, 0, 0);
                    inst.spPrimitive->vertex.positions[1] = glgeom::point3f(0, 8192, 0);
                    inst.spPrimitive->vertex.positions[2] = glgeom::point3f(8192, 8192, 0);
                    inst.spPrimitive->vertex.positions[3] = glgeom::point3f(8192, 0, 0);
                    inst.spPrimitive->vertex.normals.resize(4);
                    inst.spPrimitive->vertex.normals[0] = glgeom::vector3f(0, 0, 1);                   
                    inst.spPrimitive->vertex.normals[1] = glgeom::vector3f(0, 0, 1);                   
                    inst.spPrimitive->vertex.normals[2] = glgeom::vector3f(0, 0, 1);                   
                    inst.spPrimitive->vertex.normals[3] = glgeom::vector3f(0, 0, 1);                   
                    inst.spPrimitive->indices.resize(6);
                    inst.spPrimitive->indices[0] = 0;
                    inst.spPrimitive->indices[1] = 1;
                    inst.spPrimitive->indices[2] = 2;
                    inst.spPrimitive->indices[3] = 0;
                    inst.spPrimitive->indices[4] = 2;
                    inst.spPrimitive->indices[5] = 3;

                    // Is water height fixed at 0 for exteriors?
                    *inst.spTransform = glm::translate(glm::mat4(), glm::vec3(landscape.gridX * 8192, landscape.gridY * 8192, 0));

                    // 
                    // Water is a special-case material since it should be more a "system"
                    // than a mere material
                    //
                    {
                        lx0::lxvar graph;
                        graph["_type"] = "solid_rgba";
                        graph["color"] = lx0::lxvar(0.388235294f, 0.615686275f, 0.776470588f, .4f);
                        
                        lx0::lxvar options;
                        options["blend"] = true;

                        inst.material["graph"] = graph;
                        inst.material["options"] = options;
                    }

                    group.instances.push_back(inst);
                }
            }
            else
                std::cout << "Could not find landscape at grid location\n";
        }

        //
        // Create instances for all the objects referenced in the cell
        //
        for (auto it = spCell->references.begin(); it != spCell->references.end(); ++it)
        {
            lx0::Timer tmLoad;
            tmLoad.start();

            auto jt = mEsmIndex.names.find(it->name);
            if (jt != mEsmIndex.names.end())
            {
                ESMIterator iter(stream, *jt->second);

                switch (jt->second->name_id)
                {
                    default:
                        std::cout << "- " << jt->second->name << "  " << it->name << " (type not handled)";
                    break;

                    case kId_STAT:
                    {
                        StaticModel model (iter);
                        group.merge( *_getModel(*it, model.model) );
                        std::cout << "+ model " << model.model;
                    }
                    break;

                    case kId_ARMO:
                    {
                        Armor armor(iter);
                        group.merge( *_getModel(*it, armor.model) );
                        std::cout << "+ armor " << armor.model;
                    }
                    break;

                    case kId_CONT:
                    {
                        Container container(iter);
                        group.merge( *_getModel(*it, container.model) );
                        std::cout << "+ container " << container.model;
                    }
                    break;

                    case kId_DOOR:
                    {
                        Door door (iter);
                        group.merge( *_getModel(*it, door.model) );
                        std::cout << "+ door " << door.model;
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

                        std::cout << "+ light " << esmLight.name;
                    }
                    break;

                    case kId_MISC:
                    {
                        Miscellaneous misc(iter);
                        group.merge( *_getModel(*it, misc.model) );
                        std::cout << "+ misc " << misc.model;
                    }
                    break;

                }
            }
            else
                std::cout << "- " << it->name << " (not indexed)" << std::endl;

            tmLoad.stop();
            std::cout << boost::format(" [%u ms]\n") % tmLoad.totalMs();
        }
        
        stream.close();
        timer.stop();

        std::cout << boost::format("Cell loaded [%u ms]\n") % timer.totalMs();
    }

    std::string     mEsmFilename;
    Index           mEsmIndex;
    BsaCollection   mBsaSet;
};

ITES3Loader* ITES3Loader::create()
{
    return new TES3Loader;
}
