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


typedef std::shared_ptr<btCollisionShape>   btCollisionShapePtr;
typedef std::weak_ptr<btCollisionShape>     btCollisionShapeWPtr;

//===========================================================================//
//   I M P L E M E N T A T I O N 
//===========================================================================//

namespace lx0 { namespace core { namespace detail {

    // --------------------------------------------------------------------- //
    //! Base class for the Shape cache keys.
    /*!
        The shape caches are used to ensure that if two objects with the same
        size bounding box (or bounding sphere, for example) will reuse the
        same btCollisionShape object.  

        The ShapeKey is responsible for:

        (1) Generating a key from specific shape parameters: for example, it
            ensures that an object with a bounding sphere of size 1.00001
            and a second sphere of size 1.00002 will share the same collision
            shape - rather than needlessly creating two very similar objects.

        (2) Knowing how to generate a new collision shape when a desired 
            shape does not already exist in the cache.

        (3) Providing a comparison operator so the key can be used with 
            std::map<>.

     */
    struct ShapeKeyBase
    {
        const float granularity() const 
        {
            return 0.05f;
        }
    };

    //! Key used for caching btBoxShape
    /*!
     */
    struct BoxKey : public ShapeKeyBase
    {
        BoxKey (const vector3& halfBounds)
        {
            for (int i = 0; i < 3; ++i)
                xyz[i] = int( ceil(halfBounds[i] / granularity() ) );
        }

        btCollisionShape* createShape() const
        {
            btVector3 bulletVec;
            bulletVec.setX( xyz[0] * granularity() );
            bulletVec.setY( xyz[1] * granularity() );
            bulletVec.setZ( xyz[2] * granularity() );

            return new btBoxShape(bulletVec);
        }

        bool operator< (const BoxKey& that) const
        {
            for (int i = 0; i < 3; ++i)
            {
                int cmp = xyz[0] - that.xyz[0];
                if (cmp < 0)
                    return true;
                else if (cmp > 0)
                    return false;
            }
            return false;
        }
        int xyz[3];
    };

    //! Key used for caching btSphereShape
    /*!
     */
    struct SphereKey : public ShapeKeyBase
    {
        SphereKey (float radius)
        {
            lx_check_error(radius >= 0.0f);

            // Only respect a limited granularity so that similar shapes will simply be
            // reused
            key = int( ceil(radius /  granularity()) );
        }

        bool operator< (const SphereKey& that) const
        {
            int cmp = key - that.key;
            return (cmp < 0) ? true : false;
        }

        btCollisionShape* createShape() const
        {
            const float fCachedRadius = key * granularity();
            return new btSphereShape(fCachedRadius);
        }

        int key;
    };

    //! Wrapper on std::map<> to manage cached btCollisionShapes
    /*!
     */
    template <typename KeyType>
    class ShapeCache
    {
    public:
        typedef         KeyType     Key;

        btCollisionShapePtr acquire (const Key& key)
        {  
            auto it = mCache.find(key);
            if (it == mCache.end())
                it = mCache.insert(std::make_pair(key, btCollisionShapeWPtr() )).first;

            btCollisionShapeWPtr& wpShape = it->second;
            btCollisionShapePtr spShape = wpShape.lock();
            if (!spShape)
            {
                spShape.reset( key.createShape() );
                wpShape = spShape;
            }

            lx_check_error( spShape.get() != nullptr );
            return spShape;
        }

    protected:
        std::map<Key, btCollisionShapeWPtr>     mCache;
    };

    //! The internal physics subsystem
    /*!
     */
    class Physics : public Document::Component
    {
    public:
        Physics()
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
            mspDynamicsWorld->setGravity(btVector3(0, 0, -9.81f));


            // Create a global ground plane
            //
            mspGroundShape.reset(new btStaticPlaneShape(btVector3(0,0,1), 0) );
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

            Engine::acquire()->decObjectCount("Physics");
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

