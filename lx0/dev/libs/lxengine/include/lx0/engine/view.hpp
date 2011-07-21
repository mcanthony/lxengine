//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)

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

#include <lx0/_detail/forward_decls.hpp>
#include <lx0/core/slot/slot.hpp>
#include <lx0/core/lxvar/lxvar.hpp>
#include <lx0/engine/dom_base.hpp>
#include <lx0/engine/document.hpp>

namespace lx0 { namespace engine { namespace dom_ns {

    class KeyEvent;

    namespace detail {
        _LX_FORWARD_DECL_PTRS(LxInputManager);
    };

    //! \ingroup lx0_engine_dom
    enum KeyCodes
    {
        KC_UNASSIGNED = 0,

        KC_1,
        KC_2,
        KC_3,
        KC_4,
        KC_5,
        KC_6,
        KC_7,
        KC_8,
        KC_9,
        KC_0,

		KC_A,
        KC_B,
        KC_C,
        KC_D,
        KC_E,
        KC_F,
        KC_G,
        KC_H,
        KC_I,
        KC_J,
        KC_K,
        KC_L,
        KC_M,
        KC_N,
        KC_O,
        KC_P,
        KC_Q,
        KC_R,
        KC_S,
        KC_T,
        KC_U,
        KC_V,
        KC_W,
        KC_X,
        KC_Y,
        KC_Z,

        KC_ESCAPE,
        KC_SPACE,

        KC_SHIFT,
        KC_LSHIFT,
        KC_RSHIFT,

        KC_COUNT
    };

    //! \ingroup lx0_engine_dom
    /// WIP class
    class KeyModifiers
    {
    public:
        KeyModifiers() : packed (0) {}
        union
        {
            struct 
            {
                unsigned    ctrl    : 1;
                unsigned    rctrl   : 1;
                unsigned    lctrl   : 1;
                unsigned    alt     : 1;
                unsigned    ralt    : 1;
                unsigned    lalt    : 1;
                unsigned    shift   : 1;
                unsigned    rshift  : 1;
                unsigned    lshift  : 1;
            };
            unsigned int    packed;
        };
    };

    //! \ingroup lx0_engine_dom
    class KeyboardState
    {
    public:
        KeyboardState();

        KeyModifiers    modifiers;
        bool            bDown[KC_COUNT];
    };

    //! \ingroup lx0_engine_dom
    /// WIP class
    class MouseState
    {
    public:
        MouseState()
            : x (0)
            , y (0)
            , previousX(0)
            , previousY (0)
        {
        }

        int             deltaX      (void)      const { return x - previousX; }
        int             deltaY      (void)      const { return y - previousY; }

        int x, y;
        int previousX, previousY;
    };

    //! \ingroup lx0_engine_dom
    /// WIP class
    class ButtonState
    {
    public:
        ButtonState()
            : bDown     (false)
            , bDragging (false)
            , startX    (0)
            , startY    (0)
        {
        }

        bool            bDown;   
        bool            bDragging;              // Indicates the mouse has moved since the button was initially depressed
        int             startX, startY;         // drag starting point
    };

    //===========================================================================//

    class ViewImp
    {
    public:
        virtual     ~ViewImp() {}

        virtual void        createWindow    (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height, lxvar options) = 0;
        virtual void        destroyWindow   (void) {}
        virtual void        show            (View* pHostView, Document* pDocument) = 0;
        virtual void        swapBuffers     (void) {}


        virtual     void        _onElementAdded             (ElementPtr spElem) = 0;
        virtual     void        _onElementRemoved           (ElementPtr spElem) = 0;

        virtual     void        updateBegin     (void) = 0;
        virtual     void        updateFrame     (DocumentPtr spDocument) = 0;
        virtual     void        updateEnd       (void) = 0;

        virtual     void        addUIBinding       (UIBinding* pController) {}

        virtual     void        handleEvent     (std::string evt, lx0::lxvar params) {}
    };


    //===========================================================================//
    //! Interface for attaching objects to the View
    /*!
     */
    class ViewComponent : public Document::Component
    {
    public:

        virtual void initialize (ViewPtr spView) {}
        virtual void shutdown   (View* pView) {}

        virtual void render     (void) {}
        virtual void update     (ViewPtr spView) {}

        virtual void handleEvent (std::string evt, lx0::lxvar params) {}
    };


    //===========================================================================//
    //!
    /*!
        \ingroup lx0_engine_dom
        
        - A View is associated with one Document
        - A Document may have many views
        - A Document may embed other documents

        Developer notes:
        
        This class could use significant clean-up and componentization once 
        the code progresses a bit further to make the class' responsibilities
        more clear.
     */
    class View 
        : public std::enable_shared_from_this<View>
        , public detail::_EnableComponentList<View, ViewComponent>
    {
    public:
                    View            (std::string impType, Document* pDocument);
                    ~View           (void);

        DocumentPtr document        (void);
        
        void        show            (void);
        void        show            (lxvar options);
        void        swapBuffers     (void);

        void        updateBegin     (void);
        void        updateFrame     (void);
        void        updateEnd       (void);

        void        sendEvent       (std::string evt) { sendEvent(evt, lxvar()); }
        void        sendEvent       (std::string evt, lx0::lxvar params);

        bool        isKeyDown       (int keyCode) const;


        slot<void (KeyEvent&)>          slotKeyDown;

        void        notifyViewImpIdle   (void);
        void        notifyAttached      (ComponentPtr spComponent) { spComponent->onAttached(mpDocument->shared_from_this()); } 

        void        addUIBinding        (UIBinding* pController)           { mspImp->addUIBinding(pController); }
        void        addController       (Controller* pController);

    protected:
        /*
            Forwards all document events to the View; thus creating per-view 
            versions of the document events.
         */
        class DocForwarder : public Document::Component
        {
        public:
            DocForwarder (View* pView) : mpView (pView) {}

            virtual const char* name() const { return "_docForwarder"; }
            virtual void    onAttached          (DocumentPtr spDocument);

            virtual void    onUpdate            (DocumentPtr spDocument);

            virtual void    onElementAdded      (DocumentPtr spDocument, ElementPtr spElem);
            virtual void    onElementRemoved    (Document*   pDocument, ElementPtr spElem);
        
        protected:
            View* mpView;
        };


        void        _onElementAdded     (ElementPtr spElem);
        void        _onElementRemoved   (ElementPtr spElem);

        ViewImp*    _createViewImpOgre  (View* pView);

        Document*                   mpDocument;         //! Non-owning pointer
        detail::LxInputManagerPtr   mspLxInputManager;
        std::unique_ptr<ViewImp>    mspImp;
        DocForwarder*               mpDocForwarder;
        std::vector<lx0::ControllerPtr> mControllers;

        int mOnElementRemovedId;
        int mOnElementAddedId;
    };
}}

using namespace lx0::engine::dom_ns;
}
