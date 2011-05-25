//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

#include <lx0/lxengine.hpp>
#include <lx0/views/canvas.hpp>
#include <lx0/subsystem/canvas.hpp>

using namespace lx0;

//===========================================================================//

class LxCanvasImp : public ViewImp
{
public:
    virtual void        createWindow    (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height, lxvar options);
    virtual void        destroyWindow   (void);
    virtual void        show            (View* pHostView, Document* pDocument);

    virtual     void        _onElementAdded             (ElementPtr spElem) {}
    virtual     void        _onElementRemoved           (ElementPtr spElem) {}

    virtual     void        updateBegin     (void) {}
    virtual     void        updateFrame     (DocumentPtr spDocument);
    virtual     void        updateEnd       (void) {}

    virtual     void        handleEvent     (std::string evt, lx0::lxvar params);

    virtual void            setRenderer         (IRenderer* pRenderer)         { mspRenderer.reset( pRenderer ); }
    virtual void            addController       (lx0::UIController* pController) { mControllers.push_back(std::shared_ptr<lx0::UIController>(pController)); }

protected:
    lx0::View*                  mpHostView;
    std::auto_ptr<CanvasGL>     mspWin;
    CanvasHost                  mHost;
   
    std::vector<std::shared_ptr<lx0::UIController>> mControllers;
    std::shared_ptr<IRenderer>  mspRenderer;
};

void 
LxCanvasImp::createWindow (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height, lxvar options)
{
    mpHostView = pHostView;

    std::string title = options.query("title", "LxEngine Canvas");
    width = options.query("width", 512);
    height = options.query("height", 512);

    mspWin.reset( new CanvasGL(title.c_str(), 16, 16, width, height, false) );
    handle = mspWin->handle();

    mspRenderer->initialize();

    mspWin->slotRedraw += [&]() { mspRenderer->render(); };

    for (auto it = mControllers.begin(); it != mControllers.end(); ++it)
    {
        auto spController = *it;
        mspWin->slotLMouseClick += [spController, this](const MouseState& ms, const ButtonState& bs, KeyModifiers km) { 
            spController->onLClick(mpHostView->shared_from_this(), ms, bs, km);
        };
        mspWin->slotLMouseDrag += [spController, this](const MouseState& ms, const ButtonState& bs, KeyModifiers km) {
            spController->onLDrag(mpHostView->shared_from_this(), ms, bs, km);
        };
    };
}

void
LxCanvasImp::destroyWindow (void)
{
    mspWin->destroy();
}

void 
LxCanvasImp::show (View* pHostView, Document* pDocument)
{
    mspWin->show();
}

void 
LxCanvasImp::updateFrame (DocumentPtr spDocument) 
{
    for (auto it = mControllers.begin(); it != mControllers.end(); ++it)
        (*it)->updateFrame( mpHostView->shared_from_this(), mspWin->keyboard() );

    mspRenderer->update();
}

void        
LxCanvasImp::handleEvent (std::string evt, lx0::lxvar params)
{
    if (evt == "redraw")
        mspWin->invalidate();
    else
        mspRenderer->handleEvent(evt, params);
}

//===========================================================================//

namespace lx0
{
    namespace views
    {
        namespace canvas_ns
        {

            lx0::ViewImp* createCanvasViewImp()
            {
                return new LxCanvasImp;;
            }
        }
    }
}


