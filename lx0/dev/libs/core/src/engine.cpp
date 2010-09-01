#include <iostream>

#include <lx0/core.hpp>
#include <lx0/engine.hpp>

namespace lx0 { namespace core {

std::weak_ptr<Engine> Engine::s_wpEngine;

std::shared_ptr<Engine>
Engine::acquire()
{
   std::shared_ptr<Engine> sp( s_wpEngine.lock() );
   if (!sp.get())
   {
      sp.reset( new Engine, DeleteFunctor() );
      s_wpEngine = sp;
   }  
   return sp;
}

Engine::Engine()
{
   log("LOG: lx::core::Engine ctor");
}

Engine::~Engine()
{
   log("LOG: lx::core::Engine dtor");
}

}}