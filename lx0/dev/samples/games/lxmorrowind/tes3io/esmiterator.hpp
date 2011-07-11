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

#include "stream.hpp"

struct SubRecordHeader
{
    SubRecordHeader (Stream& stream)
    {
        offset = stream.tellg();
        stream.read5(name);
        stream.read(&size);
    }

    int         offset; 
    char        name[5];
    lx0::uint32 size;
};

struct RecordHeader : public SubRecordHeader
{
    RecordHeader(Stream& stream)
        : SubRecordHeader (stream)
    {
        stream.read(&header);
        stream.read(&flags);
    }

    lx0::uint32 header;
    lx0::uint32 flags;
};

struct ESMIterator
{
    ESMIterator (Stream& stream)
        : mCurrentRecordHeader      (stream)
        , mCurrentSubRecordHeader   (stream)
        , mStream                   (stream)
    {
    }

    ESMIterator (Stream& stream, const RecordHeader& recordHeader)
        : mCurrentRecordHeader      (recordHeader)
        , mCurrentSubRecordHeader   (stream)
        , mStream                   (stream)
    {
        mStream.seekg( recordHeader.offset );
        new (&mCurrentRecordHeader) RecordHeader(mStream);
        new (&mCurrentSubRecordHeader) SubRecordHeader(mStream);
    }

    bool is_record (const char* s)
    {
        return strncmp(s, mCurrentRecordHeader.name, 4) == 0;
    }

    void next_record()
    {
        lx_check_error(mStream.good());
        mStream.seekg(mCurrentRecordHeader.offset + mCurrentRecordHeader.size + 16);
        new (&mCurrentRecordHeader) RecordHeader(mStream);
        new (&mCurrentSubRecordHeader) SubRecordHeader(mStream);
    }

    const RecordHeader& record_header () const { return mCurrentRecordHeader; }


    bool        is_sub  (const char* s) { return strncmp(s, mCurrentSubRecordHeader.name, 4) == 0; }
    lx0::uint32 sub_size() { return mCurrentSubRecordHeader.size; }
    void next_sub() { mStream.seekg( mCurrentSubRecordHeader.offset + mCurrentSubRecordHeader.size + 8); new (&mCurrentSubRecordHeader) SubRecordHeader(mStream); }

    //
    bool                eof         (void)                                  { return mStream.eof(); }
    bool                good        (void) const                            { return mStream.good(); }
    lx0::uint32         tellg       (void)                                  { return mStream.tellg(); }
    void                seekg       (lx0::uint32 pos)                       { mStream.seekg(pos); }
    void                skip    (   size_t bytes)                           { mStream.skip(bytes); }
    
    void                read        (char* data, size_t count)              { mStream.read(data, count); }
    void                read5       (char* data)                            { mStream.read5(data); }
    void                read        (float* data, size_t count = 1)         { mStream.read(data, count); }
    void                read        (lx0::uint32* data, size_t count = 1)   { mStream.read(data, count); }
    std::string         read_string (void)                                  { return mStream.read_string(mCurrentSubRecordHeader.size); }
    Stream::auto_cast   read        (void)                                  { return mStream.read(); }
    //

protected:
    RecordHeader    mCurrentRecordHeader;
    SubRecordHeader mCurrentSubRecordHeader;
    Stream&         mStream;
};
