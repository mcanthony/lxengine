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
//   H E A D E R S
//===========================================================================//

// Standard headers
#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

// Bullet
#include <bullet/btBulletDynamicsCommon.h>

// LxEngine
#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/mesh.hpp>
#include <lx0/util.hpp>
#include <lx0/cast.hpp>
#include <lx0/lxvar_convert.hpp>

_ENABLE_LX_CAST(btVector3, point3);

using namespace lx0::core;

typedef std::shared_ptr<btCollisionShape>   btCollisionShapePtr;
typedef std::weak_ptr<btCollisionShape>     btCollisionShapeWPtr;

//===========================================================================//
//   I M P L E M E N T A T I O N 
//===========================================================================//

namespace lx0 { namespace core { namespace detail {

    //-----------------------------------------------------------------------//
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

    //-----------------------------------------------------------------------//
    //! Key used for caching btBoxShape
    /*!
        Determines (a) which cache element to use for a given bounding box, (b) how to 
        create new a cache element for the bounds if one does not exist.
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

    //-----------------------------------------------------------------------//
    //! Key used for caching btSphereShape
    /*!
        Determines (a) which cache element to use for a given bounding sphere, (b) how to 
        create new a cache element for the bounds if one does not exist.
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

    //-----------------------------------------------------------------------//
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

    //-----------------------------------------------------------------------//
    //! The internal physics subsystem
    /*!
     */
    class PhysicsDoc : public Document::Component
    {
    public:
                        PhysicsDoc();
                        ~PhysicsDoc();

        virtual void    onAttached          (DocumentPtr spDocument);

        virtual void    onElementAdded      (DocumentPtr spDocument, ElementPtr spElem);
        virtual void    onElementRemoved    (Document* pDocument, ElementPtr spElem);

        virtual void    onUpdate            (DocumentPtr spDocument);

        btCollisionShapePtr         _acquireSphereShape         (float radius);
        btCollisionShapePtr         _acquireBoxShape            (const vector3& halfBounds);

        void            setWindVelocity     (float v)           { mfWindVelocity = v; }
        void            setWindDirection    (const vector3& v);

        std::shared_ptr<btDiscreteDynamicsWorld>                mspDynamicsWorld;

    protected:
        void            _applyWind      (const float timeStep);

        float           mfWindVelocity;
        vector3         mWindDirection;

        std::shared_ptr<btBroadphaseInterface>                  mspBroadphase;
        std::shared_ptr<btDefaultCollisionConfiguration>        mspCollisionConfiguration;
        std::shared_ptr<btCollisionDispatcher>                  mspDispatcher;
        std::shared_ptr<btSequentialImpulseConstraintSolver>    mspSolver;

        std::shared_ptr<btCollisionShape>                       mspGroundShape;

        ShapeCache<SphereKey>                                   mSphereShapeCache;
        ShapeCache<BoxKey>                                      mBoxShapeCache;

        std::shared_ptr<btDefaultMotionState>                   mspGroundMotionState;
        std::shared_ptr<btRigidBody>                            mspGroundRigidBody;

        unsigned int                                            mLastUpdate;
    };

    class SceneElem : public Element::Component
    {
    public:
        SceneElem (DocumentPtr spDocument, ElementPtr spElem, PhysicsDoc* pPhysics)
            : mpDocPhysics (pPhysics)
        {
            lx_check_error( spElem->getComponent<SceneElem>("physics").get() == nullptr );
            _reset(spElem);
        }

        virtual void onAttributeChange(ElementPtr spElem, std::string name, lxvar value)
        {
            _reset(spElem);
        }

        void _reset (ElementPtr spElem)
        {
            auto windVelocity = spElem->attr("wind_velocity");
            auto windDirection = spElem->attr("wind_direction");
            auto gravity = spElem->attr("gravity");

            if (windVelocity.isDefined())
            {
                float velocity = windVelocity.query(0.0f);
                mpDocPhysics->setWindVelocity(velocity);
            }
            if (windDirection.isDefined())
            {
                lx_check_error(windDirection.isArray());

                vector3 dir(windDirection.at(0).query(0.0f), windDirection.at(1).query(0.0f), windDirection.at(2).query(0.0f) );
                mpDocPhysics->setWindDirection(dir);
            }
            if (gravity.isDefined())
            {
                vector3 val = gravity.convert();
                mpDocPhysics->mspDynamicsWorld->setGravity(btVector3(val.x, val.y, val.z));
            }
        }

    protected:
        PhysicsDoc*     mpDocPhysics;
    };

