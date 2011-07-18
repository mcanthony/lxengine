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

#include <bullet/btBulletDynamicsCommon.h>

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
    {
    }
    virtual void getWorldTransform(glgeom::point3f& position, glgeom::quatf& orientation)  {}
    virtual void setWorldTransform(const glgeom::point3f& position, const glgeom::quatf& orientation) {}
};

class PlayerElem : public PhysicsElem
{
public:
    PlayerElem (lx0::ElementPtr spElement)
        : mpElement (spElement.get())
    {
        auto spPhysics = Engine::acquire()->getComponent<IPhysicsEngine>();

        // 2 m tall x .5 m wide in x and y, and 68 kgs
        //
        auto position = spElement->value()["position"].unwrap2<glgeom::point3f>();
        auto spShape = spPhysics->acquireBoxShape(glgeom::vector3f(.25f, .25f, 1.0f));
        _initialize(spElement, position, glgeom::quatf(), spShape, 68.0f);

        mspRigidBody->setDamping(0.3f, 0.7f);
        mspRigidBody->setFriction(0.15f);
        mspRigidBody->setRestitution(0.1f);
    }

    virtual void getWorldTransform(glgeom::point3f& position, glgeom::quatf& orientation)
    {
        position = mpElement->value()["position"].unwrap2<glgeom::point3f>();
        orientation = glgeom::quatf(1, 0, 0, 0);
    }

    virtual void setWorldTransform(const glgeom::point3f& position, const glgeom::quatf& orientation)
    {
        mpElement->value()["position"].unwrap2<glgeom::point3f>() = position;
        mpElement->notifyValueChanged();
    }

    lx0::Element*   mpElement;
};

void initializePhysics()
{
    auto spPhysics = Engine::acquire()->getComponent<IPhysicsEngine>();
    spPhysics->addElementComponent<InstanceElem>("Instance"); 
    spPhysics->addElementComponent<PlayerElem>("Player");

    // Why 70? 70 Morrowind units approx. equals 1 meter
    spPhysics->setGravity( glgeom::vector3f(0, 0, 70 * -9.821f) );
}
