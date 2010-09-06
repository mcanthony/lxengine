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

#include <memory>
#include <deque>
#include <string>

#include <lx0/detail/forward_decls.hpp>

namespace lx0 { namespace core {

    namespace detail {

        _LX_FORWARD_DECL_PTRS(OgreSubsystem);
    };

//===========================================================================//
//!
/*!
 */
class Engine
{
public:
   static std::shared_ptr<Engine> acquire();

   void   sendMessage (const char* message);
   int	  run();

protected:
   static std::weak_ptr<Engine> s_wpEngine;

   Engine();
   ~Engine();

   
   // DeleteFunctor is used to expose access to the destructor to shared_ptr
   // but no one else.  
   // See: http://beta.boost.org/doc/libs/1_42_0/libs/smart_ptr/sp_techniques.html
   struct DeleteFunctor
   { 
      void operator()(Engine* p) { delete p; }
   };   

   detail::OgreSubsystemPtr m_spOgre;

   std::deque<std::string>  m_messageQueue;
};

}}
