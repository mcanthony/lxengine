//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2012 athile@athile.net (http://www.athile.net)

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

extern OpenGlApi3_2* gl;

Geometry::~Geometry()
{
    if (mVao)
        gl->deleteVertexArrays(1, &mVao);
    if (mVboPosition)
        gl->deleteBuffers(1, &mVboPosition);
    if (mVboNormal)
        gl->deleteBuffers(1, &mVboNormal);
    if (mVboColors)
        gl->deleteBuffers(1, &mVboColors);

    for (int i = 0; i < 8; ++i)
        if (mVboUVs[i])
            gl->deleteBuffers(1, &mVboUVs[i]);
}

/*!
    This method takes the entire set of geometry properties (e.g. the list of vertices,
    the indices, the vertex attributes like normals, colors, etc.) and searches for
    shader program variables to bind those attriutes to.  The shader program variable
    names are mapped to geometry variables *by convention*.
 */
void 
Geometry::activate(RasterizerGL* pRasterizer, GlobalPass& pass)
{
    check_glerror();

    auto spMaterial = pRasterizer->mContext.spMaterial;

    if (spMaterial->mGeometryType != this->mType)
        throw lx_error_exception("Material does not support this geometry type!");

    switch (mType)
    {
    case GL_POINTS:
        gl->pointSize(3);
        break;
    }

    GLint shaderProgram;
    gl->getIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

    gl->bindVertexArray(mVao);

    // Bind the position data
    gl->bindBuffer(GL_ARRAY_BUFFER, mVboPosition);
    gl->vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    gl->enableVertexAttribArray(0);
            
    // Set per-primitive flags
    {
        bool texture1dUsed = false;

        GLint idx = gl->getUniformLocation(shaderProgram, "unifFaceFlags");
        if (idx != -1)
        {
            if (mTexFlags)
            {
                const auto unit = pRasterizer->mContext.textureUnit++;

                // Set the shader uniform to the *texture unit* containing the texture
                gl->uniform1i(idx, unit);

                // Activate the corresponding texture unit and set *that* to the GL id
                gl->activeTexture(GL_TEXTURE0 + unit);
                gl->bindTexture(GL_TEXTURE_1D, mTexFlags);

                texture1dUsed = true;
            }
        }

        if (texture1dUsed)
            gl->enable(GL_TEXTURE_1D);
        else
            gl->disable(GL_TEXTURE_1D);
    }

    {
        GLint idx = gl->getUniformLocation(shaderProgram, "unifFaceCount");
        if (idx != -1)
        {
            gl->uniform1i(idx, mFaceCount);
        }
    }


    check_glerror();

    //
    // Is this an indexed primitive or a list of vertices?
    //
    if (mCount < 1)
    {
        throw lx_error_exception("Cannot render empty geometry set!");
    }

    if (mVboIndices)
        gl->drawElements(mType, mCount, GL_UNSIGNED_SHORT, 0);
    else
        gl->drawArrays(mType, 0, mCount); 

    check_glerror();
}
