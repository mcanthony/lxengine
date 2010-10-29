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
#include <memory>
#include <string>
#include <functional>

namespace lx0 { namespace core {  namespace detail {

    class _ComponentBase : public std::enable_shared_from_this<_ComponentBase>
    {
    public:
        virtual ~_ComponentBase() {}
    };

    class _ComponentList
    {
    public:
        typedef _ComponentBase                      Component;
        typedef std::shared_ptr<Component>          ComponentPtr;
        typedef std::map<std::string, ComponentPtr> Map;

        ComponentPtr        attach  (std::string name, Component* pComponent);
        ComponentPtr        get     (std::string name);
        void                remove  (std::string name);

        Map::iterator       begin   (void) { return mMap.begin(); }
        Map::iterator       end     (void) { return mMap.end(); }

    protected:
            
        Map mMap;
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
        typedef             DerivedType     Derived;
        typedef             ComponentType   Component;
        typedef std::shared_ptr<Component>  ComponentPtr;

        ComponentPtr        attachComponent (std::string name, Component* pComp) { return std::dynamic_pointer_cast<Component>( mComponents.attach(name, pComp) ); }
        template <typename T>
        std::shared_ptr<T>  getComponent    (std::string name)                   { return std::dynamic_pointer_cast<T>( mComponents.get(name) ); }
        void                removeComponent (std::string name)                   { mComponents.remove(name); }

        template <typename T>
        std::shared_ptr<T>  ensureComponent (std::string name, std::function<T* (void)> ctor)
        {
            ComponentPtr spComponent = getComponent<Component>(name);
            if (!spComponent)
            {
                spComponent = attachComponent(name, ctor());
                static_cast<Derived*>(this)->notifyAttached(spComponent);
            }
            return std::dynamic_pointer_cast<T>(spComponent);
        }

    protected:
        typedef detail::_ComponentList      ComponentList;
            

        void                _foreach        (std::function<void(ComponentPtr)> f)
        {
            for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
                f( std::dynamic_pointer_cast<Component>(it->second) );
        }

    private:
        ComponentList       mComponents;
    };
   
}}}

