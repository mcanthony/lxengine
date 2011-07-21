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

#include <iostream>
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/physics.hpp>
#include <lx0/util/misc.hpp>
#include <glgeom/ext/primitive_buffer.hpp>

#include <bullet/btBulletDynamicsCommon.h>

#include "mwphysics.hpp"

using namespace lx0;

class PhysicsElem : public lx0::Element::Component
{
public:
    PhysicsElem()
        : mpPhysicsDoc (nullptr)
    {
    }

    ~PhysicsElem()
    {
        _cleanup();
    }


protected:
    virtual void getWorldTransform(glgeom::point3f& position, glgeom::quatf& orientation) = 0;
    virtual void setWorldTransform(const glgeom::point3f& position, const glgeom::quatf& orientation) = 0;

    struct MotionState : public btMotionState
    {
        MotionState(PhysicsElem* pObject)
            : mpObject (pObject)
        {
        }

        virtual void getWorldTransform (btTransform &centerOfMassWorldTrans) const
        {
            glgeom::quatf   orientation;
            glgeom::point3f position;
            mpObject->getWorldTransform(position, orientation);

            btQuaternion q( orientation.x, orientation.y, orientation.z, orientation.w );
            btVector3    p( position.x, position.y, position.z );           
            centerOfMassWorldTrans = btTransform(q,p);
        }
        virtual void setWorldTransform (const btTransform &centerOfMassWorldTrans)
        {
            btQuaternion q = centerOfMassWorldTrans.getRotation();
            btVector3    p = centerOfMassWorldTrans.getOrigin();

            glgeom::quatf   orientation(q.w(), q.x(), q.y(), q.z());
            glgeom::point3f position(p.x(), p.y(), p.z());
            mpObject->setWorldTransform(position, orientation);
        }

        PhysicsElem* mpObject;
    };




    void _initialize(lx0::ElementPtr spElement,
                     glgeom::point3f position, 
                     glgeom::quatf   orientation,
                     std::shared_ptr<btCollisionShape> spShape,
                     float           mass)
    {
        //
        // Store a pointer to the document this element belongs to
        //
        {
            mpPhysicsDoc = spElement->document()->getComponent<IPhysicsDoc>().get();
        }

        //
        // Create the motion state
        //
        {
            mspMotionState.reset( new MotionState(this) );
        }

        //
        // Set the shape
        //
        {
            mspShape = spShape;
        }

        //
        // Connect them all into a rigid body
        //
        {
            btVector3 fallInertia(0, 0, 0);
            if (mass > 0.0f)
                mspShape->calculateLocalInertia(mass, fallInertia);

            btRigidBody::btRigidBodyConstructionInfo rigidBodyCI (mass, mspMotionState.get(), mspShape.get(), fallInertia);
            mspRigidBody.reset( new btRigidBody(rigidBodyCI) );

            mspRigidBody->setUserPointer( spElement.get() );
        }

        //
        // Add it to the world
        //
        {
            mpPhysicsDoc->addToWorld(mspRigidBody.get());
        }
    }

    void _cleanup()
    {
        if (mspRigidBody)
            mpPhysicsDoc->removeFromWorld(mspRigidBody.get());

        mspRigidBody.reset();
        mspShape.reset();
        mspMotionState.reset();
        mpPhysicsDoc = nullptr;
    }

    IPhysicsDoc*                        mpPhysicsDoc;
    std::unique_ptr<btMotionState>      mspMotionState;
    std::unique_ptr<btRigidBody>        mspRigidBody;
    std::shared_ptr<btCollisionShape>   mspShape;
};

class InstanceElem : public PhysicsElem
{
public:
    InstanceElem (lx0::ElementPtr spElement)
        : mpElement (spElement.get())
    {
        auto& primitive = spElement->value()["primitive"].unwrap<glgeom::primitive_buffer>();

        glgeom::point3f position;
        glgeom::quatf   orientation;
        getWorldTransform(position, orientation);

        //
        // Create a bullet mesh
        //
		btTriangleIndexVertexArray* meshInterface = new btTriangleIndexVertexArray;
		btIndexedMesh part;
		part.m_vertexBase = (const unsigned char*)&primitive.vertex.positions[0];
		part.m_vertexStride = sizeof(primitive.vertex.positions[0]);
		part.m_numVertices = primitive.vertex.positions.size();
		part.m_triangleIndexBase = (const unsigned char*)&primitive.indices[0];
		part.m_triangleIndexStride = sizeof(primitive.indices[0]) * 3;
		part.m_numTriangles = primitive.indices.size() / 3;
		part.m_indexType = PHY_SHORT;
		meshInterface->addIndexedMesh(part, PHY_SHORT);

		bool	useQuantizedAabbCompression = true;
		std::shared_ptr<btBvhTriangleMeshShape> spShape (new btBvhTriangleMeshShape(meshInterface, useQuantizedAabbCompression));

        _initialize(spElement, position, orientation, spShape, 0.0f);
    }

