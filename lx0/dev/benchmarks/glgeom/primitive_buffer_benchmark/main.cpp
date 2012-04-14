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
#include <glgeom/extension/mappers.hpp>

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

struct triangle3f_b
{
    glgeom::point3f     position[3];
    glgeom::vector3f    normal[3];
    glgeom::point2f     uv[8][3];
};

static void
iterate_triangles (glgeom::primitive_buffer& primitive, std::function<void(triangle3f_b&)> f)
{
    if (primitive.type != "triangles")
        glgeom::error("Valid only for triangle based meshes!");

    triangle3f_b triangle;
    
    size_t indexCount = primitive.indices.size();
    for (size_t i = 0; i < indexCount; i += 3)
    {
        for (size_t j = 0; j < 3; ++j)
        {
            size_t k = primitive.indices[i + j];

            triangle.position[j] = primitive.vertex.positions[k];
            //triangle.normal[j] = primitive.vertex.normals[i + j];
            triangle.uv[0][j] = primitive.vertex.uv[0][k];
        }

        f(triangle);
    }
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

/*
    Compute the tangent and bitangent vectors. 

    T = the tangent vector: in object space, aligned to the direction of most increasing U
    B = the bitangent vector: in object space, aligned to the direction of most increating V 
 */
static void 
test2 (glgeom::primitive_buffer& primitive)
{    
    using namespace glgeom;

    std::vector<vector3f> tangents;
    std::vector<vector3f> bitangents;

    iterate_triangles(primitive, [](triangle3f_b& triangle) {

        using namespace glgeom;

        //
        // Compute the deltas: i.e. how the UV changes relative to position
        // changes in object coordinates.
        // 
        vector3f dP0 = triangle.position[1] - triangle.position[0];
        vector3f dP1 = triangle.position[2] - triangle.position[1];

        vector2f dUV0 = triangle.uv[0][1] - triangle.uv[0][0];
        vector2f dUV1 = triangle.uv[0][2] - triangle.uv[0][1];

        //
        //
        //
        const float r = 1.0f / (dUV0.x * dUV1.y - dUV0.y * dUV1.x);
        glgeom::vector3f tangent    = sub(dP0 * dUV1.y, dP1 * dUV0.y) * r;
        glgeom::vector3f bitangent  = sub(dP1 * dUV0.x, dP0 * dUV1.x) * r;

    });

}

static void 
test3 (glgeom::primitive_buffer& primitive)
{    
    using namespace glgeom;

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

            multi_test("compute_tangent_vectors",
                [&]() { 
                    primitive = original; 
                    glgeom::compute_uv_mapping(primitive, 0, [](const glgeom::point3f& p, const glgeom::vector3f& n) -> glgeom::point2f {
                         return glgeom::scale( glgeom::mapper_planar_xy(p), glgeom::vector2f(10, 10));
                    });
                },
                [&]() { test3(primitive); }, 
                [](){}
            );
        }
    }
    spEngine->shutdown();
    return 0;
}
