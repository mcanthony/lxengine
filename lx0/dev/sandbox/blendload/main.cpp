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
#include <sstream>
#include <fstream>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>
#include <iomanip>

// Lx0 headers
#include <lx0/core.hpp>
#include <lx0/lxvar.hpp>
#include <lx0/util.hpp>

using namespace lx0::core;

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

unsigned __int64 read_ptr32 (std::ifstream& file)
{
    __int32 i;
    file.read((char*)&i, 4);
    return __int64(i);
}

unsigned __int64 read_ptr64 (std::ifstream& file)
{
    __int64 i;
    file.read((char*)&i, 8);
    return i;
}

__int32 read_int (std::ifstream& file)
{
    __int32 i;
    file.read((char*)&i, 4);
    return i;
}

std::string read_str (std::ifstream& file, size_t len)
{
    std::vector<char> buffer(len + 1);
    file.read(&buffer[0], len);
    buffer[len] = 0;
    return std::string(&buffer[0]);
}

unsigned short read_u16 (std::ifstream& file)
{
    unsigned short i;
    file.read((char*)&i, 2);
    return i;
}

void read_string_array(std::ifstream& file, std::vector<std::string>& names)
{
    unsigned int nameCount = read_int(file);
    for (unsigned int i = 0; i < nameCount; ++i)
    {
        std::string t;

        char c;
        while ((c = (char)file.get()))
            t += c;

        names.push_back(t);
    }
}

void file_align (std::ifstream& file, int align)
{
    int pos = (int)file.tellg();
    int offset = (align - (pos % align)) % align;
    file.seekg(pos + offset);
}

struct Header
{
    std::string identifier; 
    size_t      pointerSize;
    bool        littleEndian;
    int         version;
};

struct Block
{
    std::string      id;
    unsigned __int32 size;
    unsigned __int64 address;
    unsigned __int32 sdnaIndex;
    unsigned __int32 count;
    
    int              filePos;

    std::string      type;
};


struct Structure
{
    struct Field
    {
        std::string ref;
        std::string type;
        std::string name;
        size_t      dim;
        size_t      offset;
        size_t      size;
    };

    std::string         name;
    size_t              size;
    std::vector<Field>  fields;
    std::map<std::string, Field*> fieldMap;
};

std::vector<std::shared_ptr<Block>> blockIndex;
std::map<std::string, std::vector<std::shared_ptr<Block>>> blockMap;
std::map<unsigned __int64, std::shared_ptr<Block>> blockAddr;
std::vector<std::shared_ptr<Structure>> structIndex;
std::map<std::string, std::shared_ptr<Structure>> structMap;

void displayStructure (std::string name)
{
    auto spStruct = structMap[name];
    std::cout << name << std::endl;
    for (auto it = spStruct->fields.begin(); it != spStruct->fields.end(); ++it)
    {
        std::cout << "\t" << std::setw(16) << it->type << "\t" << it->name 
            << " (" << it->ref << ")"
            << " (" << it->offset << " : " << it->size << " bytes)" <<std::endl;
    }
    std::cout << std::endl;
}

class BlendReader
{
public:
    void    open    (std::string filename);
    bool    is_open (void)
    {
        return file.is_open();
    }

   
    struct Object 
    {
        template <typename T>
        T field (std::string ref, int index = 0)
        {
            auto spField = spStruct->fieldMap[ref];
            char* pBase = &pCurrent[ spField->offset ];
            if (index != 0)
                pBase += (spField->size / spField->dim) * index;
            
            lx_check_error(size_t(pBase - &chunk[0]) < chunk.size());

            return *reinterpret_cast<T*>(pBase);
        }

        void next()
        {
            pCurrent += spStruct->size;
        }

        std::shared_ptr<Structure> spStruct;
        std::shared_ptr<Block>     spBlock;
        std::vector<char>          chunk;
        char*                      pCurrent;
    };

