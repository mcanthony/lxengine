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

#include <lx0/util/blendload.hpp>
#include <lx0/libs/rasterizer.hpp>

using namespace lx0::subsystem::rasterizer_ns;

MeshCache::MeshCache(std::shared_ptr<RasterizerGL> spRasterizer)
    : mspRasterizer     (spRasterizer)
{
    mStats.acquireCount = 0;
}

MeshCache::~MeshCache()
{
    lx_log("MeshCache dtor");
}

static lx0::GeometryPtr 
_createGeometry (RasterizerGL* spRasterizer, const char* filename, float scale)
{

    glgeom::primitive_buffer primitive;
    glm::mat4 scaleMat = glm::scale(glm::mat4(), glm::vec3(scale, scale, scale));
    lx0::primitive_buffer_from_blendfile(primitive, filename, scaleMat);
            
    auto spGeometry = spRasterizer->createQuadList(primitive.indices, primitive.face.flags, primitive.vertex.positions, primitive.vertex.normals, primitive.vertex.colors);
    spGeometry->mBBox = primitive.bbox;

    return spGeometry;
}

GeometryPtr 
MeshCache::acquire (const char* filename)
{
    mStats.acquireCount++;

    auto it = mCache.find(filename);
    if (it != mCache.end())
    {
        lx_debug("Reusing cache for '%s'", filename);
        return it->second;
    }
    else
    {
        lx_log("Adding mesh '%s' to MeshCache", filename);

        auto spGeom = _createGeometry(mspRasterizer.get(), filename, 1.0f);
        mCache.insert(std::make_pair(filename, spGeom));
        return spGeom;
    }
}

size_t MeshCache::acquireCount (void) const
{
    return mStats.acquireCount;
}

size_t MeshCache::cacheSize (void) const
{
    return mCache.size();
}
