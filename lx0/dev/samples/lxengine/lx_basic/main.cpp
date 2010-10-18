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
#include <lx0/view.hpp>
#include <lx0/controller.hpp>
#include <lx0/transaction.hpp>
#include <lx0/point3.hpp>

using namespace lx0::core;

// Create the serialized lxvar representation
lxvar 
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

    return mesh;
}

// WIP object - assumes quad lists
class Mesh : public lx0::core::Object
{
public:
    struct Quad
    {
        int index[4];
    };

    std::vector<point3> mVertices;
    std::vector<Quad>   mFaces;
};

_LX_FORWARD_DECL_PTRS(Mesh);

point3 asPoint3 (const lxvar& lx)
{
    lx_check_error(lx.isArray());
    lx_check_error(lx.size() == 3);

    point3 p;
    p.x = lx.at(0).asFloat();
    p.y = lx.at(1).asFloat();
    p.z = lx.at(2).asFloat();
    return p;
}

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

    return spMesh;
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
        //TransactionPtr spTransaction = spDocument->transaction();
        //ElementPtr spRoot = spTransaction->write( spDocument->root() );
        ElementPtr spRoot = spDocument->root();
        spRoot->append(spElement);

        {
            ElementPtr spLib(new Element);
            spLib->type("Library");
            {
                ElementPtr spElement(new Element);
                spElement->type("Mesh");
                spElement->attr("id", "unit_cube");
                spElement->value(create_mesh(create_cube()));
                spLib->append(spElement);
            }    
            spRoot->append(spLib);
        }
        {
            ElementPtr spScene(new Element);
            spScene->type("Scene");
            {
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
                    }
                }
            }
            spRoot->append(spScene);
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
