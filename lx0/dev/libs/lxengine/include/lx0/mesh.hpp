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

#include <lx0/core/detail/forward_decls.hpp>
#include <lx0/object.hpp>
#include <lx0/core/math/point3.hpp>

namespace lx0 { namespace dom {
    
    lx0::core::Mesh*     load_blend (std::string name);
    lx0::core::Mesh*     load_lxson (lx0::core::lxvar& v);

}}

namespace lx0 { namespace core {

    //===========================================================================//
    //!
    /*!
     */   
    class Mesh : public Object
    {
    public:
                            Mesh (void);

        virtual lxvalue*    clone           (void) const { return nullptr; }

        float               boundingRadius  (void) const;
        vector3             boundingVector  (void) const;

        float               maxExtentScale  (lxvar attr) const;


    //protected:
        struct Flags
        {
            bool    mVertexNormals;
        };
        struct Quad
        {
            int index[4];
        };

        struct Vertex
        {
            point3  position;
            vector3 normal;
        };

        Flags               mFlags;
        std::vector<Vertex> mVertices;
        std::vector<Quad>   mFaces;
    };

}}

