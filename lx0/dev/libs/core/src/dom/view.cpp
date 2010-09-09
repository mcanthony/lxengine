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

#pragma once

#include <cassert>
#include <memory>

#include <lx0/view.hpp>
#include <lx0/core.hpp>

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreWindowEventUtilities.h>
#include <OGRE/OgreManualObject.h>
#include <OGRE/OgreMaterial.h>
#include <OGRE/OgreMaterialManager.h>

namespace lx0 { namespace core {

    namespace detail {

        class LxOgre
        {
        public:
            template <typename T> friend std::shared_ptr<T> acquireSingleton (std::weak_ptr<T>&);
            static LxOgrePtr acquire() { return acquireSingleton<LxOgre>(s_wpLxOgre); }

            Ogre::Root* root() { return mspRoot.get(); }


        protected:
            ~LxOgre() 
            {
            }

            LxOgre()
            {
                // Initialize OGRE
                try 
                {
                    mspRoot.reset(new Ogre::Root);
                    mspRoot->showConfigDialog();
                }
                catch (std::exception& e)
                {
                    fatal("OGRE exception caught during initialization");
                    throw e;
                }
            }

            static LxOgreWPtr s_wpLxOgre;

            std::auto_ptr<Ogre::Root>   mspRoot;
        };

        LxOgreWPtr LxOgre::s_wpLxOgre;


        class LxWindowEventListener : public Ogre::WindowEventListener
        {
        public:
            virtual void windowClosed(Ogre::RenderWindow* pRenderWindow)
            {
                 LxOgre::acquire()->root()->queueEndRendering();
            }
        };
    }

    using namespace detail;

    View::View()
        : mspLxOgre         ( LxOgre::acquire() )
        , mpRenderWindow    (0)
        , mpSceneMgr        (0)
    {
    }

    View::~View()
    {
        // These pointers are owned by OGRE.  Do not delete.  Set to NULL for
        // documentation purposes.
        mpRenderWindow = 0;
        mpSceneMgr  = 0;
    }

    /*!
        Makes the view or window visible.
     */
    void 
    View::show()
    {
        assert(mpRenderWindow == NULL);

        Ogre::Root& root = *mspLxOgre->root();

        // The render window creation also creates many internal OGRE data objects; therefore,
        // create it first.  Otherwise objects like the Camera won't even work.
        mpRenderWindow = root.initialise(true, "View" );  

        mpSceneMgr = root.createSceneManager(Ogre::ST_GENERIC, "generic");

        Ogre::Camera* mCamera = mpSceneMgr->createCamera("Camera");
        mCamera->setPosition(Ogre::Vector3(9.0f,8.0f,10.0f));
        mCamera->lookAt(Ogre::Vector3(0.0f,0.0f,0.0f));
        mCamera->setNearClipDistance(0.1f);
        mCamera->setFarClipDistance(100.0f);

        Ogre::Viewport* mViewport = mpRenderWindow->addViewport(mCamera);
        mViewport->setBackgroundColour(Ogre::ColourValue(0.1f,0.1f,0.16f));

        mCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));

        // Add ambient light since there currently are not standard lights
        mpSceneMgr->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.1));

        {
            Ogre::Light* light = mpSceneMgr->createLight("Light");
            light->setType(Ogre::Light::LT_POINT);
            light->setDiffuseColour(Ogre::ColourValue(1.0f,1.0f,1.0f));
            light->setSpecularColour(Ogre::ColourValue(1.0f,1.0f,1.0f));
            light->setPosition(10, 13, 18);
            light->setAttenuation(1000, 1, 0, 0);
        }

        {
            Ogre::MaterialPtr spMat = Ogre::MaterialManager::getSingleton().create("LxMaterial", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
            spMat->setLightingEnabled(true);
            spMat->setAmbient(.1, .1, .1);
            spMat->setDiffuse(1, 1, 1, 1);
            spMat->setSpecular(0, 0, 0, 0);
        }

        // See http://www.ogre3d.org/tikiwiki/ManualObject
        //
        {
            Ogre::Vector3 a (-1, -1, -1);
            Ogre::Vector3 b (-1,  1, -1);
            Ogre::Vector3 c ( 1, -1, -1);
            Ogre::Vector3 d ( 1,  1, -1);
            Ogre::Vector3 e (-1, -1,  1);
            Ogre::Vector3 f (-1,  1,  1);
            Ogre::Vector3 g ( 1, -1,  1);
            Ogre::Vector3 h ( 1,  1,  1);

            Ogre::ManualObject* pObject = mpSceneMgr->createManualObject("manual");

            auto add_quad = [pObject] (Ogre::Vector3& v0, Ogre::Vector3& v1, Ogre::Vector3& v2, Ogre::Vector3& v3) -> void {
                Ogre::Vector3 normal = (v1 - v0).crossProduct(v2 - v1);
                normal.normalise();

                pObject->position(v0); 
                pObject->normal(normal);
                pObject->position(v1);
                pObject->normal(normal);
                pObject->position(v2);
                pObject->normal(normal);

                pObject->position(v2); 
                pObject->normal(normal);
                pObject->position(v3);
                pObject->normal(normal);
                pObject->position(v0);
                pObject->normal(normal);
            };

            pObject->begin("LxMaterial", Ogre::RenderOperation::OT_TRIANGLE_LIST);
                add_quad(h, g, c, d); // X+    
                add_quad(e, f, b, a); // X- 
                add_quad(f, h, d, b); // Y+
                add_quad(g, e, a, c); // Y-
                add_quad(f, e, g, h); // Z+
                add_quad(a, b, d, c); // Z-
            pObject->end();
            mpSceneMgr->getRootSceneNode()->createChildSceneNode("Cube")->attachObject(pObject);
        }
    }

    void
    View::run()
    {
        std::auto_ptr<LxWindowEventListener> spWindowEventListener(new LxWindowEventListener);
        Ogre::WindowEventUtilities::addWindowEventListener(mpRenderWindow, spWindowEventListener.get());

        mspLxOgre->root()->startRendering();

        Ogre::WindowEventUtilities::removeWindowEventListener(mpRenderWindow, spWindowEventListener.get());
    }
}}