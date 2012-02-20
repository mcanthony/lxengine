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

#pragma once

#include <lx0/plugins/bulletphysics.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include "shape_cache.hpp"

namespace lx0
{
    namespace subsystem
    {
        namespace physics_ns
        {
            namespace detail
            {
                //-----------------------------------------------------------------------//
                //! The internal physics subsystem
                /*!
                 */
                class PhysicsDoc : public IPhysicsDoc
                {
                public:
                    virtual const char* name() const { return "physics"; }

                                    PhysicsDoc();
                                    ~PhysicsDoc();

                    virtual void    onAttached          (DocumentPtr spDocument);

                    virtual void    onElementAdded      (DocumentPtr spDocument, ElementPtr spElem);
                    virtual void    onElementRemoved    (Document* pDocument, ElementPtr spElem);

                    virtual void    onUpdate            (DocumentPtr spDocument);

                    virtual void    enableSimulation    (bool bEnable);
                    virtual void    setGravity          (const glgeom::vector3f& gravity);

                    virtual void    addToWorld          (btRigidBody* pRigidBody);
                    virtual void    removeFromWorld     (btRigidBody* pRigidBody);
                    virtual btCollisionWorld* getWorld  (void);



                    void            setWindVelocity     (float v)           { mfWindVelocity = v; }
                    void            setWindDirection    (const glgeom::vector3f& v);

                    std::shared_ptr<btDiscreteDynamicsWorld>                mspDynamicsWorld;

                protected:
                    void            _applyWind              (const float timeStep);
                    void            _applyCollisonActions   (DocumentPtr spDocument);
                    void            _updateElements         (DocumentPtr spDocument);

                    float               mfWindVelocity;
                    glgeom::vector3f    mWindDirection;

                    bool                                                    mEnableSimulation;

                    std::shared_ptr<btBroadphaseInterface>                  mspBroadphase;
                    std::shared_ptr<btDefaultCollisionConfiguration>        mspCollisionConfiguration;
                    std::shared_ptr<btCollisionDispatcher>                  mspDispatcher;
                    std::shared_ptr<btSequentialImpulseConstraintSolver>    mspSolver;

                    std::shared_ptr<btCollisionShape>                       mspGroundShape;

                    std::shared_ptr<btDefaultMotionState>                   mspGroundMotionState;
                    std::shared_ptr<btRigidBody>                            mspGroundRigidBody;

                    unsigned int                                            mLastUpdate;
                };

            }
        }
    }
}
