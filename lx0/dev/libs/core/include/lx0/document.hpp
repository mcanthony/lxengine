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
    class Document : public std::enable_shared_from_this<Document>
    {
    public:
        class Component : public std::enable_shared_from_this<Component>
        {
        public:
            virtual         ~Component() {}

            virtual void    onElementAdded      (DocumentPtr spDocument, ElementPtr spElem) {}
            virtual void    onElementRemoved    (Document*   pDocument, ElementPtr spElem) {}
        };

                                Document();
                                ~Document();

        TransactionPtr          transaction     (void);

        void                    connect         (std::string name, ViewPtr spView);
        void                    disconnect      (std::string name);

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


        void                    attachComponent (std::string name, Component* pComponent);
        template <typename T>
        std::shared_ptr<T>      getComponent    (std::string name);
        template <typename T>
        std::shared_ptr<T>      ensureComponent (std::string name, std::function<T* (void)> ctor);

        virtual void            notifyElementAdded      (ElementPtr spElem);
        virtual void            notifyElementRemoved    (ElementPtr spElem);

        slot<void(ElementPtr)>  slotElementCreated;
        slot<void(ElementPtr)>  slotElementAdded;
        slot<void(ElementPtr)>  slotElementRemoved;

        slot<void()>            slotUpdateRun;
        slot<void(KeyEvent&)>   slotKeyDown;            // Key down on any of the Document's views

        bool                    _containsElement    (ElementPtr spElementPtr);

    protected:
        typedef std::map<std::string, std::shared_ptr<Component>> ComponentList;
        typedef std::vector< TransactionWPtr > TrWList;

        std::shared_ptr<Component>  _getComponentImp    (std::string name);
        bool                        _walkElements       (std::function<bool (ElementPtr)> f);

        TrWList                         m_openTransactions;     //!< Not currently implemented
        ElementPtr                      m_spRoot;
        std::map<std::string, ViewPtr>  m_views;
        ComponentList                   mComponents;
    };

    /*!
        Get a Component and dynamic cast it to the intended type.

        Example:

        auto spPhysics = spElem->getComponent<PhysicsComponent>("physics");
     */
    template <typename T>
    std::shared_ptr<T>  
    Document::getComponent (std::string name)
    {
        return std::dynamic_pointer_cast<T>( _getComponentImp(name) );
    }

    template <typename T>
    std::shared_ptr<T>
    Document::ensureComponent (std::string name, std::function<T* (void)> ctor)
    {
        std::shared_ptr<Component> spComponent = _getComponentImp(name);
        if (!spComponent)
        {
            spComponent.reset( ctor() );
            mComponents.insert( std::make_pair(name, spComponent) );
        }
        return std::dynamic_pointer_cast<T>(spComponent);
    }

}}
