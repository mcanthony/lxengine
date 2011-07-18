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
#include "physics_engine.hpp"
#include "physics_document.hpp"
#include "physics_element.hpp"

using namespace lx0::subsystem::physics_ns::detail;
using namespace lx0;

//===========================================================================//
//
//===========================================================================//

void 
PhysicsEngine::onAttached (EnginePtr spEngine) 
{
    Element::addFunction("addImpulse", [](ElementPtr spElem, std::vector<lxvar>& args) {
        spElem->getComponent<PhysicsElem>("physics")->_addImpulseFunc(spElem, args);
    });
}

void 
PhysicsEngine::onDocumentCreated   (EnginePtr spEngine, DocumentPtr spDocument) 
{
    auto pPhysicsDoc = new PhysicsDoc;
    pPhysicsDoc->setGravity(mGravity);

    spDocument->attachComponent(pPhysicsDoc);
}

btCollisionShapePtr
PhysicsEngine::acquireSphereShape (float radius)
{
    return mSphereShapeCache.acquire(SphereKey(radius));  
}

btCollisionShapePtr      
PhysicsEngine::acquireBoxShape (const glgeom::vector3f& halfBounds)
{
    return mBoxShapeCache.acquire(BoxKey (halfBounds));
}

void 
PhysicsEngine::setGravity (const glgeom::vector3f& gravity)
{
    mGravity = gravity;
}

