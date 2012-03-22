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
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/canvas.hpp>
#include <lx0/subsystem/rasterizer/gl/glinterface.hpp>

using namespace lx0;
using namespace lx0::core;

static OpenGlApi3_2* gl = 0;

//===========================================================================//

class Renderer
{
public:
    void initialize()
    {
        // Initialization
        //
        gl = new OpenGlApi3_2;
        gl->initialize();
        lx_log("Using OpenGL v%s", (const char*)gl->getString(GL_VERSION));

        // Create the shader program
        //
        GLuint vs = createShader("media2/shaders/glsl/vertex/transform_only.vert", GL_VERTEX_SHADER);
        GLuint fs = createShader("media2/shaders/glsl/fragment/minimal.frag", GL_FRAGMENT_SHADER);

        GLuint prog = gl->createProgram();
        {
            gl->attachShader(prog, vs);
            gl->attachShader(prog, fs);
            gl->bindAttribLocation(prog, 0, "inPosition");
        }
        gl->linkProgram(prog);
        gl->useProgram(prog);

        // Create a vertex array to store the vertex data
        //
        gl->genVertexArrays(1, &vao[0]);
        gl->bindVertexArray(vao[0]);

        lx_check_error( gl->getError() == GL_NO_ERROR, "OpenGL error detected." );
        
        // Create a vertex buffer to store the data for the vertex array
        GLfloat positionData[] = 
        {
            -2.0f,  -1.0f, -10.0f,
             0.0f,   2.0f, -10.0f,
             2.0f,  -1.0f, -10.0f,
        };
        GLuint vbo[1];
        gl->genBuffers(1, &vbo[0]);
        gl->bindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        gl->bufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, &positionData[0], GL_STATIC_DRAW);
        
        // TODO: The position input to the vertex shader is apparently always 0.  Find out where
        // this is documented.
        const int positionIndex = 0;   // gl->getAttribLocation(prog, "gl_Vertex");
        gl->vertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
        gl->enableVertexAttribArray(positionIndex);
        lx_check_error( gl->getError() == GL_NO_ERROR, "OpenGL error detected." );
    }  

    void 
    resize (int width, int height)
    {
        /*glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();*/
    }

    void 
    render (void)	
    {
        gl->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        gl->bindVertexArray(vao[0]);
        gl->drawArrays(GL_TRIANGLES, 0, 3);        
    }

protected:
    GLuint 
    createShader( char* filename, GLuint type)
    {
        GLuint shaderHandle = 0; 

        std::string shaderText = lx0::string_from_file(filename);
        if (!shaderText.empty())
        {
            shaderHandle = gl->createShader(type);

            const GLchar* text = shaderText.c_str();
            gl->shaderSource(shaderHandle, 1, &text, 0);
            shaderText.swap(std::string());

            gl->compileShader(shaderHandle);
        }
        else
            throw lx_error_exception("Could not load shader '%s' (file exists = %s)", filename, lx0::file_exists(filename) ? "true" : "false");

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

    /*
     LxEngine was upgraded to use the OpenGL 3.2 Core Profile.  This sample is
     still based on the Compatibility profile.  The primary issue is the use
     of the view and projection matrices, which need to be replaced with 
     uniform variables in the shader.  This is not difficult, but there is a 
     question of how relevant this sample is in the first place.
     */
    throw lx_error_exception("This sample is currently broken! It needs to be fixed or removed.");

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
