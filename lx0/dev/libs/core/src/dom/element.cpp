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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

#include <cassert>

#include <lx0/core.hpp>
#include <lx0/element.hpp>
#include <lx0/object.hpp>
#include <lx0/document.hpp>

namespace lx0 { namespace core {

    Element::Element (Document* pDocument)
        : mpDocument (pDocument)
    {
    }

    void    
    Element::prepend (ElementPtr spElem)
    {
        lx_check_error(!"Not implemented");
    }

    void
    Element::append (ElementPtr spElem)
    {
        lx_check_error(this != nullptr);
        lx_check_error(spElem->parent().get() == nullptr);
       
        spElem->mspParent = shared_from_this();
        mChildren.push_back(spElem);

        mpDocument->slotElementAdded(spElem);
    }

    /*
        If this element has a parent, then it returns a const, read-only 
        shared-pointer to the parent element.

        If this element does not have a parent (i.e. is the root of a document
        or is the root of a sub-tree that has not been attached to a document),
        then a empty shared_ptr will be returned.
     */
    ElementCPtr     
    Element::parent() const
    {
        return mspParent;
    }
    
    ElementCPtr     
    Element::child(int i) const
    {
        if (i >= 0 && i < int(mChildren.size()))
        {
            return mChildren[i];
        }
        else
        {
            lx_error("Index %d out of range in Element::child()", i);
            return ElementCPtr();
        }
    }

    ElementPtr     
    Element::child(int i)
    {
        if (i >= 0 && i < int(mChildren.size()))
        {
            return mChildren[i];
        }
        else
        {
            lx_error("Index %d out of range in Element::child()", i);
            return ElementPtr();
        }
    }

    int
    Element::childCount (void) const
    {
        return int(mChildren.size());
    }

    ElementPtr      
    Element::_clone () const
    {
        if (this == 0)
        {
            lx_warn("Cloning a null pointer!");
            assert(0);
            return ElementPtr();
        }

        Element* pClone = new Element(mpDocument);
        pClone->mAttributes = mAttributes;
        pClone->mspParent = mspParent;
        pClone->mChildren = mChildren;
        if (mspValue.get())
            pClone->mspValue = mspValue->clone();
        return ElementPtr(pClone);
    }

    void
    Element::attr(std::string name, lxvar value)
    {
        lx_check_error( this != nullptr );

        for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
            it->second->onAttributeChange(name, value);

        mAttributes[name] = value;
    }

    const lxvar     
    Element::attr (std::string name) const
    {
        lx_check_error( this != nullptr );

        auto it = mAttributes.find(name);
        if (it != mAttributes.end())
            return it->second;
        else
            return lxvar();
    }

    /*!
        Convenience method for Element::attr().  In the case that the provided
        attribute "name" does not exist on the Element or is of the wrong type
        then the "defValue" will be returned instead.
     */
    float
    Element::queryAttr (std::string name, float defValue)
    {
        lxvar v = attr(name);
        if (v.isFloat() || v.asInt())
            return v.asFloat();
        else
            return defValue;
    }

    /*!
        Convenience method for Element::attr().  In the case that the provided
        attribute "name" does not exist on the Element or is of the wrong type
        then the "defValue" will be returned instead.
     */
    std::string
    Element::queryAttr (std::string name, std::string defValue)
    {
        lxvar v = attr(name);
        if (v.isString())
            return v.asString();
        else
            return defValue;
    }

    void
    Element::value(ObjectPtr spValue)
    {
        mspValue = spValue;
    }

    void 
    Element::attachComponent (std::string name, Component* pComponent)
    {
        std::shared_ptr<Component> spValue(pComponent);
        mComponents.insert( std::make_pair(name, spValue) );
    }

    std::shared_ptr<Element::Component> 
    Element::_getComponentImp (std::string name)
    {
        auto it = mComponents.find(name);
        if (it != mComponents.end())
            return it->second;
        else
            return std::shared_ptr<Component>();
    }

}}