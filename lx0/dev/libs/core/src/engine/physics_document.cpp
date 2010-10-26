//===========================================================================//
/*
                                   LxEngine

    LICENSE

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
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

// Standard headers
#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

// Include necessary Bullet headers for this tutorial.
#include <bullet/btBulletDynamicsCommon.h>

#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/mesh.hpp>
#include <lx0/util.hpp>
#include <lx0/cast.hpp>

_ENABLE_LX_CAST(btVector3, point3);

using namespace lx0::core;


//===========================================================================//
//   I M P L E M E N T A T I O N 
//===========================================================================//

class Physics : public Document::Component
{
public:
    Physics()
    {
        mspBroadphase.reset( new btDbvtBroadphase );

        mspCollisionConfiguration.reset( new btDefaultCollisionConfiguration );
        mspDispatcher.reset( new btCollisionDispatcher(mspCollisionConfiguration.get()) );

        mspSolver.reset( new btSequentialImpulseConstraintSolver );

        mspDynamicsWorld.reset( new btDiscreteDynamicsWorld(mspDispatcher.get(), 
                                                            mspBroadphase.get(), 
                                                            mspSolver.get(), 
                                                            mspCollisionConfiguration.get()) );
        mspDynamicsWorld->setGravity(btVector3(0, 0, -9.81f));


        // Create collison shapes - which are reusable between various objects
        //
        mspGroundShape.reset(new btStaticPlaneShape(btVector3(0,0,1), 0) );
        mspSphereShape.reset( new btSphereShape(0.5f) );
        mspCubeShape.reset( new btBoxShape(btVector3(0.5f, 0.5f, 0.5f)) );

        mspGroundMotionState.reset( new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0, 0, 0))) );

        const float fGroundMass = 0.0f;   // Infinite, immovable object
        const btVector3 groundIntertia(0, 0, 0);
        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(fGroundMass, mspGroundMotionState.get(), mspGroundShape.get(), groundIntertia);
        mspGroundRigidBody.reset( new btRigidBody(groundRigidBodyCI) );
        mspDynamicsWorld->addRigidBody(mspGroundRigidBody.get());
    }

    ~Physics()
    {
        // The shared_ptr objects will deallocate everything on destruction, but the rigid
        // body objects need to be removed from the world to ensure the destruction order
        // is handled correctly.
        //
        mspDynamicsWorld->removeRigidBody(mspGroundRigidBody.get());

        for (auto it = mRigidBodies.begin(); it != mRigidBodies.end(); ++it)
            mspDynamicsWorld->removeRigidBody( it->get() );
    }

    class PhysicsComponent : public Element::Component
    {
    public:
        PhysicsComponent (std::shared_ptr<btRigidBody> spRigidBody)
            : mspRigidBody (spRigidBody)
        {
        }
        
        std::shared_ptr<btRigidBody> mspRigidBody;
    };

    void 
    init (DocumentPtr spDocument)
    {
        auto allElems = spDocument->getElementsByTagName("Ref");
        for (auto it = allElems.begin(); it != allElems.end(); ++it)
        {
            auto spElem = *it;

            auto pos = asPoint3( spElem->attr("translation") );
            btTransform tform (btQuaternion(0,0,0,1), btVector3(pos.x, pos.y, pos.z));
            std::shared_ptr<btDefaultMotionState> spMotionState( new btDefaultMotionState(tform) );
                
            const btScalar kfMass = spElem->queryAttr("mass", 0.0f);    
            btVector3 fallInertia(0, 0, 0);
            mspCubeShape->calculateLocalInertia(kfMass, fallInertia);
            btRigidBody::btRigidBodyConstructionInfo rigidBodyCI (kfMass, spMotionState.get(), mspCubeShape.get(), fallInertia);

            std::shared_ptr<btRigidBody> spRigidBody( new btRigidBody(rigidBodyCI) );
            mspDynamicsWorld->addRigidBody(spRigidBody.get());

            mMotionStates.push_back( spMotionState );
            mRigidBodies.push_back( spRigidBody);

            spElem->attachComponent("physics", new PhysicsComponent(spRigidBody) );
        }

        mLastUpdate = lx0::util::lx_milliseconds();
    }

    void 
    update (DocumentPtr spDocument)
    {
        // In milliseconds...
        const float kFps = 60.0f;
        const unsigned int kFrameDurationMs = unsigned int( (1.0f / kFps) * 1000.0f );

        auto timeNow = lx0::util::lx_milliseconds();

        if (timeNow - mLastUpdate >= kFrameDurationMs)
        {
            const int kMaxSubSteps = 10;
            const float kStep = Engine::acquire()->environment().timeScale() * kFrameDurationMs / 1000.0f;
            mspDynamicsWorld->stepSimulation(kStep, kMaxSubSteps);
 
            auto allRefs = spDocument->getElementsByTagName("Ref");
            for (auto it = allRefs.begin(); it != allRefs.end(); ++it)
            {
                ElementPtr spElem = *it;
                auto spPhysics = spElem->getComponent<PhysicsComponent>("physics");

                btTransform trans;
                spPhysics->mspRigidBody->getMotionState()->getWorldTransform(trans);
                point3 p = lx_cast( trans.getOrigin() );
                btQuaternion q = trans.getRotation();

                spElem->attr("translation", lxvar(p.x, p.y, p.z) );
                spElem->attr("rotation", lxvar(q.x(), q.y(), q.z(), q.w()) );
            }

            mLastUpdate = timeNow;
        }
    }

protected:
    std::shared_ptr<btBroadphaseInterface>                  mspBroadphase;
    std::shared_ptr<btDefaultCollisionConfiguration>        mspCollisionConfiguration;
    std::shared_ptr<btCollisionDispatcher>                  mspDispatcher;
    std::shared_ptr<btSequentialImpulseConstraintSolver>    mspSolver;
    std::shared_ptr<btDiscreteDynamicsWorld>                mspDynamicsWorld;

    std::shared_ptr<btCollisionShape>                       mspGroundShape;
    std::shared_ptr<btCollisionShape>                       mspSphereShape;
    std::shared_ptr<btCollisionShape>                       mspCubeShape;

    std::shared_ptr<btDefaultMotionState>                   mspGroundMotionState;
    std::shared_ptr<btRigidBody>                            mspGroundRigidBody;

    std::vector< std::shared_ptr<btDefaultMotionState> >    mMotionStates;
    std::vector< std::shared_ptr<btRigidBody> >             mRigidBodies;

    unsigned int                                            mLastUpdate;
};

namespace lx0 { namespace core {

    void
    Engine::_attachPhysics (DocumentPtr spDocument)
    {
        Physics* pPhysics = new Physics;
        spDocument->attachComponent("physicsSystem", pPhysics);
        pPhysics->init(spDocument);
        spDocument->slotUpdateRun += [=] () { pPhysics->update(spDocument); };
    }

}}
