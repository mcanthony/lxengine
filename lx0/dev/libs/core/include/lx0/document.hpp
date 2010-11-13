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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

// Standard headers
#include <memory>
#include <vector>
#include <map>

// Lx headers
#include <lx0/detail/forward_decls.hpp>
#include <lx0/detail/dom_base.hpp>
#include <lx0/slot.hpp>

namespace lx0 { namespace core {

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

        virtual void    onAttached          (DocumentPtr spDocument) {}

        virtual void    onUpdate            (DocumentPtr spDocument) {}

        virtual void    onElementAdded      (DocumentPtr spDocument, ElementPtr spElem) {}
        virtual void    onElementRemoved    (Document*   pDocument, ElementPtr spElem) {}
    };

    //===========================================================================//
    //!
    /*!
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
        void                    destroyView     (std::string name);

        ViewPtr                 view            (int index);

        ElementCPtr             root            (void) const        { return m_spRoot; }
        ElementPtr              root            (void)              { return m_spRoot; }
        void                    root            (ElementPtr spRoot);

        ElementPtr              createElement           (void)                  { return createElement(""); }
        ElementPtr              createElement           (std::string type);
        ElementPtr              getElementById          (std::string id);
        std::vector<ElementPtr> getElementsByTagName    (std::string name);

        void                    beginRun        (void);
        void                    updateRun       (void);
        void                    endRun          (void);

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
    };

}}
