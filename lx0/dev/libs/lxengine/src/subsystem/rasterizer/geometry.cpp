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

GeomImp::~GeomImp()
{
    if (mVao)
        glDeleteVertexArrays(1, &mVao);
    if (mVboPosition)
        glDeleteBuffers(1, &mVboPosition);
    if (mVboNormal)
        glDeleteBuffers(1, &mVboNormal);
    if (mVboColors)
        glDeleteBuffers(1, &mVboColors);
}

/*!
    This method takes the entire set of geometry properties (e.g. the list of vertices,
    the indices, the vertex attributes like normals, colors, etc.) and searches for
    shader program variables to bind those attriutes to.  The shader program variable
    names are mapped to geometry variables *by convention*.
 */
void 
GeomImp::activate(RasterizerGL* pRasterizer, GlobalPass& pass)
{
    check_glerror();

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

    GLint uvIndex = glGetAttribLocation(shaderProgram, "vertUV");
    if (uvIndex != -1)
    {
        if (mVboUVs[0])
        {
            glBindBuffer(GL_ARRAY_BUFFER, mVboUVs[0]);
            glVertexAttribPointer(uvIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(uvIndex);
        }
        else
            glDisableVertexAttribArray(uvIndex);
    }

    // Set per-primitive flags
    {
        bool texture1dUsed = false;

        GLint idx = glGetUniformLocation(shaderProgram, "unifFaceFlags");
        if (idx != -1)
        {
            if (mTexFlags)
            {
                const auto unit = pRasterizer->mContext.textureUnit++;

                // Set the shader uniform to the *texture unit* containing the texture
                glUniform1i(idx, unit);

                // Activate the corresponding texture unit and set *that* to the GL id
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_1D, mTexFlags);

                texture1dUsed = true;
            }
        }

        if (texture1dUsed)
            glEnable(GL_TEXTURE_1D);
        else
            glDisable(GL_TEXTURE_1D);
    }

    {
        GLint idx = glGetUniformLocation(shaderProgram, "unifFaceCount");
        if (idx != -1)
        {
            glUniform1i(idx, mFaceCount);
        }
    }

    // Options
    {
        GLint loc = glGetUniformLocation(shaderProgram, "unifFlatNormals");
        if (loc != -1)
        {
            const int flatShading = (pRasterizer->mContext.tbFlatShading == true) ? 1 : 0; 
            glUniform1i(loc, flatShading);
        }
    }

    check_glerror();

    //
    // Is this an indexed primitive or a list of vertices?
    //
    if (mVboIndices)
        glDrawElements(mType, mCount, GL_UNSIGNED_SHORT, 0);
    else
        glDrawArrays(mType, 0, mCount); 

    check_glerror();
}
