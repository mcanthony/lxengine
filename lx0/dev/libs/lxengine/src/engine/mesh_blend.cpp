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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

#include <cassert>

#include <lx0/lxengine.hpp>
#include <lx0/engine/mesh.hpp>
#include <lx0/subsystem/blendreader.hpp>
#include <lx0/util/blendload.hpp>

using namespace lx0;
using namespace lx0::core;

namespace lx0 { namespace engine_ns {

    Mesh*
    load_blend (std::string filename)
    {
        glgeom::primitive_buffer primitive;
        lx0::primitive_buffer_from_blendfile(primitive, filename.c_str());

        Mesh* pMesh = new Mesh;
        pMesh->mFlags.mVertexNormals = true;

        for (size_t i = 0; i < primitive.vertex.positions.size(); ++i)
        {
            Mesh::Vertex v;
            v.position = primitive.vertex.positions[i];
            v.normal   = primitive.vertex.normals[i];
            pMesh->mVertices.push_back(v);
        }

        for (auto it = primitive.indices.begin(); it != primitive.indices.end(); /* */)
        {
            Mesh::Quad q;
            q.index[0] = *it++;
            q.index[1] = *it++;
            q.index[2] = *it++;
            q.index[3] = *it++;
            pMesh->mFaces.push_back(q);
        }

        return pMesh;
    }

}}
