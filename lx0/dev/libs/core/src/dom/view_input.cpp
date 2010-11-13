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
//   H E A D E R S
//===========================================================================//

#include <iostream>
#include <string>

#include <lx0/core.hpp>
#include <lx0/util.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include "view_input.hpp"

#include <OIS/OISMouse.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISJoyStick.h>
#include <OIS/OISInputManager.h>

using namespace OIS;
using namespace lx0::util;

//===========================================================================//
//   I M P L E M E N T A T I O N 
//===========================================================================//

namespace lx0 { namespace core { namespace detail {

    using OIS::KeyEvent;

    /*
        Developer notes:

        In general, multiple inheritance is avoided as can unnecessarily complicate
        the code.  For example, the View class does not inherit from OGRE's
        FrameListener or WindowListener classes as that would expose OGRE in the
        public header for the View (the goal is to keep the View as independent from
        the implementation as possible so other implementations can be supported
        in the future).  In this particular case, InputImp is a small leaf class 
        that is internal only, so for simplicity, just go ahead and use multiple 
        inhertance.
     */
    class InputImp
        : public OIS::KeyListener
        , public OIS::MouseListener
    {
    public:
        ///@todo pass in the window handle rather than an OGRE pointer - no need for OGRE here
        InputImp (LxInputManager* pInterface, size_t hWindowHandle, unsigned int windowWidth, unsigned int windowHeight)
            : mpInterface (pInterface)
        {
            ParamList params;
            params.insert( std::make_pair(std::string("WINDOW"), lx_itoa(hWindowHandle)) );
            
            // Don't hide the cursor
            params.insert( std::make_pair("w32_mouse", "DISCL_FOREGROUND") );
            params.insert( std::make_pair("w32_mouse", "DISCL_NONEXCLUSIVE") );    

            mpInputManager = InputManager::createInputSystem(params);


            // Adapted from the Irricht Engine:
            // http://www.irrlicht3d.org/wiki/index.php?n=Main.IntegratingOISWithIrrlicht
            //
            {
                const unsigned int v = mpInputManager->getVersionNumber();

                lx_debug("OIS Information");
                lx_debug("    Version: %d.%d.%d", (v>>16 ),((v>>8) & 0x000000FF),(v & 0x000000FF));
                lx_debug("    Release Name: %s", mpInputManager->getVersionName().c_str());
                lx_debug("    Manager: %s", mpInputManager->inputSystemName().c_str());
                lx_debug("    Devices Keyboards: %d", mpInputManager->getNumberOfDevices(OISKeyboard));
                lx_debug("    Devices Mice: %d", mpInputManager->getNumberOfDevices(OISMouse));
                lx_debug("    Devices Joysticks: %d", mpInputManager->getNumberOfDevices(OISJoyStick));
            }

            // Create buffered devices
            mpKeyboard = static_cast<OIS::Keyboard*>(mpInputManager->createInputObject( OISKeyboard, true ));
            mpMouse = static_cast<OIS::Mouse*>(mpInputManager->createInputObject( OISMouse, true ));

            // Set the window size on the mouse
            mpMouse->getMouseState().width = windowWidth;
            mpMouse->getMouseState().height = windowHeight;

            // Set up the event listeners
            mpKeyboard->setEventCallback(this);
            mpMouse->setEventCallback(this);
        }


        ~InputImp()
        {
            lx_check_error( mpInputManager != nullptr );
            lx_check_error( mpKeyboard != nullptr );
            lx_check_error( mpMouse != nullptr );

            mpKeyboard->setEventCallback(nullptr);
            mpMouse->setEventCallback(nullptr);

            mpInputManager->destroyInputObject( mpKeyboard );
            mpInputManager->destroyInputObject( mpMouse );

            InputManager::destroyInputSystem(mpInputManager);
        }

        void update()
        {
            mpKeyboard->capture();

            // Test code...
            if (mpKeyboard->isKeyDown(OIS::KC_ESCAPE))
                Engine::acquire()->sendMessage("quit");
        }

        bool 
        isKeyDown (int keyCode) const
        {
            return mpKeyboard->isKeyDown(OIS::KeyCode(keyCode));
        }

        virtual bool keyPressed( const KeyEvent &arg )
        {
            lx0::core::KeyEvent e;
            e.keyCode = int(arg.key);
            e.keyChar = char(arg.text);

            mpInterface->slotKeyDown(e);

            // true means to continue sending the event to any other listeners 
            return true;
        }

		virtual bool keyReleased( const KeyEvent &arg )
        {
            return true;
        }

        virtual bool mouseMoved( const MouseEvent &arg )
        {
            return true;
        }

		virtual bool mousePressed( const MouseEvent &arg, MouseButtonID id )
        {
            return true;
        }

		virtual bool mouseReleased( const MouseEvent &arg, MouseButtonID id )
        {
            return true;
        }

    protected:
        LxInputManager*         mpInterface;
        OIS::InputManager*      mpInputManager;
        OIS::Keyboard*          mpKeyboard;
        OIS::Mouse*             mpMouse;
    };



    LxInputManager::LxInputManager (size_t hWindowHandle, unsigned int width, unsigned int height)
        : mspImp ( new InputImp (this, hWindowHandle, width, height) )
    {
    }

    void 
    LxInputManager::update() 
    { 
        mspImp->update(); 
    }

    bool 
    LxInputManager::isKeyDown (int keyCode) const
    {
        return mspImp->isKeyDown(keyCode);
    }

}}}
