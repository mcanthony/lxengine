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
#include <lx0/core/core.hpp>
#include <lx0/core/util/util.hpp>
#include <lx0/canvas/canvas.hpp>

#include <gl/glew.h>
#include <windows.h>        // Unfortunately must be included on Windows for GL.h to work
#include <gl/GL.h>

using namespace lx0::core;
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
        GLuint vs = createShader("media/shaders/glsl/vertex/transform_only.vert", GL_VERTEX_SHADER);
        GLuint fs = createShader("media/shaders/glsl/fragment/minimal.frag", GL_FRAGMENT_SHADER);

        GLuint prog = glCreateProgram();
        {
            glAttachShader(prog, vs);
            glAttachShader(prog, fs);
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
            -2.0f,  -1.0f, -10.0f,
             0.0f,   2.0f, -10.0f,
             2.0f,  -1.0f, -10.0f,
        };
        GLuint vbo[1];
        glGenBuffers(1, &vbo[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, &positionData[0], GL_STATIC_DRAW);
        
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
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void 
    render (void)	
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glBindVertexArray(vao[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);        
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

    auto pWin = new CanvasGL("OpenGL 3.2", 16, 16, 800, 400, false);
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