    virtual void getWorldTransform(glgeom::point3f& position, glgeom::quatf& orientation)
    {
        //
        // This is quite inefficient: bullet wants a btTransform, which can be constructed
        // directly from an glm::mat4 - *but* the wrapper layer currently forces us to 
        // convert to position + orientation form.  Fix this layer.
        //
        auto& transform = mpElement->value()["transform"].unwrap<glgeom::mat4f>();

        btTransform t;
        t.setFromOpenGLMatrix(glm::value_ptr(transform));
        auto origin = t.getOrigin();
        auto orient = t.getRotation();

        position = glgeom::point3f(origin.x(), origin.y(), origin.z());
        orientation = glgeom::quatf(orient.w(), orient.x(), orient.y(), orient.z());
    }

    virtual void setWorldTransform(const glgeom::point3f& position, const glgeom::quatf& orientation) 
    {
        assert(0);
    }
    Element*    mpElement;
};

MwPhysicsDoc::MwPhysicsDoc()
    : mbEnableGravity (false)
    , mLastUpdate (0)
{
}

void    
MwPhysicsDoc::enableGravity (bool bEnable)
{
    mbEnableGravity = bEnable;
}

void    
MwPhysicsDoc::onUpdate (DocumentPtr spDocument)
{
   lx0::uint32 now = lx0::lx_milliseconds();

   if (now - mLastUpdate > 16)
   {
       mLastUpdate = now;

       if (mbEnableGravity)
       {
           auto  spPlayer = spDocument->getElementsByTagName("Player")[0];
           auto& velocity = spPlayer->value()["velocity"].unwrap2<glgeom::vector3f>();

           velocity.z += 20 * (70 * -9.821) * 0.016;

           if (!movePlayer(spPlayer, velocity * 0.016))
               velocity.z = 0;
       }
   }
}

static
bool 
_tryMove (IPhysicsEnginePtr spPhysics, IPhysicsDocPtr spPhysicsDoc, const glgeom::point3f& position, const glgeom::vector3f& dir)
{
    btVector3 from (position.x, position.y, position.z);
    btVector3 to   (position.x + dir.x, position.y + dir.y, position.z + dir.z);

    btTransform start (btQuaternion(), from);
    btTransform end   (btQuaternion(), to);
    
    auto spShape = spPhysics->acquireCapsuleShape(.025f * 70.0f, .20f * 70.0f);
    auto pWorld = spPhysicsDoc->getWorld();

    btCollisionWorld::ClosestConvexResultCallback result(from, to); 
    pWorld->convexSweepTest((btConvexShape*)spShape.get(), start, end, result);

    return !result.hasHit();
}

static
void
_setOnGround (IPhysicsEnginePtr spPhysics, IPhysicsDocPtr spPhysicsDoc, glgeom::point3f& position, glgeom::point3f& target)
{
    btVector3 from (position.x, position.y, position.z);
    btVector3 to   (position.x, position.y, position.z - 8 * 70.0f);

    btTransform start (btQuaternion(), from);
    btTransform end   (btQuaternion(), to);
    
    auto pWorld = spPhysicsDoc->getWorld();

    btCollisionWorld::ClosestRayResultCallback result(from, to); 
    pWorld->rayTest(from, to, result);

    if (result.hasHit())
    {
        float deltaZ = position.z - result.m_hitPointWorld.z();
        if (deltaZ > 0.0f && deltaZ < 155.0f)
        {
            position.z += 155.0f - deltaZ;
            target.z += 155.0f - deltaZ;
        }
    }
}

/*
    Could change this in the future to segment the test into two parts:
    - The upper core body, which if it collides, then do not move
    - The legs/feet, which if they collide, try move up some small distance
      and retest; if that succeeds, then move to that higher position - thus
      simulating the ability to step up on objects
 */
bool 
MwPhysicsDoc::movePlayer (lx0::ElementPtr spPlayer, const glgeom::vector3f& dir)
{
    auto spPhysics = Engine::acquire()->getComponent<IPhysicsEngine>();
    auto spPhysicsDoc = spPlayer->document()->getComponent<IPhysicsDoc>();

    auto& position = spPlayer->value()["position"].unwrap2<glgeom::point3f>();
    auto& target   = spPlayer->value()["target"].unwrap2<glgeom::point3f>();

    const glgeom::vector3f stepHeight (0.0f, 0.0f, 10.0f);

    if (_tryMove(spPhysics, spPhysicsDoc, position, dir))
    {
        position += dir;
        target += dir;
        _setOnGround(spPhysics, spPhysicsDoc, position, target);
        spPlayer->notifyValueChanged();
        return true;
    }
    else if (_tryMove(spPhysics, spPhysicsDoc, position + stepHeight, dir))
    {
        position += stepHeight + dir;
        target += stepHeight + dir;
        _setOnGround(spPhysics, spPhysicsDoc, position, target);
        spPlayer->notifyValueChanged();
        return true;
    }
    else
        return false;
}

void initializePhysics()
{
    auto spPhysics = Engine::acquire()->getComponent<IPhysicsEngine>();
    spPhysics->enableSimulation(false);
    spPhysics->addElementComponent<InstanceElem>("Instance"); 

    // Why 70? 70 Morrowind units approx. equals 1 meter
    spPhysics->setGravity( glgeom::vector3f(0, 0, 70 * -9.821f) );
}
