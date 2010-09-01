#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>

#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/space.hpp>
#include <lx0/view.hpp>
#include <lx0/controller.hpp>

using namespace lx0::core;

int 
main (int argc, char** argv)
{
   std::shared_ptr<Engine> spEngine( Engine::acquire() );

   std::cout << "Hello World!" << std::endl;

   std::shared_ptr<Document> spDocument;
   std::shared_ptr<Space> spSpace;

   std::shared_ptr<View> spView;
   std::shared_ptr<Controller> spController;
   
   return 0;
}
