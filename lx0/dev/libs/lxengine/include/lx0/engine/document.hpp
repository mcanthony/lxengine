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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

// Standard headers
#include <memory>
#include <vector>
#include <map>

// Lx headers
#include <lx0/_detail/forward_decls.hpp>
#include <lx0/engine/dom_base.hpp>
#include <lx0/core/slot/slot.hpp>
#include <lx0/core/lxvar/lxvar.hpp>

namespace lx0 
{ 
    namespace engine 
    { 
        namespace dom_ns 
        {

    //===========================================================================//
    //!
    /*!
     */    
    class KeyEvent
    {
    public:
        int  keyCode;           //!< Currently the OIS key code mapping; subject to change.
        char keyChar;
    };

    //===========================================================================//
    //!
    /*!
     */
    class DocumentComponent : public detail::_ComponentBase
    {
    public:
        virtual         ~DocumentComponent() {}

        /*!
            Called immediatedly after the Component is attached to the Document
         */
        virtual void    onAttached          (DocumentPtr spDocument) {}

        virtual void    onUpdate            (DocumentPtr spDocument) {}

        virtual void    onElementAdded      (DocumentPtr spDocument, ElementPtr spElem) {}
        virtual void    onElementRemoved    (Document*   pDocument, ElementPtr spElem) {}
    };

    //===========================================================================//
    //! A Document in the LxEngine Document Object Model (DOM)
    /*!
        \ingroup lx0_engine_dom

        A Document represents an XML document (i.e. a tree of Element objects) 
        with an API similar to that of a simplified HTML DOM.  The LxEngine DOM
        is tailored toward more numeric data and arrays than text data, thus has
        differences from the HTML DOM, but tries to make a general parallel that
        will be familar to HTML DOM users.s
     */
    class Document 
        : public std::enable_shared_from_this<Document>
        , public detail::_EnableComponentList<Document, DocumentComponent>

    {
    public:
                                Document();
                                ~Document();

        TransactionPtr          transaction     (void);

        ViewPtr                 createView      (std::string type, std::string name);
        ViewPtr                 createView      (std::string type, std::string name, lx0::ViewComponent* pRenderer);
        void                    destroyView     (std::string name);

        ViewPtr                 view            (int index);

        ElementCPtr             root            (void) const        { return m_spRoot; }
        ElementPtr              root            (void)              { return m_spRoot; }
        void                    root            (ElementPtr spRoot);

        ElementPtr              createElement           (void)                  { return createElement(""); }
        ElementPtr              createElement           (std::string type);
        ElementPtr              getElementById          (std::string id);
        std::vector<ElementPtr> getElementsByTagName    (std::string name);
        std::vector<ElementPtr> getElements             (void);

        void                    iterateElements     (std::function<bool (ElementPtr)> f) { _walkElements(f); }
        void                    iterateElements2    (std::function<void (ElementPtr)> f);

        void                    beginRun        (void);
        void                    updateRun       (void);
        void                    endRun          (void);

        void                    sendEvent       (std::string evt, lx0::lxvar params = lxvar());
        void                    addController   (Controller* pController);

        void                    notifyAttached          (ComponentPtr spComponent) { spComponent->onAttached(shared_from_this()); }
        void                    notifyElementAdded      (ElementPtr spElem);
        void                    notifyElementRemoved    (ElementPtr spElem);

        slot<void(ElementPtr)>  slotElementCreated;
        slot<void(ElementPtr)>  slotElementAdded;
        slot<void(ElementPtr)>  slotElementRemoved;

        slot<void()>            slotUpdateRun;
        slot<void(KeyEvent&)>   slotKeyDown;            // Key down on any of the Document's views

        bool                    _containsElement    (ElementPtr spElementPtr);

    protected:
        typedef std::map<std::string, std::shared_ptr<Component>> ComponentList;
        typedef std::vector< TransactionWPtr > TrWList;

        bool                        _walkElements       (std::function<bool (ElementPtr)> f);

        TrWList                         m_openTransactions;     //!< Not currently implemented
        ElementPtr                      m_spRoot;
        std::map<std::string, ViewPtr>  m_views;
        std::vector<lx0::ControllerPtr> mControllers;
    };

        }
    }
    using namespace lx0::engine::dom_ns;
}