    class PhysicsElem : public Element::Component
    {
    public:
        PhysicsElem (DocumentPtr spDocument, ElementPtr spElem, PhysicsDoc* pPhysics)
            : mpDocPhysics (pPhysics)
        {
            lx_check_error( spElem->getComponent<PhysicsElem>("physics").get() == nullptr );

            // Prepare the relevant variables that will be used
            //
            std::string    ref      = spElem->attr("ref").query("");
            const btScalar kfMass   = spElem->attr("mass").query(0.0f);   
            auto posAttr            = spElem->attr("translation");
            point3 pos              = posAttr.isDefined() ? point3(posAttr.convert()) : point3(0, 0, 0);
            lxvar maxExtent         = spElem->attr("max_extent");
            auto    velAttr         = spElem->attr("velocity");
            vector3 vel             = velAttr.isDefined() ? vector3(velAttr.convert()) : vector3(0, 0, 0);

            auto spMeshElem         = spDocument->getElementById(ref);
            MeshPtr spMesh          = spMeshElem->value().imp<Mesh>();

            float scale             = spMesh->maxExtentScale(maxExtent);

            // Determine the transformation
            //
            btTransform tform (btQuaternion(0,0,0,1), btVector3(pos.x, pos.y, pos.z));
            mspMotionState.reset( new btDefaultMotionState(tform) );
                      
            // Determine and acquire the collision shape to use
            //
            if (spElem->attr("bounds_type").query("box") == "box")
                mspShape = pPhysics->_acquireBoxShape( spMesh->boundingVector() * scale );
            else
                mspShape =  pPhysics->_acquireSphereShape( spMesh->boundingRadius() * scale );

            btVector3 fallInertia(0, 0, 0);
            mspShape->calculateLocalInertia(kfMass, fallInertia);
            
            // Create the RigidBody to put into the world
            //
            btRigidBody::btRigidBodyConstructionInfo rigidBodyCI (kfMass, mspMotionState.get(), mspShape.get(), fallInertia);
            mspRigidBody.reset( new btRigidBody(rigidBodyCI) );
            mspRigidBody->setRestitution( spElem->attr("restitution").query(0.1f) );
            mspRigidBody->setFriction( spElem->attr("friction").query(0.5f) );
            mspRigidBody->setLinearVelocity( btVector3(vel.x, vel.y, vel.z) );
            _addToWorld();
        }

        ~PhysicsElem()
        {
            _removeFromWorld();
        }

        virtual void onAttributeChange(ElementPtr spElem, std::string name, lxvar value)
        {
            if (name == "display")
            {
                if (value.equal("block"))
                    _addToWorld();
                else if (value.equal("none"))
                    _removeFromWorld();
                else
                    lx_error("Unexpected value for display attribute");
            }
            else if (name == "restitution")
                mspRigidBody->setRestitution( value.query(0.1f) );
            else if (name == "friction")
                mspRigidBody->setFriction( value.query(0.5f) );
        }

        void _addToWorld()
        {
            mpDocPhysics->mspDynamicsWorld->addRigidBody(mspRigidBody.get());
        }
        void _removeFromWorld()
        {
            mpDocPhysics->mspDynamicsWorld->removeRigidBody(mspRigidBody.get());
        }
        
        PhysicsDoc*                             mpDocPhysics;
        std::unique_ptr<btDefaultMotionState>   mspMotionState;
        std::unique_ptr<btRigidBody>            mspRigidBody;
        std::shared_ptr<btCollisionShape>       mspShape;
    };

    PhysicsDoc::PhysicsDoc()
        : mfWindVelocity    (0.0f)
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
        const btVector3 groundIntertia(0, 0, 0);
        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(fGroundMass, mspGroundMotionState.get(), mspGroundShape.get(), groundIntertia);
        mspGroundRigidBody.reset( new btRigidBody(groundRigidBodyCI) );
        mspGroundRigidBody->setRestitution(0.1f);
        mspGroundRigidBody->setFriction(0.5f);
        mspDynamicsWorld->addRigidBody(mspGroundRigidBody.get());
    }

    PhysicsDoc::~PhysicsDoc()
    {
        // The shared_ptr objects will deallocate everything on destruction, but the rigid
        // body objects need to be removed from the world to ensure the destruction order
        // is handled correctly.
        //
        mspDynamicsWorld->removeRigidBody(mspGroundRigidBody.get());

        Engine::acquire()->decObjectCount("Physics");
    }

    void 
    PhysicsDoc::setWindDirection (const vector3& v)  
    { 
        mWindDirection = normalize(v); 
    }
    
    void 
    PhysicsDoc::onAttached (DocumentPtr spDocument)
    {
        mLastUpdate = lx0::util::lx_milliseconds();
    }


    void PhysicsDoc::onElementAdded(DocumentPtr spDocument, ElementPtr spElem)
    {
        if (spElem->tagName() == "Ref")
        {
            // Attach the custom logic to the Element (i.e. ensures the element will
            // be updated as the physics simulation moves it)
            //
            lx_check_error( spElem->getComponent<PhysicsElem>("physics").get() == nullptr );

            spElem->attachComponent("physics", new PhysicsElem(spDocument, spElem, this) );
        }
        else if (spElem->tagName() == "Scene")
            spElem->attachComponent("physics", new SceneElem(spDocument, spElem, this) );
    }

    void PhysicsDoc::onElementRemoved(Document* pDocument, ElementPtr spElem)
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

        const vector3 airVecTemp = mfWindVelocity * mWindDirection;
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
                btVector3 impulse = (airVelocity * airDensity * surfaceArea * timeStep) * airVelocity;
                pRigidBody->applyCentralImpulse(impulse);
            }
        }
    }

    void 
    PhysicsDoc::onUpdate (DocumentPtr spDocument)
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

            _applyWind(kStep);
 
            auto allRefs = spDocument->getElementsByTagName("Ref");
            for (auto it = allRefs.begin(); it != allRefs.end(); ++it)
            {
                ElementPtr spElem = *it;
                auto spPhysics = spElem->getComponent<PhysicsElem>("physics");
                lx_check_error(spPhysics.get() != nullptr, "All elements should have an associated physics component!");

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

    btCollisionShapePtr
    PhysicsDoc::_acquireSphereShape (float radius)
    {
        return mSphereShapeCache.acquire(SphereKey(radius));  
    }

    btCollisionShapePtr      
    PhysicsDoc::_acquireBoxShape (const vector3& halfBounds)
    {
        return mBoxShapeCache.acquire(BoxKey (halfBounds));
    }

}}}

namespace lx0 { namespace core {
    
    using namespace detail;

    void
    Engine::_attachPhysics (DocumentPtr spDocument)
    {
        spDocument->attachComponent("physicsSystem", new PhysicsDoc);
    }

}}
