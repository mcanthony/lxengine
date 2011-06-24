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

#include "lx0/subsystem/rasterizer.hpp"
#include <lx0/util/blendload.hpp>


using namespace lx0::subsystem::rasterizer_ns;

MeshCache::MeshCache(std::shared_ptr<RasterizerGL> spRasterizer)
    : mspRasterizer (spRasterizer)
{
}

GeometryPtr MeshCache::acquire (const char* filename)
{
    auto it = mMeshes.find(filename);
    if (it != mMeshes.end())
    {
        lx_debug("Reusing cache for '%s'", filename);
        return it->second;
    }
    else
    {
        auto spGeom = lx0::quadlist_from_blendfile(*mspRasterizer.get(), filename);
        mMeshes.insert(std::make_pair(filename, spGeom));
        return spGeom;
    }
}
