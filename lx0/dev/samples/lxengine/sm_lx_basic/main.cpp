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
#include <lx0/object.hpp>
#include <lx0/mesh.hpp>
#include <lx0/view.hpp>
#include <lx0/controller.hpp>
#include <lx0/transaction.hpp>
#include <lx0/point3.hpp>
#include <lx0/jsonio.hpp>
#include <lx0/util.hpp>

using namespace lx0::core;

lxvar deserialize (const char* pszFilename)
{
    std::string s = lx0::util::lx_file_to_string(pszFilename);

    const char* p = s.c_str();
    lx0::serial::JsonParser parser;
    return parser.parse(p);
}

/*!
    @todo Eventually documents should be built from XML + JSON, not pure JSON.
 */
ElementPtr 
buildDocument (lxvar var)
{
    lx_check_error(var.isMap());
    
    ElementPtr spElem( new Element );

    auto type = var.find("type");
    spElem->type( type.asString() );
    
    auto attrs = var.find("attributes");
    for (auto it = attrs.beginMap(); it != attrs.endMap(); ++it)
    {
        spElem->attr( it->first.c_str(), it->second );
    }

    auto children = var.find("children");
    for (auto it = children.beginArray(); it != children.endArray(); ++it)
    {
        spElem->append( buildDocument(*it) );
    }

    auto value = var.find("value");
    if (value.isDefined())
    {
        // This should be controlled in a more dynamic, pluggable fashion
        if (spElem->type() == "Mesh") 
        {
            MeshPtr spMesh (new Mesh);
            spMesh->deserialize(value);
            spElem->value(spMesh);
        }
        else
            lx_error("No deserializer available for element of type '%s'", spElem->type().c_str());
    }

    return spElem;
}

int 
main (int argc, char** argv)
{
    int exitCode = -1;

    try
    {
        EnginePtr spEngine( Engine::acquire() );

        DocumentPtr spDocument(new Document);

        lxvar doc = deserialize("scene_000.json");
        lx_check_error(doc.isMap());
        ElementPtr spRoot = buildDocument(doc);
        spDocument->root(spRoot);
        
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

        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}