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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

// Standard headers
#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

// Library headers
#include <boost/program_options.hpp>

// Lx0 headers
#include <lx0/core.hpp>
#include <lx0/matrix.hpp>
#include <lx0/util.hpp>
#include <lx0/canvas/canvas.hpp>
#include <lx0/prototype/prototype.hpp>

#include <gl/glew.h>
#include <windows.h>        // Unfortunately must be included on Windows for GL.h to work
#include <gl/GL.h>

using namespace lx0::core;
using namespace lx0::prototype;
using namespace lx0::canvas::platform;

//===========================================================================//

class Renderer
{
public:
    void initialize()
    {
        // Initialization
        //
        lx_log("Using OpenGL v%s", (const char*)glGetString(GL_VERSION));

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

        // Create a vertex array to store the vertex data
        //
        glGenVertexArrays(1, &vao[0]);
        glBindVertexArray(vao[0]);

        lx_check_error( glGetError() == GL_NO_ERROR, "OpenGL error detected." );
        
        // Create a vertex buffer to store the data for the vertex array
        GLfloat positionData[] = 
        {
             500.0f,  -500.0f,  0.0f,
             500.0f,   500.0f,  0.0f,
            -500.0f,   500.0f,  0.0f,
            -500.0f,  -500.0f,  0.0f,
        };
        GLuint vbo[1];
        glGenBuffers(1, &vbo[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, &positionData[0], GL_STATIC_DRAW);
        
        // TODO: The position input to the vertex shader is apparently always 0.  Find out where
        // this is documented.
        const int positionIndex = 0;   // glGetAttribLocation(prog, "gl_Vertex");
        glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(positionIndex);
        lx_check_error( glGetError() == GL_NO_ERROR, "OpenGL error detected." );
    }  

    void 
    resize (int width, int height)
    {
        //
        // Setup projection matrix
        //
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        int vp[4];  // x, y, width, height
        glGetIntegerv(GL_VIEWPORT, vp);
        gluPerspective(60.0f, (GLfloat)vp[2]/(GLfloat)vp[3], 0.01f, 9000.0f);

        //
        // Setup view matrix
        //
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        Camera camera;
        set(camera.mPosition, 500, 500, 500);
        set(camera.mTarget, 0, 0, 0);
        set(camera.mWorldUp, 0, 0, 1);

        matrix4 viewMatrix;
        lookAt(viewMatrix, camera.mPosition, camera.mTarget, camera.mWorldUp);

        // OpenGL matrices are laid out with rows contiguous in memory (i.e. data[1] in 
        // the 16 element array is the second element in the first row), which is the
        // same as matrix4; therefore the matrix can be loaded directly.  Otherwise,
        // glLoadTransposeMatrix should be used.
        glLoadTransposeMatrixf(viewMatrix.data);
    }

    void 
    render (void)	
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        glBindVertexArray(vao[0]);
        glDrawArrays(GL_QUADS, 0, 4);        
    }

protected:
    GLuint 
    createShader( char* filename, GLuint type)
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

    GLuint vao[1];
};

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    int exitCode = 0;

    lx_init();

    CanvasHost host;
    Renderer renderer;

    auto pWin = new CanvasGL("OpenGL 3.2", 800, 400, false);
    host.create(pWin, "canvas", false);
    renderer.initialize();
    renderer.resize(800, 400);
    pWin->slotRedraw += [&]() { renderer.render(); };
    pWin->show();

    bool bDone = false;
    do {
        bDone = host.processEvents();
    } while (!bDone);

    return exitCode;
}
