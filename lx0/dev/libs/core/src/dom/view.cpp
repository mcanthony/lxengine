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

// Standard headers
#include <cassert>
#include <memory>

// Public headers
#include <lx0/view.hpp>
#include <lx0/core.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/engine.hpp>

// Internal headers
#include "view_input.hpp"

namespace lx0 { namespace core {


    View::View (std::string impType, Document* pDocument)
        : mpDocument (pDocument)
    {
        Engine::acquire()->incObjectCount("View");

        lx_check_error(pDocument != nullptr, "Views must have a valid host Document");

        // Create the view implementation object
        //
        if (impType == "OGRE")
            mspImp.reset( _createViewImpOgre(this) );
        else
            lx_error("Unrecognized View implementation: '%s'", impType.c_str());

        //
        // Hook into Document events
        //
        mpDocument->slotElementRemoved += [&](ElementPtr spElem) { 
            _onElementRemoved(spElem);
        };
        mpDocument->slotElementAdded += [&](ElementPtr spElem) { 
            _onElementAdded(spElem);
        };

        
    }

    View::~View()
    {
        Engine::acquire()->decObjectCount("View");
    }

    void
    View::_onElementAdded (ElementPtr spElem)
    {
        mspImp->_onElementAdded(spElem);
    }

    void
    View::_onElementRemoved (ElementPtr spElem)
    {
        mspImp->_onElementRemoved(spElem);
    }

    /*!
        Makes the view or window visible.
     */
    void 
    View::show()
    {
        size_t hWindowHandle;
        unsigned int width, height;
        mspImp->createWindow(this, hWindowHandle, width, height);

        // Create the input manager for the window, now that the window has been created
        mspLxInputManager.reset( new detail::LxInputManager(hWindowHandle, width, height) );
        mspLxInputManager->slotKeyDown += [&] (KeyEvent& e) { this->slotKeyDown(e); };
    
        mspImp->show(this, mpDocument);
    }

    void
    View::updateBegin()
    {
        mspImp->updateBegin();
    }

    void
    View::updateEnd()
    {
        mspImp->updateEnd();
    }

    /*
        Called by OGRE between queuing up call the render calls and the GPU actually
        blitting the frame: i.e. time when the CPU might potentially be idle waiting
        for the GPU.
     */
    void
    View::notifyViewImpIdle()
    {
        mspLxInputManager->update();
    }

    void
    View::updateFrame()
    {
        mspImp->updateFrame();
    }
}}
