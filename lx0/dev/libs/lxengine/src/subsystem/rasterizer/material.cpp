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
    , mGeometryType (GL_INVALID_ENUM)       // Can't use GL_NONE because GL_POINTS == GL_NONE
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

    pRasterizer->mFrameData.shaderProgramActivations++;

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
    : Material          (id)
    , mbNeedsCompile    (true)
    , mParameters       (lx0::lxvar::map())
{
}

/*!
    Compiles the lxvar set of parameters into a set of std::function<> objects.

    Using a vector of std::functions<> is a compromise between simplicity (it's
    quite simple) and efficiency (there's still more overhead than technically
    necessary).  Overall, this leads to highly flexible, highly maintainable
    code that still removes the major bottlenecks of uniform look-ups and lxvar
    conversions on every material activation.

    TODO:

    The instruction list should take the mContext as a parameter.  This will
    allow instructions to be written for uniforms that pull the value out of
    the current context, rather than via an explicit value.  We can currently
    "cheat" and store a copy of the incoming pRasterizer in the function object
    to accomplish this.
 */
void
GenericMaterial::_compile (RasterizerGL* pRasterizer)
{
    // 
    // Clear the "compiled" cache
    //
    mInstructions.clear();

    //
    // Query the list of uniforms in the actual GLSL program
    //
    int uniformCount;
    glGetProgramiv(mId, GL_ACTIVE_UNIFORMS, &uniformCount); 
    for (int i = 0; i < uniformCount; ++i)  
    {
        GLenum  uniformType;
        char    uniformName[128];
        GLsizei uniformNameLength;
        GLint   uniformSize;        // Array size, if an array, 1 otherwise

        glGetActiveUniform(mId, GLuint(i), sizeof(uniformName), &uniformNameLength, &uniformSize, &uniformType, uniformName);

        if (uniformNameLength >= sizeof(uniformName))
        {
            throw lx_error_exception("GLSL program contains a uniform with too long a name size!");
        }
        else
        {
            if (!mParameters.has_key(uniformName))
            {
                lx_log("Program contains unspecified uniform: %1%", uniformName);
            }
        }
    }

    //
    // Query the list of active attributes
    //
    int attributeCount;
    glGetProgramiv(mId, GL_ACTIVE_ATTRIBUTES, &attributeCount); 
    for (int i = 0; i < attributeCount; ++i)  
    {
        GLenum  attribType;
        char    attribName[128];
        GLsizei attribNameLength;
        GLint   attribSize;        // Array size, if an array, 1 otherwise

        glGetActiveAttrib(mId, GLuint(i), sizeof(attribName), &attribNameLength, &attribSize, &attribType, attribName);

        if (attribNameLength >= sizeof(attribName))
        {
            throw lx_error_exception("GLSL program contains an attribute with too long a name size!");
        }
        else
        {
                lx_log("Program contains active attribute: %1%", attribName);
        }
    }

    //
    // Cycle through the provided parameters.
    //
    // This list may be a superset of what's actually used in the program.
    // (Note: does that make sense? What's the use case for the superset?)
    //
    for (auto it = mParameters.begin(); it != mParameters.end(); ++it)
    {
        const std::string uniformName = it.key();
        const std::string type        = (*it)[0];
        lxvar&            value       = (*it)[1];

        GLint index = glGetUniformLocation(mId, uniformName.c_str());
        if (index != -1)
        {
            if (type == "float")
            {
                float v = value.as<float>();
                mInstructions.push_back([=]() { glUniform1f(index, v); });
            }
            else if (type == "vec2")
            {
                float v0 = value[0].as<float>();
                float v1 = value[1].as<float>();
                mInstructions.push_back([=]() { glUniform2f(index, v0, v1); });
            }
            else if (type == "vec3")
            {
                float v0 = value[0].as<float>();
                float v1 = value[1].as<float>();
                float v2 = value[2].as<float>();
                mInstructions.push_back([=]() { glUniform3f(index, v0, v1, v2); });
            }
            else if (type == "vec4")
            {
                float v0 = value[0].as<float>();
                float v1 = value[1].as<float>();
                float v2 = value[2].as<float>();
                float v3 = value[3].as<float>();
                mInstructions.push_back([=]() { glUniform4f(index, v0, v1, v2, v3); });
            }
            else if (type == "sampler2D")
            {
                GLuint textureId = GL_NONE;
                bool   bUseFBO = false;

                auto name = value.as<std::string>();
                if (name[0] != '@')
                {
                    TexturePtr spTexture;

                    auto it = pRasterizer->mTextureCache.find(name);                    
                    if (it == pRasterizer->mTextureCache.end()) 
                    {
                        spTexture = pRasterizer->createTexture(name.c_str());
                        pRasterizer->cacheTexture(name, spTexture);
                    }
                    else
                        spTexture = it->second;

                    textureId = spTexture->mId;
                }
                else
                {
                    if (name == "@sourceFBO")
                        bUseFBO = true;
                }

                if (textureId)
                {
                    // Activate the corresponding texture unit and set *that* to the GL id
                    mInstructions.push_back([=]() {
                        const auto unit = pRasterizer->mContext.textureUnit++;

                        // Set the shader uniform to the *texture unit* containing the texture (NOT
                        // the GL id of the texture)
                        glUniform1i(index, unit);

                        glActiveTexture(GL_TEXTURE0 + unit);
                        glBindTexture(GL_TEXTURE_2D, textureId);

                        // Set the parameters on the texture unit
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFilter);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFilter);
                        glEnable(GL_TEXTURE_2D);
                    });
                }
                else if (bUseFBO)
                {
                    mInstructions.push_back([this,index,pRasterizer]() {
                        const auto textureId = pRasterizer->mContext.sourceFBOTexture;
                        const auto unit = pRasterizer->mContext.textureUnit++;

                        // Set the shader uniform to the *texture unit* containing the texture (NOT
                        // the GL id of the texture)
                        glUniform1i(index, unit);

                        glActiveTexture(GL_TEXTURE0 + unit);
                        glBindTexture(GL_TEXTURE_2D, textureId);

                        // Set the parameters on the texture unit
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFilter);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFilter);
                        glEnable(GL_TEXTURE_2D);
                    });
                }
                else
                    lx_warn("Could not find referenced texture '%s' in the texture cache.", name.c_str());
            }
            else if (type == "samplerCube")
            {
                TexturePtr spTexture;

                auto name = value.as<std::string>();
                auto it = pRasterizer->mTextureCache.find(name);                    
                if (it == pRasterizer->mTextureCache.end()) 
                {
                    std::string file[6];
                    file[0] = name + "/xpos.png";
                    file[1] = name + "/xneg.png";
                    file[2] = name + "/ypos.png";
                    file[3] = name + "/yneg.png";
                    file[4] = name + "/zpos.png";
                    file[5] = name + "/zneg.png";
                    spTexture = pRasterizer->createTextureCubeMap(file[0].c_str(), file[1].c_str(), file[2].c_str(), file[3].c_str(), file[4].c_str(), file[5].c_str());
                    pRasterizer->cacheTexture(name, spTexture);
                }
                else
                    spTexture = it->second;

                if (spTexture)
                {
                    // Activate the corresponding texture unit and set *that* to the GL id
                    const GLuint texId = spTexture->mId;

                    mInstructions.push_back([=]() {
                        const auto unit = pRasterizer->mContext.textureUnit++;

                        // Set the shader uniform to the *texture unit* containing the texture (NOT
                        // the GL id of the texture)
                        glUniform1i(index, unit);

                        glActiveTexture(GL_TEXTURE0 + unit);
                        glBindTexture(GL_TEXTURE_CUBE_MAP, texId);
                    });
                }
            }
            else
                throw lx_error_exception("Unrecognized parameter type '%s'", type.c_str());
        }
    }

    //
    // Mark the compilation as done; no compile needed.
    //
    mbNeedsCompile = false;
}


void
GenericMaterial::activate (RasterizerGL* pRasterizer, GlobalPass& pass)
{
    //
    // Set up all common base settings
    //
    Material::activate(pRasterizer, pass);

    //
    // Looking up uniform locations is relatively expensive: compile the
    // parameters into a set of function calls to provide a speed-up.
    //
    if (mbNeedsCompile)
        _compile(pRasterizer);

    check_glerror();

    //
    // Run the set of instructions to set the parameters
    //
    for (auto it = mInstructions.begin(); it != mInstructions.end(); ++it)
        (*it)();

    check_glerror();
}
