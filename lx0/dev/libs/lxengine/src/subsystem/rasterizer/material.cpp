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
using namespace glgeom;


Material::Material(GLuint id)
    : mId           (id)
    , mBlend        (false)
    , mZWrite       (true)
    , mZTest        (true)
    , mWireframe    (false)
    , mFilter       (GL_LINEAR)
{
}

/*!
    Dev Notes:
    
    - Eventually the index look-ups and which fields need to be set can be 
      cached for performance reasons on the first activation.  
 */
void
Material::activate (RasterizerGL* pRasterizer, GlobalPass& pass)
{
    check_glerror();

    // Activate the shader
    glUseProgram(mId);
    
    //
    // Set up color
    //
    {
        GLint unifIndex = glGetUniformLocation(mId, "inColor");
        if (unifIndex != -1)
        {
            unsigned int id = pRasterizer->mContext.itemId;
            lx_check_error(id < (1 << 24), "ID too high to encode in 24-bit color buffer");
            
            // Why not use the alpha as well?
            // (1) Technically not all cards necessarily support "destination alpha".  This is
            // rare and limited old cards, though.
            // (2) For version 1.0, 2^24 ids should be sufficient.  Simpler is better for v1.0.
            // (3) It's easier to debug when the colors are blitted to the screen rather than
            //     solely offscreen as an intermediate render.
            //
            float r = ((id >> 16) & 0xFF) / 255.0f;
            float g = ((id >>  8) & 0xFF) / 255.0f;
            float b = ((id >>  0) & 0xFF) / 255.0f;
            float a = 255;
            
            glUniform4f(unifIndex, r, g, b, a);
        }
    }
    {
        GLint idx = glGetUniformLocation(mId, "unifMaterialDiffuse");
        if (idx != -1)
            glUniform3f(idx, 1.0f, 1.0f, 1.0f);
    }
    {
        GLint idx = glGetUniformLocation(mId, "unifMaterialSpecular");
        if (idx != -1)
            glUniform3f(idx, 1.0f, 1.0f, 1.0f);
    }
    {
        GLint idx = glGetUniformLocation(mId, "unifMaterialSpecularEx");
        if (idx != -1)
            glUniform1f(idx, 32.0f);
    }

    const char* unifTextureName[8] = {
        "unifTexture0",
        "unifTexture1",
        "unifTexture2",
        "unifTexture3",
        "unifTexture4",
        "unifTexture5",
        "unifTexture6",
        "unifTexture7",
    };

    //
    // Set up textures
    //
    int texturesUseCount = 0;
    for (int i = 0; i < 8; ++i)
    {
        // Set unifTexture0 to texture unit 0
        GLint unifIndex = glGetUniformLocation(mId, unifTextureName[i]);
        if (unifIndex != -1)
        {
            texturesUseCount++;

            if (mTextures[i])
            {
                const auto unit = pRasterizer->mContext.textureUnit++;

                // Set the shader uniform to the *texture unit* containing the texture
                glUniform1i(unifIndex, unit);

                // Activate the corresponding texture unit and set *that* to the GL id
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D, mTextures[i]->mId);

                // Set the parameters on the texture unit
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFilter);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFilter);
            }
            else
            {
                lx_warn("Active shader requires a texture for slot %d, but no texture is set.", i);
            }
        }
    }

    if (texturesUseCount > 0)
        glEnable(GL_TEXTURE_2D);
    else
        glDisable(GL_TEXTURE_2D);

    check_glerror();

    //
    // Z Test/Write
    //
    if (mZTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    glDepthMask(mZWrite ? GL_TRUE : GL_FALSE);
    

    //
    // Set up blending
    // 
    if (mBlend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
        glDisable(GL_BLEND);

    //
    // Wireframe render mode?
    //
    bool bWireframe = boost::indeterminate(pass.tbWireframe)
        ? mWireframe
        : pass.tbWireframe;

    if (bWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    check_glerror();
}


SolidColorMaterial::SolidColorMaterial (GLuint id)
    : Material (id)
{
}

void
SolidColorMaterial::activate (RasterizerGL* pRasterizer, GlobalPass& pass)
{
    check_glerror();

    Material::activate(pRasterizer, pass);

    //
    // Set up color
    //
    {
        GLint idx = glGetUniformLocation(mId, "inColor");
        if (idx != -1)
            glUniform4f(idx, mColor.r, mColor.g, mColor.b, 1.0f);
    }

    check_glerror();
}


VertexColorMaterial::VertexColorMaterial (GLuint id)
    : Material (id)
{
}

void
VertexColorMaterial::activate (RasterizerGL* pRasterizer, GlobalPass& pass)
{
    check_glerror();

    Material::activate(pRasterizer, pass);

    check_glerror();
}

PhongMaterial::PhongMaterial (GLuint id)
    : Material (id)
{
}

void
PhongMaterial::activate (RasterizerGL* pRasterizer, GlobalPass& pass)
{
    Material::activate(pRasterizer, pass);

    //
    // Set up color
    //
    {
        GLint idx = glGetUniformLocation(mId, "unifMaterialDiffuse");
        if (idx != -1)
            glUniform3f(idx, mPhong.diffuse.r, mPhong.diffuse.g, mPhong.diffuse.b);
    }
    {
        GLint idx = glGetUniformLocation(mId, "unifMaterialSpecular");
        if (idx != -1)
            glUniform3f(idx, mPhong.specular.r, mPhong.specular.g, mPhong.specular.b);
    }
    {
        GLint idx = glGetUniformLocation(mId, "unifMaterialSpecularEx");
        if (idx != -1)
            glUniform1f(idx, mPhong.specular_n);
    }
}


GenericMaterial::GenericMaterial (GLuint id)
    : Material (id)
{
}

void
GenericMaterial::activate (RasterizerGL* pRasterizer, GlobalPass& pass)
{
    //
    // Set up all common base settings
    //
    Material::activate(pRasterizer, pass);

    //
    // The parameters lxvar can be internally "compiled" into index locations
    // and function calls.  For now, the slow lxvar conversions are done every
    // time to keep the code simple and flexible.
    //
    if (mParameters.is_defined())
    {
        for (auto it = mParameters.begin(); it != mParameters.end(); ++it)
        {
            const std::string uniformName = it.key();
            const std::string type        = (*it)[0];
            lxvar&            value       = (*it)[1];

            GLint index = glGetUniformLocation(mId, uniformName.c_str());
            if (index != -1)
            {
                if (type == "vec2")
                    glUniform2f(index, value[0].as<float>(), value[1].as<float>());
                else if (type == "vec3")
                    glUniform3f(index, value[0].as<float>(), value[1].as<float>(), value[2].as<float>());
                else if (type == "vec4")
                    glUniform4f(index, value[0].as<float>(), value[1].as<float>(), value[2].as<float>(), value[3].as<float>());
            }
        }
    }
}
