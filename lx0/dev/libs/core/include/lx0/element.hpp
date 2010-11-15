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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

// Standard headers
#include <map>
#include <deque>
#include <memory>
#include <string>

// Lx headers
#include <lx0/detail/forward_decls.hpp>
#include <lx0/detail/dom_base.hpp>
#include <lx0/lxvar.hpp>


namespace lx0 { namespace core {  

    class ElementComponent : public detail::_ComponentBase
    {
    public:
        virtual         ~ElementComponent() {}

        virtual void    onAttributeChange   (ElementPtr spElem, std::string name, lxvar value) {}
        virtual void    onValueChange       (ElementPtr spElem, lxvar value) {}
        virtual void    onAdded             (void) {}
        virtual void    onRemoved           (void) {}

        virtual void    addImpulse          (const vector3& v) {}
    };

    //===========================================================================//
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
        , public detail::_EnableComponentList<Element, ElementComponent>
    {
    public:
                        Element         (void);
                        ~Element        (void);

        std::string     tagName         (void) const            { return mTagName; }    //!< Get DOM tagName of the Element
        void            tagName         (const char* s)         { mTagName = s; }       //!< Set DOM tagName of the Element
        void            tagName         (const std::string& s)  { tagName(s.c_str()); } //!< Set DOM tagName of the Element

        lxvar           attr            (std::string name) const;
        void            attr            (std::string name, lxvar value);      

        ElementCPtr     parent          (void) const;
        ElementPtr      parent          (void);
        ElementCPtr     child           (int i) const;
        ElementPtr      child           (int i);
        int             childCount      (void) const;
        void            removeChild     (ElementPtr spElem);

        lxvar           value           (void) const    { return mValue; }
        lxvar           value           (void)          { return mValue; }
        void            value           (lxvar v);

        void            prepend         (ElementPtr spElem);
        void            append          (ElementPtr spElem);

        ElementPtr      _clone          (void) const;

        void            notifyAdded     (Document* pDocument);
        void            notifyRemoved   (Document* pDocument);

        DocumentPtr     document        (void);

        void            addImpulse      (const vector3& v);

    protected:
        typedef std::map<std::string, lxvar>    AttrMap;
        typedef std::deque<ElementPtr>          ElemList;

        void            _setHostDocument    (Document* pDocument);

        Document*       mpDocument;     // Non-owning pointer to host document

        std::string     mTagName;
        AttrMap         mAttributes;
        ElementPtr      mspParent;
        ElemList        mChildren;
        mutable lxvar   mValue; 
    };

}}

