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
#include "element_scene.hpp"

using namespace lx0::subsystem::physics_ns::detail;
using namespace lx0;

//===========================================================================//
//
//===========================================================================//

PhysicsDoc::PhysicsDoc()
    : mEnableSimulation (true)
    , mfWindVelocity    (0.0f)
    , mWindDirection    (-1, 0, 0)
{
    Engine::acquire()->incObjectCount("Physics");

    mspBroadphase.reset( new btDbvtBroadphase );

    mspCollisionConfiguration.reset( new btDefaultCollisionConfiguration );
    mspDispatcher.reset( new btCollisionDispatcher(mspCollisionConfiguration.get()) );

    mspSolver.reset( new btSequentialImpulseConstraintSolver );

    mspDynamicsWorld.reset( new btDiscreteDynamicsWorld(mspDispatcher.get(), 
                                                        mspBroadphase.get(), 
                                                        mspSolver.get(), 
                                                        mspCollisionConfiguration.get()) );
    mspDynamicsWorld->setGravity(btVector3(0, 0, 0));


    // Create a global ground plane
    //
    mspGroundShape.reset(new btStaticPlaneShape(btVector3(0,0,1), 0) );
    mspGroundMotionState.reset( new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0, 0, 0))) );

    const float fGroundMass = 0.0f;   // Infinite, immovable object
    const btVector3 groundInertia(0, 0, 0);
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(fGroundMass, mspGroundMotionState.get(), mspGroundShape.get(), groundInertia);
    mspGroundRigidBody.reset( new btRigidBody(groundRigidBodyCI) );
    mspGroundRigidBody->setRestitution(0.1f);
    mspGroundRigidBody->setFriction(0.5f);
    mspDynamicsWorld->addRigidBody(mspGroundRigidBody.get());
}

PhysicsDoc::~PhysicsDoc()
{
    lx_debug("PhysicsDoc destructor");

    // The shared_ptr objects will deallocate everything on destruction, but the rigid
    // body objects need to be removed from the world to ensure the destruction order
    // is handled correctly.
    //
    mspDynamicsWorld->removeRigidBody(mspGroundRigidBody.get());

    // Explicitly release the objects to ensure destruction order is correct
    mspDynamicsWorld.reset();
    mspBroadphase.reset();

    Engine::acquire()->decObjectCount("Physics");
}

void 
PhysicsDoc::setWindDirection (const glgeom::vector3f& v)  
{ 
    mWindDirection = normalize(v); 
}
    
void 
PhysicsDoc::onAttached (DocumentPtr spDocument)
{
    mLastUpdate = lx0::lx_milliseconds();
}
    
void 
PhysicsDoc::onElementAdded (DocumentPtr spDocument, ElementPtr spElem)
{
    // Attach the custom logic to the Element (i.e. ensures the element will
    // be updated as the physics simulation moves it)
    //
    lx_check_error( spElem->getComponent<PhysicsElem>("physics").get() == nullptr );

    const std::string tag = spElem->tagName();
    if (tag == "Ref")
        spElem->attachComponent(new PhysicsElem(spDocument, spElem, this) );
    else if (tag == "Scene")
        spElem->attachComponent(new SceneElem(spDocument, spElem, this) );
    else
    {
        auto spPhysicsEng = Engine::acquire()->getComponent<PhysicsEngine>();
        auto ctor = spPhysicsEng->findElementComponentCtor(tag);
        if (ctor)
            spElem->attachComponent( ctor(spElem) );
    }
}

void 
PhysicsDoc::onElementRemoved (Document* pDocument, ElementPtr spElem)
{
    if (spElem->tagName() == "Ref")
    {
        spElem->removeComponent("physics");
    }
}

void
PhysicsDoc::_applyWind (const float timeStep)
{
    // Early out if there's no wind
    if (mfWindVelocity < 0.001f)
        return;

    const glgeom::vector3f airVecTemp = mfWindVelocity * mWindDirection;
    const btVector3 airVelocity( airVecTemp.x, airVecTemp.y, airVecTemp.z);
    const btScalar  airDensity = 1.29f;            // kg / m^3

    btCollisionObjectArray objects = mspDynamicsWorld->getCollisionObjectArray();
    mspDynamicsWorld->clearForces();
    for (int i = 0; i < objects.size(); i++) 
    {
        btRigidBody* pRigidBody = btRigidBody::upcast(objects[i]);
        if (pRigidBody) 
        {
            // Compute an approximate surface area in each direct by simply taking some
            // percentage of the bounding radius.
            //
            btScalar radius;
            btCollisionShape* pShape = pRigidBody->getCollisionShape();
            pShape->getBoundingSphere(btVector3(0,0,0), radius);
            btScalar approxArea = radius * radius * .66f;
            btVector3 surfaceArea (approxArea, approxArea, approxArea);

            // Velocity * density * surface area = amount of mass per second hitting the area
            // " * time * velocity = momentum of that mass over that period of time
            //
            // ...which is a measure of impulse, which Bullet understands.
            //
            btVector3 impulse = (airVelocity * airDensity * surfaceArea * timeStep) * airVelocity;
            pRigidBody->applyCentralImpulse(impulse);
        }
    }
}

