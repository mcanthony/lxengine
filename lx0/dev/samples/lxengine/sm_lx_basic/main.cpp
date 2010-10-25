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

// Library headers
#include <boost/program_options.hpp>

// Include necessary Bullet headers for this tutorial.
#include <bullet/btBulletDynamicsCommon.h>

// Lx0 headers
#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/view.hpp>
#include <lx0/controller.hpp>
#include <lx0/point3.hpp>

using namespace lx0::core;

//===========================================================================//
//  D E F I N I T I O N S
//===========================================================================//


static bool 
parseOptions (int argc, char** argv, lxvar& options)
{
    // See http://www.boost.org/doc/libs/1_44_0/doc/html/program_options/tutorial.html
    using namespace boost::program_options;

    // 
    // Build the description of the expected argument format and have
    // Boost parse the command line args.
    //
    std::string caption ("Syntax: %1 [options] <file>.\nOptions:");
    size_t p = caption.find("%1");
    caption = caption.substr(0, p) + argv[0] + caption.substr(p + 2);

    options_description desc (caption);
    desc.add_options()
        ("help", "Print usage information and exit.")
        ("file", value<std::string>()->default_value("data/sm_lx_basic/scene_000.xml"), "Scene file to display.")
        ;

    positional_options_description pos;
    pos.add("file", -1);

    variables_map vars;
    store(command_line_parser(argc, argv).options(desc).positional(pos).run(), vars);

    //
    // Now check the options for anything that might prevent execution 
    //

    if (vars.count("help"))
    {
        std::cout << desc << std::endl;
        return false;
    }
    if (vars.count("file") != 1)
    {
        std::cout << "Error: expected exactly one scene file to be specified." << std::endl << std::endl;
        std::cout << desc << std::endl;
        return false;
    }

    options.insert("file", vars["file"].as<std::string>());
    return true;
}


class Physics
{
public:
    Physics()
    {
        spBroadphase.reset( new btDbvtBroadphase );

        spCollisionConfiguration.reset( new btDefaultCollisionConfiguration );
        spDispatcher.reset( new btCollisionDispatcher(spCollisionConfiguration.get()) );

        spSolver.reset( new btSequentialImpulseConstraintSolver );

        spDynamicsWorld.reset( new btDiscreteDynamicsWorld(spDispatcher.get(), 
                                                           spBroadphase.get(), 
                                                           spSolver.get(), 
                                                           spCollisionConfiguration.get()) );
        spDynamicsWorld->setGravity(btVector3(0, 0, -9.81f));


        // Create collison shapes - which are reusable between various objects
        //
        spGroundShape.reset(new btStaticPlaneShape(btVector3(0,0,1), 0) );
        spFallShape.reset( new btSphereShape(0.5f) );

        spGroundMotionState.reset( new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0, 0, 0))) );

        const float fGroundMass = 0.0f;   // Infinite, immovable object
        const btVector3 groundIntertia(0, 0, 0);
        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(fGroundMass, spGroundMotionState.get(), spGroundShape.get(), groundIntertia);
        spGroundRigidBody.reset( new btRigidBody(groundRigidBodyCI) );
        spDynamicsWorld->addRigidBody(spGroundRigidBody.get());

        spFallMotionState.reset( new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,2.5f))) );
        const btScalar fFallMass = 1;
        btVector3 fallInertia(0,0,0);
        spFallShape->calculateLocalInertia(fFallMass, fallInertia);

        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(fFallMass, spFallMotionState.get(), spFallShape.get(), fallInertia);
        spFallRigidBody.reset( new btRigidBody(fallRigidBodyCI) );
        spDynamicsWorld->addRigidBody(spFallRigidBody.get());
    }

    ~Physics()
    {
        spDynamicsWorld->removeRigidBody(spFallRigidBody.get());
        spDynamicsWorld->removeRigidBody(spGroundRigidBody.get());
    }

    void 
    init (DocumentPtr spDocument)
    {
        auto spElem = spDocument->getElementById("fall");
        auto pos = asPoint3( spElem->attr("translation") );

        btTransform tform (btQuaternion(0,0,0,1), btVector3(pos.x, pos.y, pos.z));
        spFallRigidBody->getMotionState()->setWorldTransform( tform );
        spFallRigidBody->setCenterOfMassTransform(tform);
    }

    void 
    update(DocumentPtr spDocument)
    {
        const float fTimeSlice = 1.0f / 600.0f;
        const int kMaxSubSteps = 10;
        spDynamicsWorld->stepSimulation(fTimeSlice, kMaxSubSteps);
 
        btTransform trans;
        spFallRigidBody->getMotionState()->getWorldTransform(trans);
 
        const float h = trans.getOrigin().getZ();

        auto spElem = spDocument->getElementById("fall");
        lxvar pos = spElem->attr("translation");
        pos.at(2, h);
        spElem->attr("translation", pos);
    }

protected:
    std::shared_ptr<btBroadphaseInterface>                  spBroadphase;
    std::shared_ptr<btDefaultCollisionConfiguration>        spCollisionConfiguration;
    std::shared_ptr<btCollisionDispatcher>                  spDispatcher;
    std::shared_ptr<btSequentialImpulseConstraintSolver>    spSolver;
    std::shared_ptr<btDiscreteDynamicsWorld>                spDynamicsWorld;

    std::shared_ptr<btCollisionShape>                       spGroundShape;
    std::shared_ptr<btCollisionShape>                       spFallShape;

    std::shared_ptr<btDefaultMotionState>                   spGroundMotionState;
    std::shared_ptr<btRigidBody>                            spGroundRigidBody;

    std::shared_ptr<btDefaultMotionState>                   spFallMotionState;
    std::shared_ptr<btRigidBody>                            spFallRigidBody;
};

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    int exitCode = -1;

    try
    {
        lxvar options;
        if ( parseOptions(argc, argv, options) )
        {
            EnginePtr spEngine( Engine::acquire() );

            DocumentPtr spDocument = spEngine->loadDocument(*options.find("file"));

            {
                ViewPtr spView(new View);
                spDocument->connect("view", spView);
                spView->show();
            }

            Physics physics;
            physics.init(spDocument);
            spDocument->slotUpdateRun += [&] () { physics.update(spDocument); };

            ControllerPtr spController;
   
            exitCode = spEngine->run();

            // Demostrating that views *can* be detached by name.  This is not
            // necessary, as the Document releases all views on destruction 
            // automatically
            spDocument->disconnect("view");

            spEngine->shutdown();
        }
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
