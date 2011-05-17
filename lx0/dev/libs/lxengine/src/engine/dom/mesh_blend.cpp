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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

#include <cassert>

#include <lx0/lxengine.hpp>
#include <lx0/engine/mesh.hpp>
#include <lx0/subsystem/blendreader.hpp>

using namespace lx0;
using namespace lx0::core;

namespace {
    float normalizeShort (short s)
    {
        return float (s) / float(std::numeric_limits<short>::max());
    }
}

namespace lx0 { namespace dom {

    Mesh*
    load_blend (std::string filename)
    {
        BlendReader reader;
        if ( reader.open(filename) )
        {
            Mesh* pMesh = new Mesh;

            auto meshBlocks = reader.getBlocksByType("Mesh");

            if (meshBlocks.size() != 1)
            {
                lx_warn("More than one mesh found in .blend file.  Processing only the "
                        "first one that is found.");
            }

            auto spObj = reader.readObject(meshBlocks[0]->address);
            auto numVerts = spObj->field<int>("totvert");
            auto numFaces = spObj->field<int>("totface");

            pMesh->mVertices.reserve(numVerts);
            pMesh->mFaces.reserve(numFaces);

            pMesh->mFlags.mVertexNormals = true;

            auto spVerts = reader.readObject(spObj->field<unsigned __int64>("mvert"));
            for (int i = 0; i < numVerts; ++i)
            {
                Mesh::Vertex v;
                v.position = spVerts->field<glgeom::point3f>("co", 0);

                // Normals are encoded as shorts
                v.normal.x = normalizeShort( spVerts->field<short>("no", 0) );
                v.normal.y = normalizeShort( spVerts->field<short>("no", 1) );
                v.normal.z = normalizeShort( spVerts->field<short>("no", 2) );

                pMesh->mVertices.push_back(v);
                spVerts->next();
            }
            spVerts.reset();

            auto spFaces = reader.readObject(spObj->field<unsigned __int64>("mface"));
            for (int i = 0; i < numFaces; ++i)
            {
                Mesh::Quad q;
                q.index[0] = spFaces->field<int>("v1");
                q.index[1] = spFaces->field<int>("v2");
                q.index[2] = spFaces->field<int>("v3");
                q.index[3] = spFaces->field<int>("v4");

                pMesh->mFaces.push_back(q);
                spFaces->next();
            }
            spFaces.reset();

            return pMesh;
        }
        else
        {
            lx_error("Could not open file '%s'", filename.c_str());
            return nullptr;
        }
    }

}}