        /*!
            Initializes the Physics subsystem for the document according to the Document's
            current state.

            Dev. note: would it be cleaner to incrementally do this by registering an 
            onAdded() callback?
         */
        void 
        init (DocumentPtr spDocument)
        {
            auto allElems = spDocument->getElementsByTagName("Ref");
            for (auto it = allElems.begin(); it != allElems.end(); ++it)
            {
                // Prepare the relevant variable that will be used
                //
                auto           spElem   = *it;
                std::string    ref      = spElem->queryAttr("ref", "");
                const btScalar kfMass   = spElem->queryAttr("mass", 0.0f);   
                auto pos                = asPoint3( spElem->attr("translation") );

                auto spMeshElem         = spDocument->getElementById(ref);
                MeshPtr spMesh          = spMeshElem->value<Mesh>();

                // Determine the transformation
                //
                btTransform tform (btQuaternion(0,0,0,1), btVector3(pos.x, pos.y, pos.z));
                std::shared_ptr<btDefaultMotionState> spMotionState( new btDefaultMotionState(tform) );
                      
                // Determine and acquire the collision shape to use
                //
                btCollisionShapePtr spShape;
                if (spElem->queryAttr("bounds_type", "box") == "box")
                    spShape = _acquireBoxShape( spMesh->boundingVector() );
                else
                    spShape =  _acquireSphereShape( spMesh->boundingRadius() );

                btVector3 fallInertia(0, 0, 0);
                spShape->calculateLocalInertia(kfMass, fallInertia);
            
                // Create the RigidBody to put into the world
                //
                btRigidBody::btRigidBodyConstructionInfo rigidBodyCI (kfMass, spMotionState.get(), spShape.get(), fallInertia);
                std::shared_ptr<btRigidBody> spRigidBody( new btRigidBody(rigidBodyCI) );
                mspDynamicsWorld->addRigidBody(spRigidBody.get());

                // Attach the custom logic to the Element (i.e. ensures the element will
                // be updated as the physics simulation moves it)
                //
                spElem->attachComponent("physics", new PhysicsComponent(spRigidBody) );

                // Track the created variables so that clean-up can be done correctly
                //
                mMotionStates.push_back( spMotionState );
                mRigidBodies.push_back( spRigidBody);
                mShapes.push_back( spShape );
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
        btCollisionShapePtr         _acquireSphereShape         (float radius);
        btCollisionShapePtr         _acquireBoxShape            (const vector3& halfBounds);

        std::shared_ptr<btBroadphaseInterface>                  mspBroadphase;
        std::shared_ptr<btDefaultCollisionConfiguration>        mspCollisionConfiguration;
        std::shared_ptr<btCollisionDispatcher>                  mspDispatcher;
        std::shared_ptr<btSequentialImpulseConstraintSolver>    mspSolver;
        std::shared_ptr<btDiscreteDynamicsWorld>                mspDynamicsWorld;

        std::shared_ptr<btCollisionShape>                       mspGroundShape;

        ShapeCache<SphereKey>                                   mSphereShapeCache;
        ShapeCache<BoxKey>                                      mBoxShapeCache;

        std::shared_ptr<btDefaultMotionState>                   mspGroundMotionState;
        std::shared_ptr<btRigidBody>                            mspGroundRigidBody;

        std::vector< std::shared_ptr<btDefaultMotionState> >    mMotionStates;
        std::vector< std::shared_ptr<btRigidBody> >             mRigidBodies;
        std::vector< btCollisionShapePtr >                      mShapes;

        unsigned int                                            mLastUpdate;
    };

    btCollisionShapePtr
    Physics::_acquireSphereShape (float radius)
    {
        return mSphereShapeCache.acquire(SphereKey(radius));  
    }

    btCollisionShapePtr      
    Physics::_acquireBoxShape (const vector3& halfBounds)
    {
        return mBoxShapeCache.acquire(BoxKey (halfBounds));
    }
}}}

namespace lx0 { namespace core {
    
    using namespace detail;

    void
    Engine::_attachPhysics (DocumentPtr spDocument)
    {
        Physics* pPhysics = new Physics;
        spDocument->attachComponent("physicsSystem", pPhysics);
        pPhysics->init(spDocument);

        //
        // Ensure the lambda function copies a raw pointer not the shared pointer.
        // Two notes about this:
        // (1) The lamdba function local variable copies essentially have global 
        //     scope, which means the shared_ptr copy would not be released until
        //     shutdown.  That copied shared_ptr would make it impossible to free 
        //     the Document memory during the session.
        // (2) The Document lifetime is never less than slotUpdateRun, so there's
        //     no need for a reference counted pointer in this specific case.
        //
        Document* pDocument = spDocument.get();
        spDocument->slotUpdateRun += [=] () { pPhysics->update(pDocument->shared_from_this()); };
    }

}}
