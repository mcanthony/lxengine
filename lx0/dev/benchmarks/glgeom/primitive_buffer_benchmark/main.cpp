//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2012 athile@athile.net (http://www.athile.net)

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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

// Standard headers
#include <vector>
#include <iostream>

#include <lx0/lxengine.hpp>
#include <lx0/util/blendload.hpp>
#include <glgeom/extension/primitive_buffer.hpp>

int g_innerCount = 4;

static void
multi_test(std::string name, std::function<void()> e, std::function<void()> f, std::function<void()> g)
{
    lx0::Timer timer;
    for (int i = 0; i < g_innerCount; ++i)
    {
        e();
        timer.start();
        f();
        timer.stop();
        g();
    }
    lx_message("%-40s :: %5ums", name, timer.totalMs());
}

static void 
test2 (glgeom::primitive_buffer& primitive)
{       
    const size_t faceSize = primitive.type == "quads" ? 4 : 3;
    std::vector<glgeom::uint16> arrBase( (primitive.indices.size() / faceSize) * 8 );
    glgeom::uint16* arr = &arrBase[0];
    ::memset(arr, 0, arrBase.size() * sizeof(glgeom::uint16));

    std::map<glgeom::uint16,std::vector<glgeom::uint16>> extra;

    glgeom::iterate_indices(primitive, [faceSize,arr,&extra](size_t faceIndex, glgeom::uint16* vertexIndices) {
        for (size_t i = 0; i < faceSize; ++i)
        {
            auto vi = vertexIndices[i]; 
            auto ptr = &arr[faceIndex * 8];
            auto& count = *ptr;
            ptr += count;
                            
            if (count < 7)
                *ptr = vi; 
            else
                extra[faceIndex].push_back(vi);
            ++count;                            
        }
    });
}

static glgeom::primitive_buffer
load_data()
{
    const std::string filename = "common/models/landscapes/mountain_valley_b-000.blend";

    lx_message("Loading Blender model '%1%'", filename);
    glgeom::primitive_buffer primitive;
    
    glm::mat4 scaleMat = glm::scale(glm::mat4(), glm::vec3(1, 1, 1));
    lx0::primitive_buffer_from_blendfile(primitive, filename.c_str(), scaleMat);

    return primitive;
}

int 
main (int argc, char** argv)
{
#ifdef NDEBUG
    g_innerCount = 8;
#else
    g_innerCount = 2;

    if (lx0::lx_in_debugger())
        g_innerCount = 1;
#endif
 
    lx0::EnginePtr spEngine = lx0::Engine::acquire();
    spEngine->initialize();   
    {
        glgeom::primitive_buffer primitive = load_data();  
        glgeom::primitive_buffer original = primitive;
        
        for (int i = 0; i < 4; ++i)
        {
            lx_message("=== Iteration %1% ===", i);

            multi_test("compute_face_normals",
                [&]() { primitive = original; },
                [&]() { glgeom::compute_face_normals(primitive); }, 
                [](){}
            );
            multi_test("compute_vertex_normals",
                [&]() { primitive = original; },
                [&]() { glgeom::compute_face_normals(primitive); }, 
                [](){}
            );
            multi_test("compute_adjacency_vertex_to_faces", 
                [&]() { primitive = original; },
                [&]() { glgeom::compute_adjacency_vertex_to_faces(primitive); }, 
                [&]() { glgeom::verify_adjacency_vertex_to_faces(primitive, true); } 
            );

            multi_test("create_vertex_normals_mesh",
                [&]() { primitive = original; },
                [&]() { create_vertex_normals_mesh(primitive); }, 
                [](){}
            );
            multi_test("create_face_normals_mesh",
                [&]() { primitive = original; },
                [&]() { create_vertex_normals_mesh(primitive); }, 
                [](){}
            );


            multi_test("custom vertex_to_faces",
                [&]() { primitive = original; },
                [&]() { test2(primitive); }, 
                [](){}
            );
        }
    }
    spEngine->shutdown();
    return 0;
}
