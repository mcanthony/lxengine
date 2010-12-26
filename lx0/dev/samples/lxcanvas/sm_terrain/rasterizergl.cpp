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

#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>

#include "rasterizergl.hpp"


void RasterizerGL::initialize()
{
    // Initialization
    //
    lx_log("Using OpenGL v%s", (const char*)glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);
}

void RasterizerGL::shutdown()
{
}


RasterizerGL::CameraPtr       
RasterizerGL::createCamera (float fov, float nearDist, float farDist, matrix4& viewMatrix)
{
    CameraPtr spCamera (new Camera);
    spCamera->fov = fov;
    spCamera->nearDist = nearDist;
    spCamera->farDist = farDist;
    spCamera->viewMatrix = viewMatrix;
    return spCamera;
}

void
RasterizerGL::Camera::activate()
{
    //
    // Setup projection matrix
    //
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    int vp[4];  // x, y, width, height
    glGetIntegerv(GL_VIEWPORT, vp);
    GLfloat aspectRatio = (GLfloat)vp[2]/(GLfloat)vp[3];
    gluPerspective(fov, aspectRatio, nearDist, farDist);

    //
    // Setup view matrix
    //
    // OpenGL matrices are laid out with rows contiguous in memory (i.e. data[1] in 
    // the 16 element array is the second element in the first row); matrix4 uses
    // column-major ordering.  Therefore load it as a tranpose to swap the order.
    //
    // Note: it's also not necessarily faster to send the non-transpose, as the
    // OpenGL API does not necessarily match the underlying hardware representation.
    //
    glMatrixMode(GL_MODELVIEW);
    glLoadTransposeMatrixf(viewMatrix.data);
}

RasterizerGL::MaterialPtr 
RasterizerGL::createMaterial (void)
{
    // Create the shader program
    //
    GLuint vs = createShader("media/shaders/glsl/vertex/basic_00.vert", GL_VERTEX_SHADER);
    GLuint gs = createShader("media/shaders/glsl/geometry/basic_00.geom", GL_GEOMETRY_SHADER);
    GLuint fs = createShader("media2/shaders/glsl/fragment/checker_world_xy10.frag", GL_FRAGMENT_SHADER);

    GLuint prog = glCreateProgram();
    {
        glAttachShader(prog, vs);
        glAttachShader(prog, gs);
        glAttachShader(prog, fs);
        
        // OpenGL 3.2 spec states that geometry shaders must output strips for
        // lines and triangles.
        if (gs)
        {
            GLenum geometryInput = GL_TRIANGLES;
            GLenum geometryOutput = GL_TRIANGLE_STRIP;
            int maxPrimitives = 3;
                
            glProgramParameteriEXT(prog, GL_GEOMETRY_INPUT_TYPE_EXT,    geometryInput);
            glProgramParameteriEXT(prog, GL_GEOMETRY_OUTPUT_TYPE_EXT,   geometryOutput);
            glProgramParameteriEXT(prog, GL_GEOMETRY_VERTICES_OUT_EXT,  maxPrimitives);
        }
            
        glBindAttribLocation(prog, 0, "inPosition");
    }

    glLinkProgram(prog);
    glUseProgram(prog);

    return MaterialPtr(new Material);
}

GLuint 
RasterizerGL::createShader( char* filename, GLuint type)
{
    GLuint shaderHandle = 0; 

    std::string shaderText = lx0::util::lx_file_to_string(filename);
    if (!shaderText.empty())
    {
        shaderHandle = glCreateShader(type);

        const GLchar* text = shaderText.c_str();
        glShaderSource(shaderHandle, 1, &text, 0);
        shaderText.swap(std::string());

        glCompileShader(shaderHandle);
    }
    else
        lx_error("Could not load shader '%s' (file exists = %s)", filename, lx0::util::lx_file_exists(filename) ? "true" : "false");

    return shaderHandle;
}


RasterizerGL::GeometryPtr 
RasterizerGL::createQuadList (std::vector<point3>& positionData)
{
    GLuint vao[1];

    // Create a vertex array to store the vertex data
    //
    glGenVertexArrays(1, &vao[0]);
    glBindVertexArray(vao[0]);

    lx_check_error( glGetError() == GL_NO_ERROR, "OpenGL error detected." );

    GLuint vbo[1];
    glGenBuffers(1, &vbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * positionData.size(), &positionData[0], GL_STATIC_DRAW);
        
    // TODO: The position input to the vertex shader is apparently always 0.  Find out where
    // this is documented.
    const int positionIndex = 0;   // glGetAttribLocation(prog, "gl_Vertex");
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionIndex);
    lx_check_error( glGetError() == GL_NO_ERROR, "OpenGL error detected." );

    auto pQuadList = new RasterizerGL::QuadList;
    pQuadList->vao[0] = vao[0];
    pQuadList->size = positionData.size();
    return GeometryPtr(pQuadList);
}

void RasterizerGL::QuadList::activate()
{
    glBindVertexArray(vao[0]);

    glDrawArrays(GL_QUADS, 0, size); 
}

RasterizerGL::LightSetPtr     
RasterizerGL::createLightSet (void)
{
    return LightSetPtr(new LightSet);
}

RasterizerGL::TransformPtr 
RasterizerGL::createTransform (matrix4& mat)
{
    return TransformPtr(new Transform);
}

void RasterizerGL::beginScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RasterizerGL::endScene()
{
}

void RasterizerGL::rasterize(std::shared_ptr<Item> spItem)
{
    spItem->rasterize();
}

void 
RasterizerGL::Item::rasterize()
{
    spCamera->activate();
    spLightSet->activate();
    spMaterial->activate();
    spTransform->activate();
    spGeometry->activate();     
}