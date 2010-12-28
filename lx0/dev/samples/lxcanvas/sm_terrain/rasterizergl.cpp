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

using namespace Rasterizer;

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
    GLuint vs = _createShader("media2/shaders/glsl/vertex/basic_01.vert", GL_VERTEX_SHADER);
    GLuint gs = _createShader("media2/shaders/glsl/geometry/basic_01.geom", GL_GEOMETRY_SHADER);
    GLuint fs = _createShader("media2/shaders/glsl/fragment/checker_world_xy10.frag", GL_FRAGMENT_SHADER);

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

    _linkProgram(prog);
    glUseProgram(prog);

    return MaterialPtr(new Material);
}

void 
RasterizerGL::_linkProgram (GLuint prog)
{
    lx_check_error(glGetError() == GL_NO_ERROR, "GL error status detected!");

    glLinkProgram(prog);

    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);    
    if (success != GL_TRUE)
    {
        std::vector<char> log;
        GLint maxSize;
        GLint size;

        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &maxSize);
        log.resize(maxSize);

        glGetProgramInfoLog(prog, maxSize, &size, &log[0]);
        log[size] = '\0';

        const char* text = &log[0];
        lx_error("Shader compilation error: '%s'", text);            
    }
}

GLuint 
RasterizerGL::_createShader( char* filename, GLuint type)
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
        
    lx_check_error( glGetError() == GL_NO_ERROR, "OpenGL error detected." );

    auto pQuadList = new RasterizerGL::QuadList;
    pQuadList->vbo[0] = vbo[0];
    pQuadList->vao[0] = vao[0];
    pQuadList->size = positionData.size();
    return GeometryPtr(pQuadList);
}



void RasterizerGL::QuadList::activate()
{
    glBindVertexArray(vao[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

    // TODO: The position input to the vertex shader is apparently always 0.  Find out where
    // this is documented.
    const int positionIndex = 0;   // glGetAttribLocation(prog, "gl_Vertex");
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionIndex);

    glDrawArrays(GL_QUADS, 0, size); 
}

RasterizerGL::GeometryPtr
RasterizerGL::createQuadList (std::vector<unsigned short>& indices, 
                              std::vector<point3>& positions, 
                              std::vector<vector3>& normals,
                              std::vector<vector3>& colors)
{
    // Create a vertex array to store the vertex data
    //
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vio;
    glGenBuffers(1, &vio);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);

    GLuint vboPositions;
    glGenBuffers(1, &vboPositions);
    glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]), &positions[0], GL_STATIC_DRAW);
    
    GLuint vboNormals;
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), &normals[0], GL_STATIC_DRAW);

    GLuint vboColors;
    glGenBuffers(1, &vboColors);
    glBindBuffer(GL_ARRAY_BUFFER, vboColors);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors[0]), &colors[0], GL_STATIC_DRAW);

    // Create the cache to encapsulate the created OGL resources
    //
    auto pGeom = new GeomImp;
    pGeom->mType        = GL_QUADS;
    pGeom->mVao         = vao;
    pGeom->mVboIndices  = vio;
    pGeom->mCount       = indices.size();
    pGeom->mVboPosition = vboPositions;
    pGeom->mVboNormal   = vboNormals;
    pGeom->mVboColors   = vboColors;
    return GeometryPtr(pGeom);
}

void 
Rasterizer::GeomImp::activate()
{
    GLint shaderProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

    glBindVertexArray(mVao);

    // Bind the position data
    glBindBuffer(GL_ARRAY_BUFFER, mVboPosition);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
            
    GLint normalIndex = glGetAttribLocation(shaderProgram, "vertNormal");
    if (normalIndex != -1)
    {
        if (mVboNormal)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mVboNormal);
            glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(normalIndex);
        }
        else
            glDisableVertexAttribArray(normalIndex);
    }

    GLint colorIndex = glGetAttribLocation(shaderProgram, "vertColor");
    if (colorIndex != -1)
    {
        if (mVboColors)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mVboColors);
            glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(colorIndex);
        }
        else
            glDisableVertexAttribArray(colorIndex);
    }

    // Options
    {
        GLint loc = glGetUniformLocation(shaderProgram, "unifFlatNormals");
        glUniform1i(loc,  1);
    }

    glDrawElements(mType, mCount, GL_UNSIGNED_SHORT, 0);
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

RasterizerGL::TransformPtr 
RasterizerGL::createTransform (float tx, float ty, float tz)
{
    TransformPtr spTransform(new Transform);
    set_translation(spTransform->mat, tx, ty, tz);
    return spTransform;
}

void
RasterizerGL::Transform::activate (void)
{
    glMatrixMode(GL_MODELVIEW);
    glMultTransposeMatrixf(mat.data);
}

void RasterizerGL::beginScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RasterizerGL::endScene()
{
}

void RasterizerGL::rasterizeList (std::vector<std::shared_ptr<Item>>& list)
{
    for (auto it = list.begin(); it != list.end(); ++it)
        rasterize(*it);
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