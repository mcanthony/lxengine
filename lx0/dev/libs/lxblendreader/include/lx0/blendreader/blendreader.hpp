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

#pragma once

#include <iostream>
#include <fstream>

namespace lx0 { namespace blendreader {

    struct Structure;

    //=======================================================================//
    //! Information from the .blend header section
    /*!
     */
    struct Header
    {
        std::string identifier; 
        size_t      pointerSize;
        bool        littleEndian;
        int         version;
    };

    //=======================================================================//
    //! Info about a binary block in the .blend file
    /*!
     */
    struct Block
    {
        std::string      id;
        unsigned __int32 size;
        unsigned __int64 address;
        unsigned __int32 sdnaIndex;
        unsigned __int32 count;
    
        int                        filePos;
        std::shared_ptr<Structure> spStruct;
    };

    typedef std::shared_ptr<Block> BlockPtr;


    //=======================================================================//
    //! Self-describing data type from the .blend file
    /*!
     */
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

    typedef std::shared_ptr<Structure> StructurePtr;

    //=======================================================================//
    //! Cross-referencing structure to locate data in the .blend file
    /*!
        This structure is built-up from the information stored in the "DNA1"
        block of the .blend file.   This is not a direct mapping of that 
        information, but rather a set of indices for more easily and quickly
        locating particular pieces of interest.
     */
    struct DNA
    {
        std::vector<BlockPtr>                           blockIndex;
        std::map<std::string, std::vector<BlockPtr>>    blockMap;
        std::map<unsigned __int64, BlockPtr>            blockAddr;
        std::vector<StructurePtr>                       structIndex;
        std::map<std::string, StructurePtr>             structMap;
    };

    //=======================================================================//
    //! Object model for the .blend file
    /*!
     */
    class BlendReader
    {
    public:
        class Object 
        {
        public:
            friend class BlendReader;

            template <typename T>
            T                   field        (std::string ref, int index = 0);
            
            void                next         (void);

        protected:
            char*               fieldImp     (std::string ref, int index);

            StructurePtr        spStruct;
            BlockPtr            spBlock;
            std::vector<char>   chunk;
            char*               pCurrent;
        };

        typedef std::shared_ptr<Object> ObjectPtr;

        bool                    open                (std::string filename);
        ObjectPtr               readObject          (unsigned __int64 address);

        StructurePtr            getStructureByName  (std::string name);
        std::vector<BlockPtr>   getBlocksByType     (std::string type);

    protected:
        struct IO
        {
            unsigned __int64 (*read_address) (std::ifstream& file);
        };

        void          _readBlocks    (void);
        void          _indexBlocks   (void);

        IO            mIO;
        std::ifstream mFile;
        Header        mHeader;
        DNA           mDNA;
    };

    template <typename T>
    T BlendReader::Object::field (std::string ref, int index)
    {
        return *reinterpret_cast<T*>( fieldImp(ref, index) );
    }
}}
