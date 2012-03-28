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

#include <lx0/libs/rasterizer.hpp>

using namespace lx0;

extern OpenGlApi3_2* gl;

glm::mat4 
Camera::projectionMatrix (void) const
{
    int vp[4];  // x, y, width, height
    gl->getIntegerv(GL_VIEWPORT, vp);
    GLfloat aspectRatio = (GLfloat)vp[2]/(GLfloat)vp[3];

    float fovDegrees = glgeom::degrees(fov).value;
    return glm::perspective(fovDegrees, aspectRatio, nearDist, farDist);
}

void
Camera::activate(RasterizerGL* pRasterizer)
{
    check_glerror();
        
    //
    // Setup projection matrix
    //
    auto projMatrix = projectionMatrix();
    pRasterizer->mContext.uniforms.spProjMatrix.reset(new glm::mat4(projMatrix));
    pRasterizer->mContext.uniforms.spViewMatrix.reset(new glm::mat4(viewMatrix));

    check_glerror();
}
