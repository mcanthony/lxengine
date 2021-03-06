//===========================================================================//
/*
                                   LxEngine

    LICENSE

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
//   H E A D E R S
//===========================================================================//

#include <memory>

#include <glm/glm.hpp>

// The forward declarations require use of lx0::lxshared_ptr<>
#include <lx0/core/lxshared_ptr/lxshared_ptr.hpp>

//===========================================================================//
//  M A C R O S
//===========================================================================//

/*
    Note about the use of std::shared_ptr:

    std::shared_ptr is not as lightweight or efficient as could be desired,
    since it is non-intrusive and supports weak pointers.  However, it is
    a standard class, therefore it is used whenever the inefficiency is not
    a bottleneck.  Keep is simple.  Don't force developers to learn new 
    classes unless necessary.
 */

#define _LX_FORWARD_DECL_PTRS(Klass) \
    class Klass; \
    typedef std::shared_ptr<Klass> Klass ## Ptr; \
    typedef std::shared_ptr<const Klass> Klass ## CPtr; \
    typedef std::weak_ptr<Klass> Klass ## WPtr; \
    typedef std::weak_ptr<const Klass> Klass ## CWPtr; 

#define _LX_FORWARD_DECL_LXPTRS(Klass) \
    class Klass; \
    typedef lx0::lxshared_ptr<Klass> Klass ## Ptr; \
    typedef lx0::lxshared_ptr<const Klass> Klass ## CPtr; \

//===========================================================================//
//  F O R W A R D   D E C L A R A T I O N S 
//===========================================================================//

namespace lx0
{
    typedef glm::detail::int8   int8;       //!<  \ingroup lx0_core_types
    typedef glm::detail::int16  int16;      //!<  \ingroup lx0_core_types
    typedef glm::detail::int32  int32;      //!<  \ingroup lx0_core_types
    typedef glm::detail::int64  int64;      //!<  \ingroup lx0_core_types

    typedef glm::detail::uint8   uint8;     //!<  \ingroup lx0_core_types
    typedef glm::detail::uint16  uint16;    //!<  \ingroup lx0_core_types
    typedef glm::detail::uint32  uint32;    //!<  \ingroup lx0_core_types
    typedef glm::detail::uint64  uint64;    //!<  \ingroup lx0_core_types
}

namespace lx0
{
    namespace core
    {
    }

    namespace engine_ns
    {
        _LX_FORWARD_DECL_PTRS(Element);
        _LX_FORWARD_DECL_PTRS(Transaction);
        _LX_FORWARD_DECL_PTRS(Document);
        _LX_FORWARD_DECL_PTRS(Space);
        _LX_FORWARD_DECL_PTRS(Engine);
        _LX_FORWARD_DECL_PTRS(View);
        _LX_FORWARD_DECL_PTRS(UIBinding);
        _LX_FORWARD_DECL_PTRS(Controller);
        _LX_FORWARD_DECL_PTRS(Object);
        _LX_FORWARD_DECL_PTRS(LxVarObject);

        _LX_FORWARD_DECL_LXPTRS(Mesh);

        class DocumentComponent;
        class ElementComponent;
        class ViewComponent;
        class ViewImp;
    }
    
    using namespace lx0::engine_ns;
}


namespace lx0 { 

    namespace detail {
   
        //! Helper function for correctly making a Singleton of a class
        /*!
            This helper works by expecting that a single global, std::weak_ptr<> is used to track
            the singleton and std::shared_ptr<>'s are used to access the singleton.  The first
            shared_ptr<> acquisition results in creation of the Singleton.  The last release of
            the shared_ptr<> results in deletion of the Singleton.

            To use:
            
            1. Declare a protected/private constructor and destructor on the class.

            2. Declare a static member of type std::weak_ptr<> to point to the singleton.
            
            3. Declare and define a public acquire method on the class using the following syntax.
            This example assumes the class is called "MyClass" and the static member pointer has
            been declared as "static_singleton_weak_ptr":

            <pre>
template <typename T> friend std::shared_ptr<T> detail::acquireSingleton (std::weak_ptr<T>&);
static std::shared_ptr<MyClass> acquire() { return detail::acquireSingleton<MyClass>(static_singleton_weak_ptr); }
            </pre>

            @remark
            Note that there is no corresponding release() required, as shared_ptr<> is used to 
            implicity release the Singleton when the last reference is gone.

         */
        template <typename Klass>
        std::shared_ptr<Klass> 
        acquireSingleton (std::weak_ptr<Klass>& global_wptr)
        {
            // DeleteFunctor is used to expose access to the destructor to shared_ptr
            // but no one else.  
            // See: http://beta.boost.org/doc/libs/1_42_0/libs/smart_ptr/sp_techniques.html
            struct DeleteFunctor
            { 
                void operator()(Klass* p) { delete p; }
            }; 

            std::shared_ptr<Klass> sp( global_wptr.lock() );
            if (!sp.get())
            {
                sp.reset( new Klass, DeleteFunctor() );
                global_wptr = sp;
            }  
            return sp;
        }
    }
}

namespace Ogre
{
    //
    // Note: This "pollutes" the Ogre namespace with Lx typedefs.  This is not
    // ideal.  A better solution should be found for the future.
    //
    _LX_FORWARD_DECL_PTRS(Root);
    _LX_FORWARD_DECL_PTRS(RenderWindow);
    _LX_FORWARD_DECL_PTRS(SceneManager);
}
