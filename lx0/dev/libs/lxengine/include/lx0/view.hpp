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

#include <lx0/core/detail/forward_decls.hpp>
#include <lx0/core/detail/dom_base.hpp>
#include <lx0/core/base/slot.hpp>

namespace lx0 { namespace core {

    namespace detail {
        _LX_FORWARD_DECL_PTRS(LxInputManager);
    };

    class ViewImp
    {
    public:
        virtual     ~ViewImp() {}

        virtual void        createWindow    (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height) = 0;
        virtual void        destroyWindow   (void) {}
        virtual void        show            (View* pHostView, Document* pDocument) = 0;

        virtual     void        _onElementAdded             (ElementPtr spElem) = 0;
        virtual     void        _onElementRemoved           (ElementPtr spElem) = 0;

        virtual     void        updateBegin     (void) = 0;
        virtual     void        updateFrame     (void) = 0;
        virtual     void        updateEnd       (void) = 0;
    };


    //===========================================================================//
    //!
    /*!
        Developer notes:
        
        This class could use significant clean-up and componentization once 
        the code progresses a bit further to make the class' responsibilities
        more clear.
     */
    class View 
    {
    public:
                    View            (std::string impType, Document* pDocument);
                    ~View           (void);
        
        void        show            (void);

        void        updateBegin     (void);
        void        updateFrame     (void);
        void        updateEnd       (void);

        bool        isKeyDown       (int keyCode) const;


        slot<void (KeyEvent&)>      slotKeyDown;

        void        notifyViewImpIdle   (void);

    protected:


        void        _onElementAdded     (ElementPtr spElem);
        void        _onElementRemoved   (ElementPtr spElem);

        ViewImp*    _createViewImpOgre  (View* pView);

        Document*                   mpDocument;         //! Non-owning pointer
        detail::LxInputManagerPtr   mspLxInputManager;
        std::unique_ptr<ViewImp>    mspImp;
    };
}}