/*
    The physics simulation has iterated a time-step.  The results need to be
    written back to the DOM.
    */
void
PhysicsDoc::_updateElements (DocumentPtr spDocument)
{
    auto allRefs = spDocument->getElementsByTagName("Ref");
    for (auto it = allRefs.begin(); it != allRefs.end(); ++it)
    {
        ElementPtr spElem = *it;
        auto spPhysics = spElem->getComponent<PhysicsElem>("physics");
        lx_check_error(spPhysics.get() != nullptr, "All elements should have an associated physics component!");

        btTransform trans;
        spPhysics->mspRigidBody->getMotionState()->getWorldTransform(trans);
        glgeom::point3f p = glgeom::point3f( trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z() );
        btQuaternion q = trans.getRotation();

        spElem->attr("translation", lxvar(p.x, p.y, p.z) );
        spElem->attr("rotation", lxvar(q.x(), q.y(), q.z(), q.w()) );
    }
}

ElementPtr 
getElementPtrFor (btCollisionObject* pBtObject)
{
    lx_check_error(pBtObject != nullptr,  "Trying to get ElementPtr for a null object!");

    void* pUserPtr = pBtObject->getUserPointer();
    if (pUserPtr)
    {
        Element* pElem = reinterpret_cast<Element*>(pUserPtr);
        return pElem->shared_from_this();
    }
    else
    {
        return ElementPtr();
    }
}

void            
PhysicsDoc::_applyCollisonActions (DocumentPtr spDocument)
{
    int numManifolds = mspDispatcher->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold =  mspDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
		btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());
	
		int numContacts = contactManifold->getNumContacts();
		for (int j = 0; j < numContacts; j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance() < 00.f)
			{
				const btVector3& ptA = pt.getPositionWorldOnA();
				const btVector3& ptB = pt.getPositionWorldOnB();
				    
                ElementPtr spElemA = getElementPtrFor(obA);
                ElementPtr spElemB = getElementPtrFor(obB);

                // Objects like the ground plane are not in the Document.
                if (spElemA.get() && spElemB.get())
                {
                    struct Wrapper : public lx0::core::lxvar_ns::detail::lxvalue
                    {
                        virtual lx0::core::lxvar_ns::detail::lxvalue* clone  (void) const    { auto p = new Wrapper; p->mspValue = mspValue; return p; } 
                        virtual bool        isHandle    (void) const    { return true; }
                        virtual std::string handleType  (void) const    { return "Element"; }
                        virtual void*       unwrap      (void)          { return mspValue.get(); }
                        ElementPtr mspValue;
                    };

                    Wrapper* wrapper = new Wrapper;
                    wrapper->mspValue = spElemB;
                    std::vector<lxvar> args;
                    args.push_back( lxvar(wrapper) );

                    spElemA->call("onCollision", args);
                    wrapper->mspValue = spElemA;
                    spElemB->call("onCollision", args);
                }
            }
		}
	}
}

void 
PhysicsDoc::onUpdate (DocumentPtr spDocument)
{
    //
    // The physics sub-system may be paused or be used as pure collision detection system,
    // in which case, we do not want the simulation tick to run
    //
    if (!mEnableSimulation)
        return;

    // In milliseconds...
    const float kFps = 60.0f;
    const unsigned int kFrameDurationMs = unsigned int( (1.0f / kFps) * 1000.0f );

    auto timeNow = lx0::lx_milliseconds();

    if (timeNow - mLastUpdate >= kFrameDurationMs)
    {
        const int kMaxSubSteps = 10;
        const float kStep = Engine::acquire()->environment().timeScale() * kFrameDurationMs / 1000.0f;

        _applyWind(kStep);

        mspDynamicsWorld->stepSimulation((timeNow - mLastUpdate) / 1000.0f, kMaxSubSteps);

        _updateElements(spDocument);
        _applyCollisonActions(spDocument);

        mLastUpdate = timeNow;
    }
}

void 
PhysicsDoc::enableSimulation (bool bEnable)
{
    mEnableSimulation = bEnable;
}

void    
PhysicsDoc::setGravity (const glgeom::vector3f& gravity)
{
    mspDynamicsWorld->setGravity( btVector3(gravity.x, gravity.y, gravity.z) );
}

void
PhysicsDoc::addToWorld (btRigidBody* pRigidBody)
{
    mspDynamicsWorld->addRigidBody(pRigidBody);
}

void
PhysicsDoc::removeFromWorld (btRigidBody* pRigidBody)
{
    mspDynamicsWorld->removeRigidBody(pRigidBody);
}

btCollisionWorld* 
PhysicsDoc::getWorld (void)
{
    return mspDynamicsWorld.get();
}

