//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

#include <map>
#include <deque>
#include <memory>
#include <string>

#include <lx0/detail/forward_decls.hpp>
#include <lx0/lxvar.hpp>


namespace lx0 { namespace core {

    class Object;
    

    // --------------------------------------------------------------------- //
    //! Represents an Element in the Document Object Model.  
    /*!
        An Element has:
        * A Tag Name
        * A lxvar value
        * A Set of Attributes
        * A Set of Children

        <b>Comparison to HTML DOM</b>
        The HTML Document is composed of a tree of Nodes.  These nodes may be Elements, Text, Comments, 
        Attributes, etc.  Lx simplifies the DOM to be a tree of nodes where every node is an Element.

        In HTML, an Element can contains it's "value" as a set of child nodes - including text nodes.
        Again, the Lx DOM is simplified: the Element contains a single value field (stored as an 
        Object) and the notion of the Element's value is separate distinct and unrelated to the
        Element's children.
     */
    class Element 
        : public std::enable_shared_from_this<Element>
    {
    public:
        class Component : public std::enable_shared_from_this<Component>
        {
        public:
            virtual         ~Component() {}
            virtual void    onAttributeChange   (std::string name, lxvar value) {}
        };

        std::string     tagName     (void) const            { return mTagName; }    //!< Get DOM tagName of the Element
        void            tagName     (const char* s)         { mTagName = s; }       //!< Set DOM tagName of the Element
        void            tagName     (const std::string& s)  { tagName(s.c_str()); } //!< Set DOM tagName of the Element

        const lxvar     attr        (std::string name) const;
        void            attr        (std::string name, lxvar value);      

        ElementCPtr     parent      () const;
        ElementCPtr     child       (int i) const;
        ElementPtr      child       (int i);
        int             childCount  (void) const;

        const ObjectPtr value       (void) const    { return mspValue; }
        ObjectPtr       value       (void)          { return mspValue; }
        void            value       (ObjectPtr spValue);

        void            prepend     (ElementPtr spElem);
        void            append      (ElementPtr spElem);

        ElementPtr      _clone () const;

        void                attachComponent (std::string name, Component* pComponents);
        template <typename T>
        std::shared_ptr<T>  getComponent    (std::string name);

        float           queryAttr   (std::string name, float defValue);
        std::string     queryAttr   (std::string name, std::string defValue);

    protected:
        typedef std::map<std::string, lxvar>                      AttrMap;
        typedef std::deque<ElementPtr>                            ElemList;
        typedef std::map<std::string, std::shared_ptr<Component>> ComponentList;

        std::shared_ptr<Component>  _getComponentImp    (std::string name);

        std::string     mTagName;
        AttrMap         mAttributes;
        ElementPtr      mspParent;
        ElemList        mChildren;
        ObjectPtr       mspValue;      // May be a proxy object for delay-loading

        ComponentList   mComponents;
    };

    typedef std::shared_ptr<Element> ElementPtr;

    /*!
        Get a Component and dynamic cast it to the intended type.

        Example:

        auto spPhysics = spElem->getComponent<PhysicsComponent>("physics");
     */
    template <typename T>
    std::shared_ptr<T>  
    Element::getComponent (std::string name)
    {
        return std::dynamic_pointer_cast<T>( _getComponentImp(name) );
    }

}}

