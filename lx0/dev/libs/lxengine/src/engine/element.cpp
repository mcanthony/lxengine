//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

#include <algorithm>

#include <lx0/lxengine.hpp>
#include <lx0/engine/element.hpp>
#include <lx0/engine/object.hpp>
#include <lx0/engine/document.hpp>

namespace lx0 { namespace engine { namespace dom_ns {

    //===========================================================================//

    Element::FunctionMap Element::s_funcMap;
    
    //---------------------------------------------------------------------------//

    Element::Element (void)
        : mpDocument (nullptr)
        , mFlags     (0)
    {
    }

    //---------------------------------------------------------------------------//

    Element::~Element ()
    {
        lx_assert(mpDocument == nullptr, "Element being deleted whilst actively in a Document");

        mComponents.clear();
    }

    //---------------------------------------------------------------------------//

    void    
    Element::prepend (ElementPtr spElem)
    {
        lx_check_error(this != nullptr);
        lx_check_error(spElem->parent().get() == nullptr);
       
        spElem->mspParent = shared_from_this();
        mChildren.push_front(spElem);

        if (mpDocument)
            spElem->notifyAdded(mpDocument);
    }

    //---------------------------------------------------------------------------//

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

    //---------------------------------------------------------------------------//
    /*!
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

    //---------------------------------------------------------------------------//

    ElementPtr     
    Element::parent()
    {
        return mspParent;
    }

    //---------------------------------------------------------------------------//
    
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

    //---------------------------------------------------------------------------//

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

    //---------------------------------------------------------------------------//

    int
    Element::childCount (void) const
    {
        return int(mChildren.size());
    }

    //---------------------------------------------------------------------------//

    void
    Element::removeChild (ElementPtr spElem)
    {
        lx_check_error(spElem->parent().get() == this);

        auto it = std::find(mChildren.begin(), mChildren.end(), spElem);
        if (it != mChildren.end())
        {
            mChildren.erase(it);
            spElem->mspParent.reset();

            if (mpDocument)
                spElem->notifyRemoved(mpDocument);
        }
        else
            lx_warn("Trying to remove an element that is not a child of this element");
    }

    //---------------------------------------------------------------------------//

    void
    Element::removeAll (void)
    {
        while (!mChildren.empty())
        {
            auto spChild = mChildren.back();
            spChild->removeAll();
         
            mChildren.pop_back();
            spChild->mspParent.reset();

            if (mpDocument)
                spChild->notifyRemoved(mpDocument);
        }
    }


    //---------------------------------------------------------------------------//

    /*!
        Creates a shallow clone of the Element.

        WARNING: This method is a bit dangerous as the "shallow" clone actually
        contains a shared reference to the same attributes, parent, and children
        as the original object.  This could easily be used to create a corrupt
        tree structure as the shared reference modified in one object affects the
        other.  A shallow clone does not necessarily make a lot of sense in the
        Element class.
     */ 
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

    //---------------------------------------------------------------------------//

    ElementPtr
    Element::cloneDeep (void) const
    {
        Element* pClone = new Element;
        ElementPtr spClone(pClone);
        
        pClone->mTagName = mTagName;
        pClone->mpDocument = nullptr;
        pClone->mspParent = nullptr;    // Create a detached clone
        
        for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
            pClone->mAttributes.insert( std::make_pair(it->first, it->second.clone()) );
        
        pClone->mValue = mValue.clone();
        
        for (auto it = mChildren.begin(); it != mChildren.end(); ++it)
        {
            auto spChild = (*it)->cloneDeep();
            spChild->mspParent = spClone;
            pClone->mChildren.push_back(spChild);
        }
        
        return spClone;
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
        notifyValueChanged();

        mValue = value;
    }

    void
    Element::notifyValueChanged (void)
    {
        _foreach([&](ComponentPtr it) {
            it->onValueChange(shared_from_this());
        });
    }
    
    void
    Element::notifyValueChanged (const char* selector)
    {
        ///@todo Implement notifyValueChanged(selector)
        /*
            Provide a finer grain notification of what about the value has changed.
            The value is likely a map of some sort, therefore this should describe
            which key or keys has changed.   

            A good, consistent, but easy to parse selector syntax should be chosen.

            pElem->value()["position"] = lxvar::wrap(glgeom::point3f(1, 2, 3));
            pElem->notifyValueChanged("position");
         */
        lx_error("Not implemented");
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

        pDocument->notifyElementAdded(shared_from_this());

        _foreach([](ComponentPtr it) {
            it->onAdded();
        });

        for (auto it = mChildren.begin(); it != mChildren.end(); ++it)
            (*it)->notifyAdded(pDocument);
    }

