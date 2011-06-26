//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)

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

#include <lx0/subsystem/rasterizer.hpp>

using namespace lx0;

void
Camera::activate()
{
    check_glerror();

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
    // the 16 element array is the second element in the first row); glm::mat4 uses
    // column-major ordering.  Therefore load it as a tranpose to swap the order.
    //
    // Note: it's also not necessarily faster to send the non-transpose, as the
    // OpenGL API does not necessarily match the underlying hardware representation.
    //
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(viewMatrix));

    check_glerror();
}
