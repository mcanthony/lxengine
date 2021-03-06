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

#pragma once

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

// Standard headers
#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <functional>

#include <lx0/core/log/log.hpp>

namespace lx0 { namespace engine_ns {  namespace detail {

    class _ComponentBase : public std::enable_shared_from_this<_ComponentBase>
    {
    public:
        virtual ~_ComponentBase() {}

        virtual const char*     name() const { return nullptr; }
    };

    /*!
        Example:
        auto spPhysics = spElem->getComponent<PhysicsComponent>("physics");


        Developer Notes:

        This class is a mixed bag: it consolidates a chunk of shared code between
        the DOM classes, but at the same time is messy in its use of templates
        and its interaction with two derived types.  For now, "it works" but it
        would be good to revisit this to see if the common DOM code can be 
        reshared in a more straightforward manner so that the code remains
        maintainable.
    */
    template <typename DerivedType, typename ComponentType>
    class _EnableComponentList
    {
    public:
        typedef             DerivedType             Derived;
        typedef             ComponentType           Component;
        typedef std::shared_ptr<Component>          ComponentPtr;
        typedef std::map<std::string, ComponentPtr> Map;

        ComponentPtr attachComponent (Component* pComponent) 
        { 
            ComponentPtr spValue(pComponent);
            return attachComponent(spValue);
        }

        ComponentPtr attachComponent (ComponentPtr spValue) 
        { 
            const char* slotName = spValue->name();

            //
            // If no name is assigned to the component, it will never be accessible
            // (which is fine in many cases), but still an random name is generated
            // to avoid map collisions.
            //
            if (slotName == nullptr)
                slotName = _generateUniqueSlotName();

            if (!mComponents.insert( std::make_pair(slotName, spValue) ).second)
            {
#ifndef NDEBUG
                auto spExisting = mComponents.find(slotName);
                assert(!"Existing component already in the slot by that name");
#endif
            }

            // Have the host object send out notification that the Component
            // was attached
            static_cast<Derived*>(this)->notifyAttached(spValue);
            
            return spValue;
        }
        
        
        template <typename T>
        std::shared_ptr<T>  getComponent    (std::string name)                   
        { 
            auto it = mComponents.find(name);
            if (it != mComponents.end())
                return std::dynamic_pointer_cast<T>(it->second);
            else
                return std::shared_ptr<T>();
        }

        template <typename T>
        std::shared_ptr<T>  getComponent    (void)                   
        { 
            return getComponent<T>( T::s_name() );
        }

        void removeComponent (std::string name)
        {
            auto it = mComponents.find(name);
            if (it != mComponents.end())
                mComponents.erase(it);
            else
                throw lx_error_exception("Attempt to remove a Component '%s' that is not attached.", name.c_str());
        }

        void removeComponent (Component* pComponent)
        {
            for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
            {
                if (it->second.get() == pComponent)
                {
                    mComponents.erase(it);
                    return;
                }
            }

            throw lx_error_exception("Attempt to remove a Component that is not attached.");
        }

        void clearComponents (void)
        {
            _clearComponents();
        }

        template <typename T>
        std::shared_ptr<T>  ensureComponent (std::string name, std::function<T* (void)> ctor)
        {
            ComponentPtr spComponent = getComponent<Component>(name);
            if (!spComponent)
            {
                // Ensure the constructor is actually creating an object for the slot being
                // tested
                auto pValue = ctor();
                assert(name == pValue->name());

                spComponent = attachComponent(pValue);
            }
            return std::dynamic_pointer_cast<T>(spComponent);
        }

        void foreachComponent (std::function<void(ComponentPtr)> f) { _foreach(f); }

    protected:         

        void _foreach        (std::function<void(ComponentPtr)> f)
        {
            // Structure the loop so that the iterator is advanced before the callback 
            // is called.  This allows the callback to remove itself from the ComponentList
            // safely (see http://www.sgi.com/tech/stl/Map.html, std::map::erase() only
            // invalidates the current iterator).
            //
            auto it = mComponents.begin(); 
            while (it != mComponents.end()) 
            {
                auto spCurrent = it->second;
                ++it;
                f(spCurrent);
            }
        }

        void _clearComponents ()
        {
            mComponents.clear();
        }

        Map mComponents;

    private:
        const char* _generateUniqueSlotName ()
        {
            static unsigned int count = 0;
            static char buffer[32];

            unsigned int num = ++count;
            int i = 0;
            buffer[i++] = '_';
            buffer[i++] = '_';
            buffer[i++] = '_';
            while (num > 0)
            {
                buffer[i++] = '0' + num % 10;
                num /= 10;
            }
            buffer[i++] = '_';
            buffer[i++] = '_';
            buffer[i++] = '_';
            return buffer;
        }
    };
   
}}}

