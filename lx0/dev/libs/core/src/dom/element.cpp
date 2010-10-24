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


#include <cassert>

#include <lx0/core.hpp>
#include <lx0/element.hpp>
#include <lx0/object.hpp>

namespace lx0 { namespace core {

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

        Element* pClone = new Element;
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

    void
    Element::value(ObjectPtr spValue)
    {
        mspValue = spValue;
    }

}}