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

#include <algorithm>

#include <lx0/core.hpp>
#include <lx0/element.hpp>
#include <lx0/object.hpp>
#include <lx0/document.hpp>

namespace lx0 { namespace core {

    Element::Element (void)
        : mpDocument (nullptr)
    {
    }

    Element::~Element ()
    {
        lx_assert(mpDocument == nullptr, "Element being deleted whilst actively in a Document");
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

        if (mpDocument)
            spElem->notifyAdded(mpDocument);
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

    ElementPtr     
    Element::parent()
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

    void
    Element::removeChild (ElementPtr spElem)
    {
        auto it = std::find(mChildren.begin(), mChildren.end(), spElem);
        if (it != mChildren.end())
        {
            mChildren.erase(it);

            if (mpDocument)
                spElem->notifyRemoved(mpDocument);
        }
        else
            lx_warn("Trying to remove an element that is not a child of this element");
    }

    ElementPtr      
    Element::_clone () const
    {
        if (this == 0)
        {
            lx_error("Cloning a null pointer!");
            return ElementPtr();
        }

        Element* pClone = new Element;
        pClone->mpDocument = nullptr;           // mpDocument is NOT copied!  The clone is not part of the document
        pClone->mAttributes = mAttributes;
        pClone->mspParent = mspParent;
        pClone->mChildren = mChildren;
        pClone->mValue = mValue.clone();
        return ElementPtr(pClone);
    }

    void
    Element::attr(std::string name, lxvar value)
    {
        lx_check_error( this != nullptr );

        _foreach([&](ComponentPtr it) {
            it->onAttributeChange(shared_from_this(), name, value);
        });

        mAttributes[name] = value;
    }

    lxvar     
    Element::attr (std::string name) const
    {
        lx_check_error( this != nullptr );

        auto it = mAttributes.find(name);
        if (it != mAttributes.end())
            return it->second;
        else
            return lxvar();
    }

    void
    Element::value(lxvar value)
    {
        _foreach([&](ComponentPtr it) {
            it->onValueChange(shared_from_this(), value);
        });

        mValue = value;
    }

    void
    Element::_setHostDocument (Document* pDocument)
    {
        if (pDocument)
        {
            lx_check_error(mpDocument == nullptr);
            mpDocument = pDocument;
        }
        else
        {
            lx_check_error(mpDocument != nullptr);
            mpDocument = nullptr;
        }
    }

    void
    Element::notifyAdded (Document* pDocument)
    {        
        if (mpDocument == pDocument)
            lx_error("Element already added to this Document.");
        lx_check_error(mpDocument == nullptr, 
            "Element notified that it being added to a Document, but already belongs to a Document");

        _setHostDocument(pDocument);

        _foreach([](ComponentPtr it) {
            it->onAdded();
        });

        pDocument->notifyElementAdded(shared_from_this());

        for (auto it = mChildren.begin(); it != mChildren.end(); ++it)
            (*it)->notifyAdded(pDocument);
    }

    void
    Element::notifyRemoved (Document* pDocument)
    {
        lx_check_error(mpDocument == pDocument, 
            "Element notified that it being removed from a Document that it did not belong to.");

        _setHostDocument(nullptr);

        _foreach([](ComponentPtr it) {
            it->onRemoved();
        });

        pDocument->notifyElementRemoved(shared_from_this());

        for (auto it = mChildren.begin(); it != mChildren.end(); ++it)
            (*it)->notifyRemoved(pDocument);

        lx_check_error(mpDocument == nullptr);
    }

    DocumentPtr
    Element::document (void)
    { 
        return mpDocument->shared_from_this(); 
    }

    void 
    Element::addImpulse (const vector3& v)
    {
        _foreach([&](ComponentPtr it) {
            it->addImpulse(v);
        });
    }

}}
