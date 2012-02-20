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

#include <map>
#include <functional>
#include <lx0/_detail/forward_decls.hpp>

class btRigidBody;
class btCollisionWorld;
_LX_FORWARD_DECL_PTRS(btCollisionShape);


namespace lx0
{
    namespace subsystem
    {
        namespace physics_ns
        {
            _LX_FORWARD_DECL_PTRS(IPhysicsEngine);
            _LX_FORWARD_DECL_PTRS(IPhysicsDoc);

            /*!
                
             */
            class IPhysicsEngine : public lx0::Engine::Component
            {
            public:
                virtual const char* name() const { return "physics"; }
                static  const char* s_name()     { return "physics"; }

                typedef std::function<lx0::Element::Component* (lx0::ElementPtr spElement)>  Constructor;

                template <typename T>
                void addElementComponent (std::string tag);
                Constructor findElementComponentCtor (std::string tag);

                virtual btCollisionShapePtr     acquireSphereShape  (float radius) = 0;
                virtual btCollisionShapePtr     acquireBoxShape     (const glgeom::vector3f& halfBounds) = 0;
                virtual btCollisionShapePtr     acquireCapsuleShape (float width, float height) = 0;

                virtual void                    enableSimulation    (bool bEnable) = 0;

                virtual void                    setGravity          (const glgeom::vector3f& gravity) = 0;


            protected:
                std::map<std::string, Constructor>   mComponentCtors;
            };

            template <typename T>
            void IPhysicsEngine::addElementComponent (std::string tag)
            {
                auto ctor = [](lx0::ElementPtr spElement) { return new T(spElement); };
                mComponentCtors.insert(std::make_pair(tag, ctor));
            }

            class IPhysicsDoc : public lx0::Document::Component
            {
            public:
                virtual const char* name() const { return "physics"; }
                static  const char* s_name()     { return "physics"; }

                virtual void    enableSimulation    (bool bEnable) = 0;
                virtual void    setGravity          (const glgeom::vector3f& gravity) = 0;

                virtual void    addToWorld      (btRigidBody* pRigidBody) = 0;
                virtual void    removeFromWorld (btRigidBody* pRigidBody) = 0;
                virtual btCollisionWorld* getWorld (void) = 0;
            };
        }
    }
    using namespace lx0::subsystem::physics_ns;
}
