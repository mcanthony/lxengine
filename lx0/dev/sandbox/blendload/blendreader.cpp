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
#include <lx0/vector3.hpp>
#include <lx0/util.hpp>

#include "blendreader.hpp"

using namespace lx0::core;

namespace lx0 { namespace blendreader { 

    namespace io_util
    {
        /*!
            Aligns the file pointer to the next position that is evenly divisble
            by "align".
         */
        void file_align (std::ifstream& file, int align)
        {
            int pos = (int)file.tellg();
            int offset = (align - (pos % align)) % align;
            file.seekg(pos + offset);
        }

        /*!
            Read a 32-bit little endian pointer and return it as a
            native unsigned 64 bit address.
         */
        unsigned __int64 read_addr_32L (std::ifstream& file)
        {
            __int32 i;
            file.read((char*)&i, 4);
            return __int64(i);
        }

        unsigned __int64 read_addr_64L (std::ifstream& file)
        {
            __int64 i;
            file.read((char*)&i, 8);
            return i;
        }

        unsigned short read_u16 (std::ifstream& file)
        {
            unsigned short i;
            file.read((char*)&i, 2);
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

    }

    using namespace io_util;


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
    readBlockDNA1 (std::ifstream& file, const Header& header, DNA& dna)
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

                if (*p == '[')
                {
                    f.dim = 0;

                    p++;
                    do
                    {
                        f.dim += int(*p - '0');
                        p++;
                        if (*p == ']')
                            break;
                        else
                            f.dim *= 10;
                    } while (*p != ']');
                }
                else
                    f.dim = 1;

                if (f.name[0] == '*')
                    f.size = header.pointerSize;
                else
                    f.size = typeSizes[typeIndex];
                f.size *= f.dim;

                currentOffset += f.size;
                spStruct->fields.push_back(f);
                spStruct->fieldMap[f.ref] = &spStruct->fields.back();
            }

            dna.structIndex.push_back(spStruct);
            dna.structMap.insert(std::make_pair(spStruct->name, spStruct));
        }
    }

    //===========================================================================//
    //   BlendReader
    //===========================================================================//

    void    
    BlendReader::Object::next (void)
    {
        pCurrent += spStruct->size;
    }

    bool    
    BlendReader::open (std::string filename)
    {
        mFile.open (filename, std::ios::in | std::ios::binary);
        if (mFile.is_open())
        {
            readHeader(mFile, mHeader);

            switch (mHeader.pointerSize)
            {
            case 8: mIO.read_address = read_addr_64L; break;
            case 4: mIO.read_address = read_addr_32L; break;
            default: 
                lx_error("Unexpected pointer size read from .blend file");
            }

            _readBlocks();
            _indexBlocks();
        }

        return mFile.is_open();
    }


    /*!
        Stage 1 read in the block data.
        Stage 2 is to index them via indexBlocks().
     */
    void
    BlendReader::_readBlocks (void)
    {
        // Loop over the blocks in the file
        //
        bool bDone = false;
        while (!bDone)
        {
            std::shared_ptr<Block> spBlock(new Block);         
            spBlock->id = read_str(mFile, 4);
            spBlock->size = read_int(mFile);
            spBlock->address = mIO.read_address(mFile);
            spBlock->sdnaIndex = read_int(mFile);
            spBlock->count = read_int(mFile);

            spBlock->filePos = (int)mFile.tellg();
            int nextBlock = (int)mFile.tellg();
            nextBlock += int(spBlock->size);

            mDNA.blockIndex.push_back(spBlock);

            if (spBlock->id == "ENDB")
                bDone = true;
            else if (spBlock->id == "DNA1")
                readBlockDNA1(mFile, mHeader, mDNA);

            mFile.seekg(nextBlock);
        }
    }

    /*!
        After the initial read of the blocks (including the .blend DNA structure),
        build some cross-referencing index structures so that data can quickly
        be found by type name, address, etc.
     */
    void          
    BlendReader::_indexBlocks (void)
    {
        for (auto it = mDNA.blockIndex.begin(); it != mDNA.blockIndex.end(); ++it)
        {
            (*it)->spStruct = mDNA.structIndex[(*it)->sdnaIndex];
            mDNA.blockMap[(*it)->spStruct->name].push_back(*it);
            mDNA.blockAddr[(*it)->address] = *it;
        }
    }

    std::shared_ptr<BlendReader::Object> 
    BlendReader::readObject (unsigned __int64 address)
    {
        lx_check_error(address != 0);
        lx_check_error(mFile.is_open());

        std::shared_ptr<Object> spObj (new Object);
        
        spObj->spBlock = mDNA.blockAddr[address];
        spObj->spStruct = spObj->spBlock->spStruct;

        spObj->chunk.resize( spObj->spBlock->size );
        mFile.seekg( spObj->spBlock->filePos );
        mFile.read(&spObj->chunk[0], spObj->chunk.size());

        spObj->pCurrent = &spObj->chunk[0];

        return spObj;
    }

    StructurePtr
    BlendReader::getStructureByName (std::string name)
    {
        return mDNA.structMap[name];
    }

    std::vector<BlockPtr>
    BlendReader::getBlocksByType (std::string type)
    {
        return mDNA.blockMap[type];
    }

}}

