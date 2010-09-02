#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/space.hpp>
#include <lx0/element.hpp>
#include <lx0/view.hpp>
#include <lx0/controller.hpp>
#include <lx0/transaction.hpp>


using namespace lx0::core;

int 
main (int argc, char** argv)
{
   EnginePtr spEngine( Engine::acquire() );

   std::cout << "Hello World!" << std::endl;

   DocumentPtr spDocument;
   SpacePtr spSpace;            // TODO: Should this class be deprecated?

   ElementPtr spElement(new Element);
   TransactionPtr spTransaction = spDocument->transaction();
   ElementPtr spRoot = spTransaction->write( spDocument->root() );
   spRoot->append(spElement);

   ViewPtr spView;
   ControllerPtr spController;
   
   return 0;
}
