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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

#include <glm/gtc/matrix_inverse.hpp>

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/blendreader.hpp>
#include <lx0/util/blendload.hpp>

using namespace lx0;

//===========================================================================//
//   I M P L E M E N T A T I O N 
//===========================================================================//

lx0::GeometryPtr    
lx0::util::blendload_ns::geometry_from_blendfile (lx0::RasterizerGLPtr spRasterizer, const char* filename)
{
    glgeom::abbox3f bbox;
    auto spGeometry = lx0::quadlist_from_blendfile(*spRasterizer.get(), filename, 1.0f, &bbox);
    spGeometry->mBBox = bbox;

    return spGeometry;
}

lx0::GeometryPtr
lx0::util::blendload_ns::quadlist_from_blendfile (RasterizerGL& rasterizer, const char* filename, float scale, glgeom::abbox3f* pBounds)
{
    glgeom::primitive_buffer primitive;
    glm::mat4 scaleMat = glm::scale(glm::mat4(), glm::vec3(scale, scale, scale));
    
    primitive_buffer_from_blendfile(primitive, filename, scaleMat);

    if (pBounds)
        *pBounds = primitive.bbox;
    return rasterizer.createQuadList(primitive.indices, primitive.face.flags, primitive.vertex.positions, primitive.vertex.normals, primitive.vertex.colors);
}

static
void
_primitive_buffer_from_block (glgeom::primitive_buffer& primitive, lx0::BlendReader& reader, lx0::BlockPtr spBlock, const glm::mat4& pretransform)
{
    auto spMesh = reader.readObject( spBlock->address );
    const auto totalVertices = spMesh->field<int>("totvert");
    const auto totalFaces = spMesh->field<int>("totface");

    primitive.type = "quadlist";

    //
    // Reserve the memory needed in advance for efficiency
    //
    primitive.indices.reserve(totalFaces * 4);
    primitive.vertex.positions.reserve(totalVertices);
    primitive.vertex.normals.reserve(totalVertices);
    primitive.vertex.colors.reserve(totalVertices);
    primitive.face.flags.reserve(totalFaces);

    glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(pretransform));

    //
    // Walk the vertices
    //
    auto spVerts = reader.readObject( spMesh->address("mvert") );
    for (int i = 0; i < totalVertices; ++i)
    {
        glm::vec4 p;
        p.x = spVerts->field<float>("co", 0);
        p.y = spVerts->field<float>("co", 1);
        p.z = spVerts->field<float>("co", 2);
        p.w = 1.0f;
        p = pretransform * p;
        glgeom::point3f p2(p.x, p.y, p.z);
        primitive.vertex.positions.push_back(p2);
        primitive.bbox.merge(p2);

        glgeom::vector3f n;
        n.x = spVerts->field<short>("no", 0) / float(std::numeric_limits<short>::max());
        n.y = spVerts->field<short>("no", 1) / float(std::numeric_limits<short>::max());
        n.z = spVerts->field<short>("no", 2) / float(std::numeric_limits<short>::max());
        n.vec = normalMatrix * n.vec;
        primitive.vertex.normals.push_back(n);

        primitive.vertex.colors.push_back( glgeom::color3f(1, 1, 1) );

        spVerts->next();
    }

    //
    // Walk the faces
    //
    auto spFaces = reader.readObject( spMesh->address("mface") );
    for (int i = 0; i < totalFaces; ++i)
    {
        int vi[4];
        vi[0] = spFaces->field<int>("v1");
        vi[1] = spFaces->field<int>("v2");
        vi[2] = spFaces->field<int>("v3");
        vi[3] = spFaces->field<int>("v4");
            
        // Convert tris into degenerate quads
        // Is there a better way to handle meshes which can potentially contain tri/quad mixes?
        if (vi[3] == 0)
            vi[3] = vi[2];

        for (int j = 0; j < 4; ++j)
            primitive.indices.push_back(vi[j]);

        // The blender source code implies that
        // 1 = ME_SMOOTH
        // 2 = ME_FACE_SEL
        lx0::uint8 flag = spFaces->field<lx0::uint8>("flag");
        primitive.face.flags.push_back(flag);

        spFaces->next();
    }

    //
    // Compute the bounding sphere from the bounding box
    // (Tighter bounds than calling merge() iteratively).
    //
    primitive.bsphere = glgeom::bsphere3f(primitive.bbox);

    //
    // Check post-conditions
    //
    lx_check_error( !primitive.vertex.positions.empty() );
    lx_check_error( primitive.vertex.normals.empty() || primitive.vertex.normals.size() == primitive.vertex.positions.size() );
    lx_check_error( primitive.vertex.colors.empty() || primitive.vertex.colors.size() == primitive.vertex.positions.size() );
    lx_check_error( primitive.bbox.is_finite() );
    lx_check_error( primitive.bsphere.is_finite() );
}

/*!
    \param pretransform 
        Applies a transform to each vertex as the file is read.
 */
void 
lx0::util::blendload_ns::primitive_buffer_from_blendfile (glgeom::primitive_buffer& primitive, const char* filename, const glm::mat4& pretransform)
{
    lx_check_error( lx0::file_exists(filename) );

    //
    // Use the BlendReader to iterate over the .blend data
    //
    lx0::BlendReader reader;
    reader.open(filename);
            
    //
    // Currently, the function only support files with a single "Mesh" entry.  I
    //
    auto meshBlocks = reader.getBlocksByType("Mesh");
    switch (meshBlocks.size())
    {
    case 0:
        lx_error("Loaded .blend file but no mesh block were found");
    case 1:
        // A-OK
        break;
    default:
        lx_warn("More than one mesh block detected.  Only the first will be read!");
    }

    _primitive_buffer_from_block(primitive, reader, meshBlocks.front(), pretransform);

    lx_debug("Loaded '%s'.  %u vertices, %u faces.", filename, primitive.vertex.positions.size(), primitive.indices.size() / 4);
}
