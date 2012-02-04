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

#include <bullet/btBulletDynamicsCommon.h>

namespace lx0
{
    namespace subsystem
    {
        namespace physics_ns
        {
            namespace detail
            {
                using namespace lx0;

                class PhysicsDoc;

                class PhysicsElem : public Element::Component
                {
                public:
                    virtual const char* name() const;

                                        PhysicsElem     (DocumentPtr spDocument, ElementPtr spElem, PhysicsDoc* pPhysics);
                                        ~PhysicsElem    ();

                    void            _setVelocity (lxvar value);
                    void            _setLinearDamping (lxvar value) ;
                    void            _setAngularDamping (lxvar value) ;
                    void            _setFriction (lxvar value);
                    void            _setRestitution (lxvar value);
                    void            _addImpulseFunc (ElementPtr spElem, std::vector<lxvar>& args) ;

                    virtual lx0::uint32 flags               (void) const { return eSkipUpdate; }
                    virtual void    onAttributeChange(ElementPtr spElem, std::string name, lxvar value);
                    virtual void    addImpulse (const glgeom::vector3f& v) ;
                    void            _addToWorld();
                    void            _removeFromWorld();


                    typedef void (PhysicsElem::*AttributeHandler)(lxvar);
                    typedef std::map<std::string, AttributeHandler> AttributeTable;
                    
                    static  AttributeTable                  s_attrHandlers;

                    PhysicsDoc*                             mpDocPhysics;
                    std::unique_ptr<btDefaultMotionState>   mspMotionState;
                    std::unique_ptr<btRigidBody>            mspRigidBody;
                    std::shared_ptr<btCollisionShape>       mspShape;
                };
            }
        }
    }
}
