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

#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/space.hpp>
#include <lx0/element.hpp>
#include <lx0/view.hpp>
#include <lx0/controller.hpp>
#include <lx0/transaction.hpp>

using namespace lx0::core;

ElementPtr 
create_cube()
{
    lxvar mesh;
    mesh.insert("type", "quad_list");

    const float p = 0.5f;
    lxvar vertices; 
    vertices.push( lxvar(-p, -p, -p) );  // 0 a
    vertices.push( lxvar(-p,  p, -p) );  // 1 b
    vertices.push( lxvar( p, -p, -p) );  // 2 c
    vertices.push( lxvar( p,  p, -p) );  // 3 d
    vertices.push( lxvar(-p, -p,  p) );  // 4 e
    vertices.push( lxvar(-p,  p,  p) );  // 5 f
    vertices.push( lxvar( p, -p,  p) );  // 6 g
    vertices.push( lxvar( p,  p,  p) );  // 7 h
    mesh.insert("vertices", vertices);

    lxvar faces;
    faces.push( lxvar(7, 6, 2, 3) ); // X+ 
    faces.push( lxvar(4, 5, 1, 0) ); // X- 
    faces.push( lxvar(5, 7, 3, 1) ); // Y+
    faces.push( lxvar(6, 4, 0, 2) ); // Y-
    faces.push( lxvar(5, 4, 6, 7) ); // Z+
    faces.push( lxvar(0, 1, 3, 2) ); // Z-
    mesh.insert("faces", faces);

    ElementPtr spElement(new Element);
    spElement->attr("id", "unit_cube");
    spElement->value(mesh);

    return spElement;
}

int 
main (int argc, char** argv)
{
    int exitCode = -1;

    try
    {
        EnginePtr spEngine( Engine::acquire() );

        DocumentPtr spDocument(new Document);

        ElementPtr spElement(new Element);
        TransactionPtr spTransaction = spDocument->transaction();
        ElementPtr spRoot = spTransaction->write( spDocument->root() );
        spRoot->append(spElement);

        {
            ElementPtr spLib(new Element);
            spLib->type("Library");
            spLib->append(create_cube());

            spRoot->append(spLib);
        }

        spEngine->connect(spDocument);

        {
            ViewPtr spView(new View);
            spDocument->connect("view", spView);
            spView->show();
        }


        ControllerPtr spController;
   
        exitCode = spEngine->run();

        // Demostrating that views *can* be detached by name.  This is not
        // necessary, as the Document releases all views on destruction 
        // automatically
        spDocument->disconnect("view");
    }
    catch (std::exception& e)
    {
        lx0::core::fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
