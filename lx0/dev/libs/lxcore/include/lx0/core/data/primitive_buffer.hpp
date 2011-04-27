//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

#include <vector>
#include <memory>
#include <map>
#include <glgeom/glgeom.hpp>

namespace lx0 { namespace core {

    namespace detail {

        class primitive_buffer_imp
        {
        public:
            virtual void add_stream     (const char* name, std::vector<float>& data) = 0;
            virtual void add_stream     (const char* name, std::vector<glgeom::vector3f>& data) = 0;
        };

        class primitive_buffer_sw : public primitive_buffer_imp
        {
        public:
            virtual void add_stream     (const char* name, std::vector<float>& data);
            virtual void add_stream     (const char* name, std::vector<glgeom::vector3f>& data);

            struct Stream
            {
                std::string type;
                size_t      count;
                union
                {
                    float*      pFloat;
                    glgeom::vector3f*    pVector3;
                };
            };
            std::map<std::string, Stream> mStreams;
        };
    }

    /*!
        A primitive_buffer is designed to be a generic container for multi-stream data.

        The class uses the Imp design pattern so that the internal representation can be
        switched dynamically based on its usage characteristics.  For example, when the
        primitive is being rendered to HW, the internal representation may be a hardware
        buffer.  When the representation is a geometry processor, a software representation
        would be more effective.
     */
    class primitive_buffer
    {
    public:
        primitive_buffer();

        void add_stream     (const char* name, std::vector<float>& data)        { mpImp->add_stream(name, data); }
        void add_stream     (const char* name, std::vector<glgeom::vector3f>& data)      { mpImp->add_stream(name, data); }

    protected:
        typedef std::unique_ptr<detail::primitive_buffer_imp> ImpPtr;

        ImpPtr    mpImp;
    };

    typedef std::shared_ptr<primitive_buffer> PrimitiveBufferPtr;
}}
