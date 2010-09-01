#pragma once

#include <memory>

#include <lx0/detail/forward_decls.hpp>

namespace lx0 { namespace core {

//===========================================================================//
//!
/*!
 */
class Engine
{
public:
   static std::shared_ptr<Engine> acquire();

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
};

}}