    std::shared_ptr<Object> 
    readObject (unsigned __int64 address)
    {
        lx_check_error(address != 0);
        lx_check_error(file.is_open());

        std::shared_ptr<Object> spObj (new Object);
        
        spObj->spBlock = blockAddr[address];
        spObj->spStruct = structMap[spObj->spBlock->type];

        spObj->chunk.resize( spObj->spBlock->size );
        file.seekg( spObj->spBlock->filePos );
        file.read(&spObj->chunk[0], spObj->chunk.size());

        spObj->pCurrent = &spObj->chunk[0];

        return spObj;
    }

//protected:

    Header        mHeader;
    std::ifstream file;
};

static void 
readHeader (std::ifstream& file, Header& header)
{
    // The header is always 12-bytes long.  Read it all
    // as a single chunk, then process that data
    //
    char buffer[12];
    file.read(buffer, 12);
    
    // First 7 characters are the BLENDER id
    //
    header.identifier.resize(7);
    for (int i = 0; i < 7; ++i)
        header.identifier[i] = buffer[i];
    lx_check_error(header.identifier == "BLENDER");

    // Next character indicates the pointer size on the
    // system that saved the file (will affect the size
    // of all stored pointers in the file).
    //
    if (buffer[7] == '_')
        header.pointerSize = 4;
    else if (buffer[7] == '-')
        header.pointerSize = 8;
    else
        lx_error("Unrecognized pointer size!");

    // Next character indicates the endianness with which
    // the numbers were saved.
    //
    if (buffer[8] == 'v')
        header.littleEndian = true;
    else if (buffer[8] == 'V')
        header.littleEndian = false;
    else
        lx_error("Unrecognized little endian");

    // The final three characters are the version number, written as 
    // 3 ascii characters
    header.version = (buffer[9] - '0') * 100 + (buffer[10] - '0') * 10 + (buffer[11] - '0');
}

static void
readBlockDNA1 (std::ifstream& file, const Header& header)
{
    std::string sdna = read_str(file, 4);
                    
    std::vector<std::string> names;
    std::string name = read_str(file, 4);
    read_string_array(file, names);

    file_align(file, 4);

    std::vector<std::string> types;            
    std::string type = read_str(file, 4);
    read_string_array(file, types);

    file_align(file, 4);

    std::string tlen = read_str(file, 4);
    std::vector<unsigned short> typeSizes(types.size());
    for (size_t i = 0; i < types.size(); ++i)
    {
        file.read((char*)&typeSizes[i], 2);
    }

    file_align(file, 4);

    std::string strc = read_str(file, 4);
    unsigned int count = read_int(file);
    for (unsigned int i = 0; i < count; ++i)
    {
        std::shared_ptr<Structure> spStruct(new Structure);

        unsigned short type;
        file.read((char*)&type, 2);

        spStruct->name = types[type];
        spStruct->size = typeSizes[type];

        unsigned short fields;
        file.read((char*)&fields, 2);

        size_t currentOffset = 0;
        spStruct->fields.reserve(fields);
        for (unsigned short j = 0; j < fields; j++)
        {
            auto typeIndex = read_u16(file);
            auto nameIndex = read_u16(file);
                            
            Structure::Field f;
            f.ref = "";
            f.type = types[typeIndex];
            f.name = names[nameIndex];
            f.offset = currentOffset;
            f.size = 0;

            const char* p = f.name.c_str();
            while (*p == '*') p++;
            while (*p != '[' && *p != '\0') f.ref += *p++;

            f.dim = 1;
            if (*p == '[')
            {
                p++;
                do
                {
                    f.dim += int(*p - '0');
                    p++;
                } while (*p != ']');
            }

            if (f.name[0] == '*')
                f.size = header.pointerSize;
            else
                f.size = typeSizes[typeIndex];
            f.size *= f.dim;

            currentOffset += f.size;
            spStruct->fields.push_back(f);
            spStruct->fieldMap[f.ref] = &spStruct->fields.back();
        }

        structIndex.push_back(spStruct);
        structMap.insert(std::make_pair(spStruct->name, spStruct));
    }
}

