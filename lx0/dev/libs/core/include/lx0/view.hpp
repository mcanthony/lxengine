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

#include <string>
#include <memory>

#include <lx0/detail/forward_decls.hpp>
#include <lx0/slot.hpp>

namespace lx0 { namespace core {

    class KeyEvent;
    _LX_FORWARD_DECL_PTRS(Mesh);

    namespace detail {
        _LX_FORWARD_DECL_PTRS(LxOgre);
        _LX_FORWARD_DECL_PTRS(LxInputManager);

        class LxWindowEventListener;
        class LxFrameEventListener;
    };

    /*!
        Developer notes:
        
        This class could use significant clean-up and componentization once 
        the code progresses a bit further to make the class' responsibilities
        more clear.
     */
    class View
    {
    public:
        friend class detail::LxFrameEventListener;

        View();
        ~View();
        
        void        show            (void);

        void        updateBegin     (void);
        void        updateFrame     (void);
        void        updateEnd       (void);

        void        attach          (Document* pDocument);
        void        detach          (Document* pDocument);

        slot<void (KeyEvent&)>      slotKeyDown;

    protected:
        void        _updateFrameRenderingQueued();

        void        _onElementAdded (ElementPtr spElem);
        void        _processGroup   (ElementPtr spElem);
        void        _processRef     (ElementPtr spElem);
        void        _addMesh        (std::string name, MeshPtr spMesh);

        detail::LxOgrePtr           mspLxOgre;
        detail::LxInputManagerPtr   mspLxInputManager;
        Ogre::RenderWindow*         mpRenderWindow; //! Non-owning pointer.  OGRE owns this pointer.
        Ogre::SceneManager*         mpSceneMgr;     //! Non-owning pointer.  OGRE owns this pointer.
        Document*                   mpDocument;     //! Non-owning pointer.  Document will detach itself.

        std::unique_ptr<detail::LxWindowEventListener> mspWindowEventListener;
        std::unique_ptr<detail::LxFrameEventListener>  mspFrameEventListener;
    };
}}
