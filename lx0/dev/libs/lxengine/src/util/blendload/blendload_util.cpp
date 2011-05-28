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

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/blendreader.hpp>
#include <lx0/util/blendload.hpp>

using namespace lx0;

lx0::GeometryPtr
lx0::util::blendload_ns::quadlist_from_blendfile (RasterizerGL& rasterizer, const char* filename)
{
    lx0::BlendReader reader;
    reader.open(filename);
            
    auto meshBlocks = reader.getBlocksByType("Mesh");

    switch (meshBlocks.size())
    {
    case 0:
        lx_error("Loaded .blend file but no mesh block were found");
        return lx0::GeometryPtr();
    case 1:
        // A-OK
        break;
    default:
        lx_warn("More than one mesh block detected.  Only the first will be read!");
    }

    for (auto it = meshBlocks.begin(); it != meshBlocks.end(); ++it)
    {
        auto spBlock = *it;
        auto spMesh = reader.readObject( spBlock->address );
        const auto totalVertices = spMesh->field<int>("totvert");
        const auto totalFaces = spMesh->field<int>("totface");

        std::vector<glgeom::point3f>  positions;
        std::vector<glgeom::vector3f> normals;
        std::vector<glgeom::color3f>  colors;
        std::vector<unsigned short> indicies;

        positions.reserve(totalVertices);
        normals.reserve(totalVertices);
        colors.reserve(totalVertices);
        indicies.reserve(totalFaces * 4);

        auto spVerts = reader.readObject( spMesh->field<unsigned __int64>("mvert") );
        for (int i = 0; i < totalVertices; ++i)
        {
            glgeom::point3f p;
            p.x = spVerts->field<float>("co", 0);
            p.y = spVerts->field<float>("co", 1);
            p.z = spVerts->field<float>("co", 2);
            p.vec *= 200;
            positions.push_back(p);

            glgeom::vector3f n;
            n.x = spVerts->field<short>("no", 0) / float(std::numeric_limits<short>::max());
            n.y = spVerts->field<short>("no", 1) / float(std::numeric_limits<short>::max());
            n.z = spVerts->field<short>("no", 2) / float(std::numeric_limits<short>::max());
            normals.push_back(n);

            colors.push_back( glgeom::color3f(1, 1, 1) );

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

            // Convert tris into degenerate quads
            if (vi[3] == 0)
                vi[3] = vi[2];

            for (int j = 0; j < 4; ++j)
                indicies.push_back(vi[j]);

            spFaces->next();
        }

        lx_debug("Loaded '%s'.  %u vertices, %u faces.", filename, positions.size(), indicies.size() / 4);

        return rasterizer.createQuadList(indicies, positions, normals, colors);
    }
}

