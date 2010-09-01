#include <iostream>
#include <string>

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
        // Define a helper lambda function that returns a function (this effectively 
        // acts as runtime template function).
        auto prefix_print = [](std::string prefix) -> std::function<void(const char*)> {
            return [prefix](const char* s) { std::cout << prefix << s << std::endl; };
        };
        slotDebug   = prefix_print("DBG: ");
        slotLog     = prefix_print("LOG: ");
        slotWarn    = prefix_print("WARN: ");
        slotError   = prefix_print("ERROR: ");
        slotFatal   = prefix_print("FATAL: ");

        log("lx::core::Engine ctor");
    }

    Engine::~Engine()
    {
       log("lx::core::Engine dtor");
    }

}}