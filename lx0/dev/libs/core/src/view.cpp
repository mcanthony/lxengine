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

#include <lx0/view.hpp>

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreRenderWindow.h>

namespace lx0 { namespace core {

    namespace detail {
        class ViewImp
        {
        };
    }

    View::View()
        :  m_pRenderWindow (NULL)
    {
    }

    /*!
        Assigns the view a data source.

        A view may display the entire document or a portion of it.  
     */
    void 
    View::connect (DocumentCPtr spDoc)
    {
    }

    /*!
        Makes the view or window visible.
     */
    void 
    View::show()
    {
        assert(m_pRenderWindow == NULL);

        Ogre::Root& root = Ogre::Root::getSingleton();

        // The render window creation also creates many internal OGRE data objects; therefore,
        // create it first.  Otherwise objects like the Camera won't even work.
        m_pRenderWindow = root.initialise(true, "View" );  

        Ogre::SceneManager* pSceneMgr = root.createSceneManager(Ogre::ST_GENERIC, "generic");

        Ogre::Camera* mCamera = pSceneMgr->createCamera("Camera");
        mCamera->setPosition(Ogre::Vector3(0.0f,0.0f,500.0f));
        mCamera->lookAt(Ogre::Vector3(0.0f,0.0f,0.0f));
        mCamera->setNearClipDistance(1.0f);
        mCamera->setFarClipDistance(5000.0f);

         

        Ogre::Viewport* mViewport = m_pRenderWindow->addViewport(mCamera);
        mViewport->setBackgroundColour(Ogre::ColourValue(0.0f,0.0f,0.0f));

        mCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));
    }
}}