    /*!
        Dev Notes:
        Is this correctly named?  This seems to both remove the Element from the host
        Document as well as send out the notification of the removal to the Components
        and children.
     */
    void
    Element::notifyRemoved (Document* pDocument)
    {
        lx_check_error(mpDocument == pDocument, 
            "Element notified that it is being removed from a Document that it did not belong to.");

        _setHostDocument(nullptr);

        pDocument->notifyElementRemoved(shared_from_this());

        for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
        {
            auto spComponent = it->second;
            spComponent->onRemoved();
        }

        for (auto it = mChildren.begin(); it != mChildren.end(); ++it)
            (*it)->notifyRemoved(pDocument);

        lx_check_error(mpDocument == nullptr);
    }

    void
    Element::notifyUpdate (Document* pDocument)
    {
        //
        // Track flags on whether any components override the onUpdate() method.
        // This way in the common case that no components override the method,
        // an early exit is possible.
        //
        if (mFlags & eCallUpdate)
        {
            _foreach([&](ComponentPtr it) {
                it->onUpdate(this->shared_from_this());
            });
        }
    }

    void
    Element::recomputeFlags (void)
    {
        lx0::uint32 elemFlags = 0;

        for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
        {
            auto pComponent = it->second.get();
            lx0::uint32 compFlags = pComponent->flags();

            if (compFlags & ElementComponent::eCallUpdate)
                elemFlags |= Element::eCallUpdate;
            else
            {
                if (!(compFlags & ElementComponent::eSkipUpdate))
                    lx_error("Component needs to explicitly return either eSkipUpdate or eCallUpdate from flags() implementation!");
            }
        }

        mFlags = elemFlags;

        document()->notifyFlagsModified(this);
    }

    void
    Element::notifyAttached  (ComponentPtr spComponent)
    {
        // The new component may affect the cached flags
        recomputeFlags();
    }

    DocumentPtr
    Element::document (void)
    { 
        lx_check_error(this != nullptr);

#ifdef _DEBUG
        // If there's a parent node, it should be in the same document unless somehow the
        // data structure has been corrupted.
        if (mspParent)
        {
            lx_check_error(mspParent->mpDocument == mpDocument);
        }
#endif
        if (mpDocument)
            return mpDocument->shared_from_this();
        else
            return DocumentPtr();
    }

    //! Dynamically add a named function on the Element
    void     
    Element::addFunction (std::string name, Element::Function func)
    {
        s_funcMap.insert(std::make_pair(name, func));
    }

    void
    Element::getFunctions (std::vector<std::string>& names)
    {
        names.reserve( s_funcMap.size() );
        for (auto it = s_funcMap.begin(); it != s_funcMap.end(); ++it)
            names.push_back(it->first);
    }

    //! Dynamically add a named function on the Element
    void     
    Element::addCallback(std::string name, Element::Function func)
    {
        mCallbackMap[name] += func;
    }

    void
    Element::call (std::string name)
    {
        std::vector<lxvar> args(0);
        call(name, args);
    }

    void
    Element::call (std::string name, lxvar a0)
    {
        std::vector<lxvar> args(1);
        args[0] = a0;
        call(name, args);
    }

    void
    Element::call (std::string name, lxvar a0, lxvar a1)
    {
        std::vector<lxvar> args(2);
        args[0] = a0;
        args[1] = a1;
        call(name, args);
    }

    void
    Element::call (std::string name, lxvar a0, lxvar a1, lxvar a2)
    {
        std::vector<lxvar> args(3);
        args[0] = a0;
        args[1] = a1;
        args[2] = a2;
        call(name, args);
    }

    void
    Element::call (std::string name, std::vector<lxvar>& args)
    {
        ElementPtr spElem = shared_from_this();

        auto jt = s_funcMap.find(name);
        if (jt != s_funcMap.end())
            jt->second(spElem, args);

        auto it = mCallbackMap.find(name);
        if (it != mCallbackMap.end())
            it->second(spElem, args);
    }

}}}
