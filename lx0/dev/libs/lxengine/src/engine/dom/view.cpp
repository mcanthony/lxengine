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
#include <lx0/engine/view.hpp>
#include <lx0/lxengine.hpp>
#include <lx0/engine/document.hpp>
#include <lx0/engine/element.hpp>
#include <lx0/engine/engine.hpp>

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
        EnginePtr spEngine = Engine::acquire();
        static bool once = false;
        if (!once) 
        {
            once = true;
            spEngine->addViewPlugin("OGRE", [&](View* pView) { return _createViewImpOgre(pView); } );
        }
        ViewImp* pImp = spEngine->_createViewImp(impType, this);
        mspImp.reset( pImp );

        //
        // Hook into Document events
        //
        mOnElementRemovedId = (mpDocument->slotElementRemoved += [&](ElementPtr spElem) { _onElementRemoved(spElem); }); 
        mOnElementAddedId = (mpDocument->slotElementAdded += [&](ElementPtr spElem) { _onElementAdded(spElem); });
    }

    View::~View()
    {
        mpDocument->slotElementAdded -= mOnElementAddedId;
        mpDocument->slotElementRemoved -= mOnElementRemovedId; 

        mspImp->destroyWindow();
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

    void        
    View::sendEvent (std::string evt, lxvar params)
    {
        mspImp->handleEvent(evt, params);
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
        mspImp->updateFrame(mpDocument->shared_from_this());
    }

    bool
    View::isKeyDown (int keyCode) const
    {
        return mspLxInputManager->isKeyDown(keyCode);
    }

}}
