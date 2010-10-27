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

#include <iostream>
#include <string>

#include <lx0/core.hpp>
#include <lx0/util.hpp>
#include <lx0/engine.hpp>
#include "view_input.hpp"

#include <OIS/OISMouse.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISJoyStick.h>
#include <OIS/OISInputManager.h>

using namespace OIS;
using namespace lx0::util;

namespace lx0 { namespace core { namespace detail {

    class InputImp
    {
    public:
        ///@todo pass in the window handle rather than an OGRE pointer - no need for OGRE here
        InputImp (size_t hWindowHandle, unsigned int windowWidth, unsigned int windowHeight)
        {
            ParamList params;
            params.insert( std::make_pair(std::string("WINDOW"), lx_atoi(hWindowHandle)) );

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
        }


        ~InputImp()
        {
            lx_check_error( mpInputManager );
            lx_check_error( mpKeyboard );
            lx_check_error( mpMouse );

            mpInputManager->destroyInputObject( mpKeyboard );
            mpInputManager->destroyInputObject( mpMouse );

            InputManager::destroyInputSystem(mpInputManager);
        }

        void update()
        {
            mpKeyboard->capture();

            // Test code...
            if (mpKeyboard->isKeyDown(OIS::KC_Q))
                Engine::acquire()->sendMessage("quit");
        }

    protected:
        OIS::InputManager*      mpInputManager;
        OIS::Keyboard*          mpKeyboard;
        OIS::Mouse*             mpMouse;
    };



    LxInputManager::LxInputManager (size_t hWindowHandle, unsigned int width, unsigned int height)
        : mspImp ( new InputImp (hWindowHandle, width, height) )
    {
    }

    void LxInputManager::update() { mspImp->update(); }

}}}