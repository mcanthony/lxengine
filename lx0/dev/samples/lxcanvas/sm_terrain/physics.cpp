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

#include <lx0/lxengine.hpp>
#include "physics.hpp"
#include "terrain.hpp"

using namespace lx0;

void PhysicsSubsystem::onElementAdded (DocumentPtr spDocument, ElementPtr spElem) 
{
    if (spElem->tagName() == "Terrain" || spElem->tagName() == "Sprite")
    {
        mElems.insert(std::make_pair(spElem.get(), ElemData(spElem)));
    }
}

void PhysicsSubsystem::onElementRemoved (Document*   pDocument, ElementPtr spElem) 
{
    auto it = mElems.find(spElem.get());
    if (it != mElems.end())
        mElems.erase(it);
}
    
float PhysicsSubsystem::drop (float x, float y)
{
    lx0::uint64 start = lx0::lx_milliseconds();
            
    float maxZ = std::numeric_limits<float>::min();
    for (auto it = mElems.begin(); it != mElems.end(); ++it)
    {
        if (it->second.spElem->tagName() == "Terrain")
        {
            auto spTerrain = it->second.spElem->getComponent<Terrain::Runtime>("runtime");
            maxZ = std::max(maxZ, spTerrain->calcHeight(x, y));
        }
    }

    lx0::uint64 end = lx0::lx_milliseconds();
    Engine::acquire()->incPerformanceCounter("PhysicsSubsystem drop()", end - start);

    return maxZ; 
}

void PhysicsSubsystem::onUpdate (DocumentPtr spDocument)
{
    const float terrainHeight = drop(gCamera.mPosition.x, gCamera.mPosition.y);
    const float deltaZ = (terrainHeight + 32.0f) - gCamera.mPosition.z;
    gCamera.mPosition.z += deltaZ;
    gCamera.mTarget.z += deltaZ;

    int processed = 0;
    int skipped = 0;

    for (auto it = mElems.begin(); it != mElems.end(); ++it)
    {
        auto spElement = it->second.spElem;
        if (spElement->tagName() == "Sprite")
        {
            lxvar attrPos = spElement->attr("position");
            const float z = attrPos.size() > 2 ? attrPos.at(2).asFloat() : 0.0f;
            glgeom::point3f pos( attrPos.at(0).asFloat(), attrPos.at(1).asFloat(), z);
            if (pos != it->second.lastPosition || true)
            {
                pos.z = drop(pos.x, pos.y);
                spElement->attr("position", lxvar(pos.x, pos.y, pos.z));
                it->second.lastPosition = pos;

                processed++;
            }
            else
                skipped++;
        }
    }

    spDocument->view(0)->sendEvent("redraw", lxvar::undefined());
}
