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

namespace lx0 { namespace core {

    namespace detail {

        class LxOgre
        {
        public:
            template <typename T> friend std::shared_ptr<T> acquireSingleton (std::weak_ptr<T>&);
            static LxOgrePtr acquire() { return acquireSingleton<LxOgre>(s_wpLxOgre); }

            Ogre::Root* root() { return mpRoot; }
            Ogre::Root* mpRoot;

        protected:
            ~LxOgre() 
            {
                //@todo Leaking mpRoot!  Crash on shutdown otherwise due to a corrupt std::vector<>
            }

            LxOgre()
            {
                // Initialize OGRE
                try 
                {
                    mpRoot = new Ogre::Root;
                    mpRoot->showConfigDialog();
                }
                catch (std::exception& e)
                {
                    fatal("OGRE exception caught during initialization");
                    throw e;
                }
            }

            static LxOgreWPtr s_wpLxOgre;
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
        mCamera->setPosition(Ogre::Vector3(0.0f,0.0f,500.0f));
        mCamera->lookAt(Ogre::Vector3(0.0f,0.0f,0.0f));
        mCamera->setNearClipDistance(1.0f);
        mCamera->setFarClipDistance(5000.0f);

        Ogre::Viewport* mViewport = mpRenderWindow->addViewport(mCamera);
        mViewport->setBackgroundColour(Ogre::ColourValue(1.0f,1.0f,0.98f));

        mCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));
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