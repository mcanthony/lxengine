//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

    Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)

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

#include <lx0/_detail/forward_decls.hpp>
#include <lx0/engine/object.hpp>
#include <glgeom/glgeom.hpp>

namespace lx0 { namespace engine { namespace dom_ns {

    lx0::engine::dom_ns::Mesh*     load_blend (std::string name);
    lx0::engine::dom_ns::Mesh*     load_lxson (lx0::lxvar& v);

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
        glgeom::vector3f    boundingVector  (void) const;

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
            glgeom::point3f  position;
            glgeom::vector3f normal;
        };

        Flags               mFlags;
        std::vector<Vertex> mVertices;
        std::vector<Quad>   mFaces;
    };

}}
using namespace lx0::engine::dom_ns;
}

