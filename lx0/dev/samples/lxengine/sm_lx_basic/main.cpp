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
#include <lx0/util.hpp>

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

        mspFallMotionState.reset( new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,2.5f))) );
        const btScalar fFallMass = 1;
        btVector3 fallInertia(0,0,0);
        mspCubeShape->calculateLocalInertia(fFallMass, fallInertia);

        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(fFallMass, mspFallMotionState.get(), mspCubeShape.get(), fallInertia);
        mspFallRigidBody.reset( new btRigidBody(fallRigidBodyCI) );
        mspDynamicsWorld->addRigidBody(mspFallRigidBody.get());
    }

    ~Physics()
    {
        mspDynamicsWorld->removeRigidBody(mspFallRigidBody.get());
        mspDynamicsWorld->removeRigidBody(mspGroundRigidBody.get());

        for (auto it = mRigidBodies.begin(); it != mRigidBodies.end(); ++it)
            mspDynamicsWorld->removeRigidBody( it->get() );
    }

    void 
    init (DocumentPtr spDocument)
    {
        auto spElem = spDocument->getElementById("fall");
        auto pos = asPoint3( spElem->attr("translation") );

        btTransform tform (btQuaternion(0,0,0,1), btVector3(pos.x, pos.y, pos.z));
        mspFallRigidBody->getMotionState()->setWorldTransform( tform );
        mspFallRigidBody->setCenterOfMassTransform(tform);

        auto allElems = spDocument->getElementsByTagName("Ref");
        for (auto it = allElems.begin(); it != allElems.end(); ++it)
        {
            auto spElem = *it;
            auto id = spElem->attr("id");

            if (!id.isString() || id.asString() != "fall")
            {
                auto pos = asPoint3( spElem->attr("translation") );
                btTransform tform (btQuaternion(0,0,0,1), btVector3(pos.x, pos.y, pos.z));
                std::shared_ptr<btDefaultMotionState> spMotionState( new btDefaultMotionState(tform) );
                
                const btScalar kfMass = 0;
                btVector3 fallInertia(0,0,0);
                mspCubeShape->calculateLocalInertia(kfMass, fallInertia);
                btRigidBody::btRigidBodyConstructionInfo rigidBodyCI (kfMass, spMotionState.get(), mspCubeShape.get(), fallInertia);

                std::shared_ptr<btRigidBody> spRigidBody( new btRigidBody(rigidBodyCI) );
                mspDynamicsWorld->addRigidBody(spRigidBody.get());

                mMotionStates.push_back( spMotionState );
                mRigidBodies.push_back( spRigidBody);
            }
        }

        mLastUpdate = lx0::util::lx_milliseconds();
    }

    void 
    update(DocumentPtr spDocument)
    {
        // In milliseconds...
        const float kFps = 60.0f;
        const unsigned int kFrameDurationMs = unsigned int( (1.0f / kFps) * 1000.0f );

        auto timeNow = lx0::util::lx_milliseconds();

        if (timeNow - mLastUpdate >= kFrameDurationMs)
        {
            const int kMaxSubSteps = 10;
            mspDynamicsWorld->stepSimulation(kFrameDurationMs / 1000.0f, kMaxSubSteps);
 
            btTransform trans;
            mspFallRigidBody->getMotionState()->getWorldTransform(trans);
 
            const float h = trans.getOrigin().getZ();

            auto spElem = spDocument->getElementById("fall");
            lxvar pos = spElem->attr("translation");
            pos.at(2, h);
            spElem->attr("translation", pos);

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

    std::shared_ptr<btDefaultMotionState>                   mspFallMotionState;
    std::shared_ptr<btRigidBody>                            mspFallRigidBody;

    std::vector< std::shared_ptr<btDefaultMotionState> >    mMotionStates;
    std::vector< std::shared_ptr<btRigidBody> >             mRigidBodies;

    unsigned int                                            mLastUpdate;
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
