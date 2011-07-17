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
#include <lx0/engine/mesh.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>

#include "physics_element.hpp"
#include "physics_document.hpp"

using namespace lx0::subsystem::physics_ns::detail;
using namespace lx0;

//===========================================================================//
//
//===========================================================================//

PhysicsElem::AttributeTable PhysicsElem::s_attrHandlers;

const char* PhysicsElem::name() const { return "physics"; }

PhysicsElem::PhysicsElem (DocumentPtr spDocument, ElementPtr spElem, PhysicsDoc* pPhysics)
    : mpDocPhysics (pPhysics)
{
    lx_check_error( spElem->getComponent<PhysicsElem>("physics").get() == nullptr );

    //
    // Register all attribute handlers on the first call
    //
    if (s_attrHandlers.empty())
    {
        s_attrHandlers["velocity"] = &PhysicsElem::_setVelocity;
        s_attrHandlers["friction"] = &PhysicsElem::_setFriction;
        s_attrHandlers["restitution"] = &PhysicsElem::_setRestitution;
        s_attrHandlers["linear_damping"] = &PhysicsElem::_setLinearDamping;
        s_attrHandlers["angular_damping"] = &PhysicsElem::_setAngularDamping;
    }

    // Prepare the relevant variables that will be used
    //
    std::string    ref      = query(spElem->attr("ref"), "");
    const btScalar kfMass   = query(spElem->attr("mass"), 0.0f);   
    auto posAttr            = spElem->attr("translation");
    glgeom::point3f pos     = posAttr.is_defined() ? glgeom::point3f(posAttr.convert()) : glgeom::point3f(0, 0, 0);
    lxvar maxExtent         = spElem->attr("max_extent");


    auto spMeshElem         = spDocument->getElementById(ref);
    MeshPtr spMesh          = spMeshElem->value().imp<Mesh>();

    float scale             = spMesh->maxExtentScale(maxExtent);

    // Determine the transformation
    //
    btTransform tform (btQuaternion(0,0,0,1), btVector3(pos.x, pos.y, pos.z));
    mspMotionState.reset( new btDefaultMotionState(tform) );
                      
    // Determine and acquire the collision shape to use
    //
    if (query(spElem->attr("bounds_type"), "box") == "box")
        mspShape = pPhysics->_acquireBoxShape( spMesh->boundingVector() * scale );
    else
        mspShape =  pPhysics->_acquireSphereShape( spMesh->boundingRadius() * scale );

    btVector3 fallInertia(0, 0, 0);
    mspShape->calculateLocalInertia(kfMass, fallInertia);
            
    // Create the RigidBody to put into the world
    //
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI (kfMass, mspMotionState.get(), mspShape.get(), fallInertia);
    mspRigidBody.reset( new btRigidBody(rigidBodyCI) );
    mspRigidBody->setUserPointer( spElem.get() );

    // Set all attributes
    //
    for (auto it = s_attrHandlers.begin(); it != s_attrHandlers.end(); ++it)
    {
        lxvar value = spElem->attr(it->first);
        auto  method = it->second;

        (this->*method)(value);
    }

    _addToWorld();
}

PhysicsElem::~PhysicsElem()
{
    _removeFromWorld();
}

void PhysicsElem::_setVelocity (lxvar value)
{
    glgeom::vector3f vel = value.is_defined() ? glgeom::vector3f(value.convert()) : glgeom::vector3f(0, 0, 0);
    mspRigidBody->setLinearVelocity( btVector3(vel.x, vel.y, vel.z) );
}

void PhysicsElem::_setLinearDamping (lxvar value) 
{
    const btScalar angDamp = mspRigidBody->getAngularDamping();
    mspRigidBody->setDamping( query(value, 0.0f), angDamp );
}

void PhysicsElem::_setAngularDamping (lxvar value) 
{
    const btScalar linDamp = mspRigidBody->getLinearDamping();
    mspRigidBody->setDamping( linDamp, query(value, 0.0f) );
}

void PhysicsElem::_setFriction (lxvar value)
{
    mspRigidBody->setFriction( query(value, 0.5f) );
}

void PhysicsElem::_setRestitution (lxvar value)
{
    mspRigidBody->setRestitution( query(value, 0.1f) );
}

void PhysicsElem::_addImpulseFunc (ElementPtr spElem, std::vector<lxvar>& args) 
{
    if (args.size() == 1)
        addImpulse(args[0].convert());
} 

void PhysicsElem::onAttributeChange(ElementPtr spElem, std::string name, lxvar value)
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
    else
    {
        auto it = s_attrHandlers.find(name);
        if (it != s_attrHandlers.end())
            (this->*(it->second))(value);
    }
                
}

    void    
PhysicsElem::addImpulse (const glgeom::vector3f& v) 
{
#ifdef _DEBUG
    if (mspRigidBody->getInvMass() == 0.0f)
        lx_warn_once("addImpulse() called on a 0 mass object: objects without mass are immovable!");
#endif
    mspRigidBody->applyCentralImpulse( btVector3(v.x, v.y, v.z) );
}

void PhysicsElem::_addToWorld()
{
    mpDocPhysics->mspDynamicsWorld->addRigidBody(mspRigidBody.get());
}
void PhysicsElem::_removeFromWorld()
{
    mpDocPhysics->mspDynamicsWorld->removeRigidBody(mspRigidBody.get());
}
        
