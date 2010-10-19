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

using namespace lx0::core;

// Deserialize the lxvar representation into the run-time object
MeshPtr create_mesh (lxvar v)
{
    lx_check_error(v.isMap());
    lx_check_error(v.containsKey("type"));
    lx_check_error(v.containsKey("vertices"));
    lx_check_error(v.containsKey("faces"));

    // Temporary limitation
    lx_check_error(v.find("type").equals("quad_list"));

    MeshPtr spMesh (new Mesh);
    
    {
        lxvar lxverts = v.find("vertices");
        const int vertexCount = lxverts.size();
        spMesh->mVertices.reserve(vertexCount);
        for (int i = 0; i < vertexCount; ++i)
        {
            spMesh->mVertices.push_back( asPoint3(lxverts.at(i)) );
        }
    }

    {
        lxvar lxfaces = v.find("faces");
        const int faceCount = lxfaces.size();
        spMesh->mFaces.reserve(faceCount);
        for (int i = 0; i < faceCount; ++i)
        {
            Mesh::Quad q;
            for (int j = 0; j < 4; ++j)
                q.index[j] = lxfaces.at(i).at(j).asInt();
            spMesh->mFaces.push_back(q);
        }
    }

    lx_check_error(spMesh);
    return spMesh;
}

lxvar deserialize (const char* pszFilename)
{
    std::string s;
    FILE* fp;
    fopen_s(&fp, pszFilename, "r");

    lx_check_error(fp != nullptr);

    char szString[4096];
    while (fgets(szString, 4096, fp) != NULL) {
        s += std::string(szString);
    }
    fclose(fp);

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
            spElem->value( create_mesh(value) );
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

        /*
            @todo Need to embed a JS script to generate the grid rather than
                hardcode it into the data file

                for (int gy = 0; gy < 3; gy ++)
                {
                    for (int gx = 0; gx < 3; gx ++)
                    {
                        // g2w = grid to world coordinate
                        auto g2w = [](int i) { return (i - 1) * 1.5f; };
                        lxvar pos;
                        pos.push(g2w(gx));
                        pos.push(g2w(gy));
                        pos.push(0.5f);

                        ElementPtr spRef (new Element);
                        spRef->type("Ref");
                        spRef->attr("translation", pos);
                        spRef->attr("ref", "unit_cube");

                        spScene->append(spRef);
                    }
        */

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
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