void    
BlendReader::open (std::string filename)
{
    file.open (filename, std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        readHeader(file, mHeader);
    }
}

int 
main (int argc, char** argv)
{
    lx_init();

    BlendReader reader;
    reader.open("media/models/unit_cube.blend");
    std::ifstream& file = reader.file;
    if (reader.is_open())
    {
        unsigned __int64 (*read_ptr)   (std::ifstream& file) = nullptr;

        Header& header = reader.mHeader;
        if (header.pointerSize == 8)
            read_ptr = read_ptr64;
        else
            read_ptr = read_ptr32;


        // Loop over the blocks in the file
        //
        bool bDone = false;
        while (!bDone)
        {
            std::shared_ptr<Block> spBlock(new Block);         
            spBlock->id = read_str(file, 4);
            spBlock->size = read_int(file);
            spBlock->address = read_ptr(file);
            spBlock->sdnaIndex = read_int(file);
            spBlock->count = read_int(file);

            spBlock->filePos = (int)file.tellg();
            int nextBlock = (int)file.tellg();
            nextBlock += int(spBlock->size);

            blockIndex.push_back(spBlock);

            if (spBlock->id == "ENDB")
                bDone = true;
            else if (spBlock->id == "DNA1")
                readBlockDNA1(file, header);

            file.seekg(nextBlock);
        }
        
        int i = 0;
        for (auto it = blockIndex.begin(); it != blockIndex.end(); ++it)
        {
            (*it)->type = structIndex[(*it)->sdnaIndex]->name;
            blockMap[(*it)->type].push_back(*it);
            blockAddr[(*it)->address] = *it;
            ++i;
        }

        std::cout << header.identifier << " " << header.version << std::endl;
        std::cout << "Pointer size: " << header.pointerSize << std::endl;
        std::cout << "Litte endian: " << (header.littleEndian ? "true" : "false") << std::endl;

        displayStructure("Mesh");
        displayStructure("MVert");
        displayStructure("MFace");
        displayStructure("MTFace");

        std::cout << "Meshs = " << blockMap["Mesh"].size() << std::endl;
        
        for (auto it = blockMap["Mesh"].begin(); it != blockMap["Mesh"].end(); ++it)
        {
            auto spBlock = *it;
            std::cout << "Mesh address: 0x" << spBlock->address << std::endl;
            auto spMesh = reader.readObject( spBlock->address );

            printf("%c%c%c%c\n", spMesh->pCurrent[0],
                spMesh->pCurrent[1],
                spMesh->pCurrent[2],
                spMesh->pCurrent[3]);

            auto totalVertices = spMesh->field<int>("totvert");
            auto totalFaces = spMesh->field<int>("totface");
            std::cout << "Total vertices: " << totalVertices << std::endl;

            auto spVerts = reader.readObject( spMesh->field<unsigned __int64>("mvert") );
            for (int i = 0; i < totalVertices; ++i)
            {
                float x = spVerts->field<float>("co", 0);
                float y = spVerts->field<float>("co", 1);
                float z = spVerts->field<float>("co", 2);

                std::cout << "V" << i << ": " << x << ", " << y << ", " << z << std::endl;

                spVerts->next();
            }

            auto spFaces = reader.readObject( spMesh->field<unsigned __int64>("mface") );
            for (int i = 0; i < totalFaces; ++i)
            {
                int vi[4];
                vi[0] = spFaces->field<int>("v1");
                vi[1] = spFaces->field<int>("v2");
                vi[2] = spFaces->field<int>("v3");
                vi[3] = spFaces->field<int>("v4");

                std::cout << "F" << i << ": " 
                    << vi[0] << ", " 
                    << vi[1] << ", "
                    << vi[2] << ", "
                    << vi[3] << std::endl;

                spFaces->next();
            }
        }
    }
    else
        lx_warn("Could not open file!");

    return 0;
}
