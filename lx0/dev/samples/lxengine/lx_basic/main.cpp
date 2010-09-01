#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>

#include <lx0/engine.hpp>

using namespace lx0::core;

int 
main (int argc, char** argv)
{
   std::shared_ptr<Engine> spEngine( Engine::acquire() );

   std::cout << "Hello World!" << std::endl;
   
   return 0;
}
