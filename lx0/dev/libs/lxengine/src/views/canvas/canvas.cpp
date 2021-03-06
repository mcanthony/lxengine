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
                        LxCanvasImp     (void);

    virtual void        createWindow    (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height, lxvar options);
    virtual void        destroyWindow   (void);
    virtual void        show            (View* pHostView, Document* pDocument);
    virtual void        swapBuffers     (void);

    virtual int         width           (void) const;
    virtual int         height          (void) const;

    virtual     void        _onElementAdded             (ElementPtr spElem) {}
    virtual     void        _onElementRemoved           (ElementPtr spElem) {}

    virtual     void        runBegin     (void) {}
    virtual     void        update     (DocumentPtr spDocument);
    virtual     void        runEnd       (void) {}

    virtual     void        handleEvent     (std::string evt, lx0::lxvar params);

    virtual void            addUIBinding       (lx0::UIBinding* pController) { mBindings.push_back(std::shared_ptr<lx0::UIBinding>(pController)); }

protected:
    void                    _onKeyDown      (unsigned int keyCode);

    lx0::View*                  mpHostView;
    std::unique_ptr<CanvasGL>   mspWin;
    CanvasHost                  mHost;
   
    std::vector<std::shared_ptr<lx0::UIBinding>> mBindings;
};

LxCanvasImp::LxCanvasImp()
    : mpHostView (nullptr)
{
}

void 
LxCanvasImp::createWindow (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height, lxvar options)
{
    mpHostView = pHostView;

    std::string title = query_path(options, "title", "LxEngine Canvas");
    width = query_path(options, "width", 512);
    height = query_path(options, "height", 512);

    mspWin.reset( new CanvasGL(title.c_str(), 16, 16, width, height, false) );
    handle = mspWin->handle();

    pHostView->foreachComponent([=](View::ComponentPtr spComp) {
        spComp->initialize(pHostView->shared_from_this());
    });

    pHostView->foreachComponent([&](View::ComponentPtr spComp) {
        mspWin->slotRedraw += [spComp]() { spComp->render(); };
    });

    for (auto it = mBindings.begin(); it != mBindings.end(); ++it)
    {
        auto spController = *it;
        mspWin->slotLMouseClick += [spController, this](const MouseState& ms, const ButtonState& bs, KeyModifiers km) { 
            spController->onLClick(mpHostView->shared_from_this(), ms, bs, km);
        };
        mspWin->slotLMouseDrag += [spController, this](const MouseState& ms, const ButtonState& bs, KeyModifiers km) {
            spController->onLDrag(mpHostView->shared_from_this(), ms, bs, km);
        };
    };

    mspWin->slotKeyDown += [this](unsigned int keyCode) { _onKeyDown(keyCode); };
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
LxCanvasImp::swapBuffers (void)
{
    mspWin->swapBuffers();
}

int
LxCanvasImp::width (void) const
{
    return mspWin->width();
}

int 
LxCanvasImp::height (void) const
{
    return mspWin->height();
}

void 
LxCanvasImp::update (DocumentPtr spDocument) 
{
    lx_check_error(mpHostView, "Host view not set!");

    for (auto it = mBindings.begin(); it != mBindings.end(); ++it)
        (*it)->update( mpHostView->shared_from_this(), mspWin->keyboard() );

    mpHostView->foreachComponent([this](View::ComponentPtr spComp) {
        spComp->update(mpHostView->shared_from_this());
    });
}

void        
LxCanvasImp::handleEvent (std::string evt, lx0::lxvar params)
{
    if (evt == "redraw")
        mspWin->invalidate();
    else
    {
        mpHostView->foreachComponent([&](View::ComponentPtr spComp) {
            spComp->handleEvent(evt, params);
        });
    }
}

void        
LxCanvasImp::_onKeyDown (unsigned int keyCode)
{
    for (auto it = mBindings.begin(); it != mBindings.end(); ++it)
    {
        (*it)->onKeyDown(mpHostView->shared_from_this(), (int)keyCode);
    }
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

lx0::ViewImp* _hidden_createCanvasViewImp(lx0::View* pView)
{
    return lx0::createCanvasViewImp();
}


