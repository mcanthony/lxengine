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
#include <glm/gtc/matrix_inverse.hpp>

using namespace lx0;
using namespace glgeom;

extern OpenGlApi3_2* gl;


MaterialType::MaterialType (GLuint id)
    : mProgram    (id)
    , mVertShader (0)
    , mGeomShader (0)
    , mFragShader (0)
    , mDefaults   (lx0::lxvar::map())
{
    lx_check_error( gl->isProgram(id) );
}

MaterialType::~MaterialType()
{
}

MaterialInstancePtr 
MaterialType::createInstance (lx0::lxvar& parameters)
{
    return MaterialInstancePtr( new MaterialInstance(shared_from_this(), parameters) );
}

void 
MaterialType::iterateUniforms (std::function<void(const Uniform& uniform)> f)
{
    int uniformCount;
    gl->getProgramiv(mProgram, GL_ACTIVE_UNIFORMS, &uniformCount); 
    for (int i = 0; i < uniformCount; ++i)  
    {
        Uniform uniform;
        char    uniformName[128];
        GLsizei uniformNameLength;

        gl->getActiveUniform(mProgram, GLuint(i), sizeof(uniformName), &uniformNameLength, &uniform.size, &uniform.type, uniformName);

        if (uniformNameLength >= sizeof(uniformName))
        {
            throw lx_error_exception("GLSL program contains a uniform with too long a name size!");
        }
        else
        {
            uniform.name = uniformName;
            uniform.location = gl->getUniformLocation(mProgram, uniformName);
            f(uniform);
        }
    }
}

void 
MaterialType::iterateAttributes (std::function<void(const Attribute& attribute)> f)
{
    int attributeCount;
    gl->getProgramiv(mProgram, GL_ACTIVE_ATTRIBUTES, &attributeCount); 
    for (int i = 0; i < attributeCount; ++i)  
    {
        Attribute attribute;
        char    attribName[128];
        GLsizei attribNameLength;

        gl->getActiveAttrib(mProgram, GLuint(i), sizeof(attribName), &attribNameLength, &attribute.size, &attribute.type, attribName);

        if (attribNameLength >= sizeof(attribName))
        {
            throw lx_error_exception("GLSL program contains an attribute with too long a name size!");
        }
        else
        {
            attribute.name = attribName;
            attribute.location = gl->getAttribLocation(mProgram, attribName);
            f(attribute);
        }
    }
}

void    
MaterialType::activate (RasterizerGL* pRasterizer, GlobalPass& pass)
{
    pRasterizer->mFrameData.shaderProgramActivations++;

    // Activate the shader
    gl->useProgram(mProgram);
}

MaterialInstance::MaterialInstance (MaterialTypePtr spMaterialType, lx0::lxvar& parameters)
    : mspMaterialType (spMaterialType)
    , mParameters   (parameters.clone())
    , mBlend        (false)
    , mZTest        (true)
    , mZWrite       (true)
    , mWireframe    (false)
    , mFilter       (GL_LINEAR)
    , mbDirty       (true)
{
}

void    
MaterialInstance::activate (RasterizerGL* pRasterizer, GlobalPass& pass)
{
    check_glerror();

    if (mbDirty)
        _compile(pRasterizer);

    mspMaterialType->activate(pRasterizer, pass);

    int count = 0;
    for (auto it = mInstructions.begin(); it != mInstructions.end(); ++it)
    {
        std::function<void()>& instruction = *it;
        lx_check_error(instruction);
        
        instruction();
        check_glerror();

        ++count;
    }

    check_glerror();
}

void    
MaterialInstance::_compile (RasterizerGL* pRasterizer)
{
    //
    // Clear any cached instructions
    // 
    mInstructions.clear();

    //
    // Add the base code common to all instances
    //
    mInstructions.push_back( _generateBaseInstruction(pRasterizer) );

    //
    // Loop the material parameters and generate instructions to set a 
    // value for each instruction.
    //
    // First determine the 'specified value' by checking in order for:
    // - The instance parameters table contains a value for the name
    // - The type defaults table contains a value for the name
    // - The name is a known 'standard' which has a built-in default
    //
    // Second, translate the 'specified value' into an instruction that sets a 'direct value'
    // - If it is a direct value, generate an instruction to set that value
    // - If it is an indirect/semantic value, generate an instruction to 
    //   look-up the direct value and set it
    //
    // The specified to direct value may involve implicit type conversions, 
    // references to the current context, and other objects.
    //
    auto& parameters = mParameters;
    auto& defaults = mspMaterialType->mDefaults;
    auto& standards = pRasterizer->mStandardParameterValues;

    auto findSpecifiedValue = [&](const std::string& name) -> lx0::lxvar {
        if (parameters.has_key(name))
            return parameters[name];
        else if (defaults.has_key(name))
            return defaults[name];
        else if (standards.has_key(name))
            return standards[name];
        else
            return lx0::lxvar();
    };

    mspMaterialType->iterateAttributes([&](const Attribute& attribute) {
        lx0::lxvar specifiedValue = findSpecifiedValue(attribute.name);
        auto instr = _generateInstruction(pRasterizer, attribute, specifiedValue);
        if (instr)
            mInstructions.push_back(instr);
    });

    mspMaterialType->iterateUniforms([&](const Uniform& uniform) {
        lx0::lxvar specifiedValue = findSpecifiedValue(uniform.name);
        auto instr = _generateInstruction(pRasterizer, uniform, specifiedValue);
        if (instr)
            mInstructions.push_back(instr);
    });

    mbDirty = false;
}

/*
    For consistency, it might make sense to store all these as parameters:
    hide the fact that they result in GL calls rather than shader parameters.
 */
std::function<void()>   
MaterialInstance::_generateBaseInstruction (RasterizerGL* pRasterizer)
{
    return [this, pRasterizer]() {

        check_glerror();

        auto& pass = *pRasterizer->mContext.pGlobalPass;

        //
        // Z Test/Write
        //
        if (mZTest)
            gl->enable(GL_DEPTH_TEST);
        else
            gl->disable(GL_DEPTH_TEST);

        gl->depthMask(mZWrite ? GL_TRUE : GL_FALSE);
    

        //
        // Set up blending
        // 
        if (mBlend)
        {
            gl->enable(GL_BLEND);
            gl->blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
            gl->disable(GL_BLEND);

        //
        // Wireframe render mode?
        //
        bool bWireframe = boost::indeterminate(pass.tbWireframe)
            ? mWireframe
            : pass.tbWireframe;

        if (bWireframe)
            gl->polygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            gl->polygonMode(GL_FRONT_AND_BACK, GL_FILL);

        check_glerror();

    };
}

std::function<void()>   
MaterialInstance::_generateInstruction(RasterizerGL* pRasterizer, const Attribute& attribute, lx0::lxvar& value)
{
    auto location = attribute.location;

    if (attribute.name == "vertNormal")
    {
        return [=]() {
            auto& vboNormals = pRasterizer->mContext.spGeometry->mVboNormal;
            if (vboNormals)
            {
                gl->bindBuffer(GL_ARRAY_BUFFER, vboNormals);
                gl->vertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
                gl->enableVertexAttribArray(location);
            }
            else
                gl->disableVertexAttribArray(location);

            check_glerror();
        };
    }
    else if (attribute.name == "vertColor")
    {
        return [=]() {
            auto& mVboColors = pRasterizer->mContext.spGeometry->mVboColors;
            if (mVboColors)
            {
                gl->bindBuffer(GL_ARRAY_BUFFER, mVboColors);
                gl->vertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
                gl->enableVertexAttribArray(location);
            }
            else
                gl->disableVertexAttribArray(location);

            check_glerror();
        };
    }
    else if (attribute.name == "vertUV")
    {
        return [=]() {
            auto& mVboUVs = pRasterizer->mContext.spGeometry->mVboUVs;
            if (mVboUVs[0])
            {
                gl->bindBuffer(GL_ARRAY_BUFFER, mVboUVs[0]);
                gl->vertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 0, 0);
                gl->enableVertexAttribArray(location);
            }
            else
                gl->disableVertexAttribArray(location);
            
            check_glerror();
        };
    }
    else if (attribute.name == "gl_Vertex")
    {
        return [=]() {
            auto& mVboPosition = pRasterizer->mContext.spGeometry->mVboPosition;
            if (mVboPosition)
            {
                gl->bindBuffer(GL_ARRAY_BUFFER, mVboPosition);
                gl->vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                gl->enableVertexAttribArray(0);
            }
            else
                gl->disableVertexAttribArray(0);

            check_glerror();
        };
    }

    lx_warn("No instruction generated for attribute '%1%'", attribute.name);
    return std::function<void()>();
}

std::function<void()>   
MaterialInstance::_generateInstruction (RasterizerGL* pRasterizer, const Uniform& uniform, lx0::lxvar& value)
{
    auto loc = uniform.location;

    if (value.is_defined())
    {
        if (uniform.type == GL_FLOAT)
        {
            if (uniform.size == 1)
            {
                float v = value.as<float>();
                return [=]() { 
                    gl->uniform1f(loc, v); 
                    check_glerror();
                };
            }
            else if (uniform.size == 2)
            {
                float v0 = value[0].as<float>();
                float v1 = value[1].as<float>();
                return [=]() { 
                    gl->uniform2f(loc, v0, v1); 
                    check_glerror();
                };
            }
            else if (uniform.size == 3)
            {
                float v0 = value[0].as<float>();
                float v1 = value[1].as<float>();
                float v2 = value[2].as<float>();
                return [=]() { 
                    gl->uniform3f(loc, v0, v1, v2); 
                    check_glerror();
                };
            }
            else if (uniform.size == 4)
            {
                float v0 = value[0].as<float>();
                float v1 = value[1].as<float>();
                float v2 = value[2].as<float>();
                float v3 = value[3].as<float>();
                return [=]() { 
                    gl->uniform4f(loc, v0, v1, v2, v3); 
                    check_glerror();
                };
            }
        }
        else if (uniform.type == GL_SAMPLER_2D)
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
                return [=]() {
                    const auto unit = pRasterizer->mContext.textureUnit++;

                    // Set the shader uniform to the *texture unit* containing the texture (NOT
                    // the GL id of the texture)
                    gl->uniform1i(loc, unit);

                    gl->activeTexture(GL_TEXTURE0 + unit);
                    gl->bindTexture(GL_TEXTURE_2D, textureId);

                    // Set the parameters on the texture unit
                    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFilter);
                    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFilter);
                    gl->enable(GL_TEXTURE_2D);
                    check_glerror();
                };
            }
            else if (bUseFBO)
            {
                return [this,loc,pRasterizer]() {
                    const auto textureId = pRasterizer->mContext.sourceFBOTexture;
                    const auto unit = pRasterizer->mContext.textureUnit++;

                    // Set the shader uniform to the *texture unit* containing the texture (NOT
                    // the GL id of the texture)
                    gl->uniform1i(loc, unit);

                    gl->activeTexture(GL_TEXTURE0 + unit);
                    gl->bindTexture(GL_TEXTURE_2D, textureId);

                    // Set the parameters on the texture unit
                    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    gl->enable(GL_TEXTURE_2D);
                    check_glerror();
                };
            }
            else
                lx_warn("Could not find referenced texture '%s' in the texture cache.", name.c_str());
        }
        else if (uniform.type == GL_SAMPLER_CUBE)
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

                return [=]() {
                    const auto unit = pRasterizer->mContext.textureUnit++;

                    // Set the shader uniform to the *texture unit* containing the texture (NOT
                    // the GL id of the texture)
                    gl->uniform1i(loc, unit);

                    gl->activeTexture(GL_TEXTURE0 + unit);
                    gl->bindTexture(GL_TEXTURE_CUBE_MAP, texId);
                    check_glerror();
                };
            }
        }
    }
    else
    {
        if (uniform.name == "unifProjMatrix")
        {        
            return [=]() {
                auto& spMatrix = pRasterizer->mContext.uniforms.spProjMatrix;
                gl->uniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(*spMatrix));
                check_glerror();
            };
        }
        else if (uniform.name == "unifViewMatrix")
        {
            return [=]() {
                auto& spMatrix = pRasterizer->mContext.uniforms.spViewMatrix;
                gl->uniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(*spMatrix));
                check_glerror();
            };
        }
        else if (uniform.name == "unifNormalMatrix")
        {
            return [=]() {
                auto& spViewMatrix = pRasterizer->mContext.uniforms.spViewMatrix;
                glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(*spViewMatrix));
                gl->uniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
                check_glerror();
            };
        }
        else if (uniform.name == "unifFlatNormals")
        {
            return [=]() {
                const int flatShading = (pRasterizer->mContext.tbFlatShading == true) ? 1 : 0; 
                gl->uniform1i(loc, flatShading);
                check_glerror();
            };
        }
    }

    lx_warn("No instruction generated for uniform '%1%'", uniform.name);
    return std::function<void()>();
}

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
    gl->useProgram(mId);
        
    //
    // Set up color
    //
    {
        GLint unifIndex = gl->getUniformLocation(mId, "inColor");
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
            
            gl->uniform4f(unifIndex, r, g, b, a);
        }
    }
    {
        GLint idx = gl->getUniformLocation(mId, "unifMaterialDiffuse");
        if (idx != -1)
            gl->uniform3f(idx, 1.0f, 1.0f, 1.0f);
    }
    {
        GLint idx = gl->getUniformLocation(mId, "unifMaterialSpecular");
        if (idx != -1)
            gl->uniform3f(idx, 1.0f, 1.0f, 1.0f);
    }
    {
        GLint idx = gl->getUniformLocation(mId, "unifMaterialSpecularEx");
        if (idx != -1)
            gl->uniform1f(idx, 32.0f);
    }

    check_glerror();

    //
    // Z Test/Write
    //
    if (mZTest)
        gl->enable(GL_DEPTH_TEST);
    else
        gl->disable(GL_DEPTH_TEST);

    gl->depthMask(mZWrite ? GL_TRUE : GL_FALSE);
    

    //
    // Set up blending
    // 
    if (mBlend)
    {
        gl->enable(GL_BLEND);
        gl->blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
        gl->disable(GL_BLEND);

    //
    // Wireframe render mode?
    //
    bool bWireframe = boost::indeterminate(pass.tbWireframe)
        ? mWireframe
        : pass.tbWireframe;

    if (bWireframe)
        gl->polygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        gl->polygonMode(GL_FRONT_AND_BACK, GL_FILL);

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
        GLint idx = gl->getUniformLocation(mId, "inColor");
        if (idx != -1)
            gl->uniform4f(idx, mColor.r, mColor.g, mColor.b, 1.0f);
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
        GLint idx = gl->getUniformLocation(mId, "unifMaterialDiffuse");
        if (idx != -1)
            gl->uniform3f(idx, mPhong.diffuse.r, mPhong.diffuse.g, mPhong.diffuse.b);
    }
    {
        GLint idx = gl->getUniformLocation(mId, "unifMaterialSpecular");
        if (idx != -1)
            gl->uniform3f(idx, mPhong.specular.r, mPhong.specular.g, mPhong.specular.b);
    }
    {
        GLint idx = gl->getUniformLocation(mId, "unifMaterialSpecularEx");
        if (idx != -1)
            gl->uniform1f(idx, mPhong.specular_n);
    }
}

template <typename Material>
std::function<void()>
_generateBaseInstruction (RasterizerGL* pRasterizer, Material* pMaterial)
{
    return [=]() {

        check_glerror();

        auto& pass = *pRasterizer->mContext.pGlobalPass;
    
        pRasterizer->mFrameData.shaderProgramActivations++;

        //
        // Z Test/Write
        //
        if (pMaterial->mZTest)
            gl->enable(GL_DEPTH_TEST);
        else
            gl->disable(GL_DEPTH_TEST);

        gl->depthMask(pMaterial->mZWrite ? GL_TRUE : GL_FALSE);
    
        //
        // Set up blending
        // 
        if (pMaterial->mBlend)
        {
            gl->enable(GL_BLEND);
            gl->blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
            gl->disable(GL_BLEND);

        //
        // Wireframe render mode?
        //
        bool bWireframe = boost::indeterminate(pass.tbWireframe)
            ? pMaterial->mWireframe
            : pass.tbWireframe;

        if (bWireframe)
            gl->polygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            gl->polygonMode(GL_FRONT_AND_BACK, GL_FILL);

        check_glerror();
    };
}

std::function<void()> 
_generateStandardUniform (RasterizerGL* pRasterizer, GLint location, std::string name, GLenum type, GLint count)
{
    if (name == "unifProjMatrix")
    {        
        return [=]() {
            auto& spMatrix = pRasterizer->mContext.uniforms.spProjMatrix;
            gl->uniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(*spMatrix));
        };
    }
    else if (name == "unifViewMatrix")
    {
        return [=]() {
            auto& spMatrix = pRasterizer->mContext.uniforms.spViewMatrix;
            gl->uniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(*spMatrix));
        };
    }
    else if (name == "unifNormalMatrix")
    {
        return [=]() {
            auto& spViewMatrix = pRasterizer->mContext.uniforms.spViewMatrix;
            glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(*spViewMatrix));
            gl->uniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        };
    }
    else if (name == "unifFlatNormals")
    {
        return [=]() {
            const int flatShading = (pRasterizer->mContext.tbFlatShading == true) ? 1 : 0; 
            gl->uniform1i(location, flatShading);
        };
    }

    return std::function<void()>();
}

std::function<void()> 
_generateStandardAttribute (RasterizerGL* pRasterizer, GLint location, std::string name, GLenum type, GLint count)
{
    if (name == "vertNormal")
    {
        return [=]() {
            auto& vboNormals = pRasterizer->mContext.spGeometry->mVboNormal;
            if (vboNormals)
            {
                gl->bindBuffer(GL_ARRAY_BUFFER, vboNormals);
                gl->vertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
                gl->enableVertexAttribArray(location);
            }
            else
                gl->disableVertexAttribArray(location);
        };
    }
    else if (name == "vertColor")
    {
        return [=]() {
            auto& mVboColors = pRasterizer->mContext.spGeometry->mVboColors;
            if (mVboColors)
            {
                gl->bindBuffer(GL_ARRAY_BUFFER, mVboColors);
                gl->vertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
                gl->enableVertexAttribArray(location);
            }
            else
                gl->disableVertexAttribArray(location);
        };
    }
    else if (name == "vertUV")
    {
        return [=]() {
            auto& mVboUVs = pRasterizer->mContext.spGeometry->mVboUVs;
            if (mVboUVs[0])
            {
                gl->bindBuffer(GL_ARRAY_BUFFER, mVboUVs[0]);
                gl->vertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 0, 0);
                gl->enableVertexAttribArray(location);
            }
            else
                gl->disableVertexAttribArray(location);
        };
    }

    return std::function<void()>();
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
    gl->getProgramiv(mId, GL_ACTIVE_UNIFORMS, &uniformCount); 
    for (int i = 0; i < uniformCount; ++i)  
    {
        GLenum  uniformType;
        char    uniformName[128];
        GLsizei uniformNameLength;
        GLint   uniformSize;        // Array size, if an array, 1 otherwise
        GLint   uniformLocation;

        gl->getActiveUniform(mId, GLuint(i), sizeof(uniformName), &uniformNameLength, &uniformSize, &uniformType, uniformName);
        uniformLocation = gl->getUniformLocation(mId, uniformName);

        if (uniformNameLength >= sizeof(uniformName))
        {
            throw lx_error_exception("GLSL program contains a uniform with too long a name size!");
        }
        else
        {
            //
            // Check instance parameters, type default parameters, then "standard" names.
            // Check for direct values, implicitly convertible values, then indirect semantic values.
            //

            std::function<void()> instruction;

            if (mParameters.has_key(uniformName))
            {
                // Ignore for now.
            }
            else if (/*mDefaults has key*/ false) 
            { 
            }
            else if (instruction = _generateStandardUniform(pRasterizer, uniformLocation, uniformName, uniformType, uniformSize))
            {
                mInstructions.push_back(instruction);
            }
            else
                lx_log("Program contains unspecified uniform: %1%", uniformName);
        }
    }

    //
    // Query the list of active attributes
    //
    int attributeCount;
    gl->getProgramiv(mId, GL_ACTIVE_ATTRIBUTES, &attributeCount); 
    for (int i = 0; i < attributeCount; ++i)  
    {
        GLenum  attribType;
        char    attribName[128];
        GLsizei attribNameLength;
        GLint   attribSize;        // Array size, if an array, 1 otherwise
        GLint   attribLocation;

        gl->getActiveAttrib(mId, GLuint(i), sizeof(attribName), &attribNameLength, &attribSize, &attribType, attribName);
        attribLocation = gl->getAttribLocation(mId, attribName);

        if (attribNameLength >= sizeof(attribName))
        {
            throw lx_error_exception("GLSL program contains an attribute with too long a name size!");
        }
        else
        {
            std::function<void()> instruction;
            if (instruction = _generateStandardAttribute(pRasterizer, attribLocation, attribName, attribType, attribSize))
                mInstructions.push_back(instruction);
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

        GLint index = gl->getUniformLocation(mId, uniformName.c_str());
        if (index != -1)
        {
            if (type == "float")
            {
                float v = value.as<float>();
                mInstructions.push_back([=]() { gl->uniform1f(index, v); });
            }
            else if (type == "vec2")
            {
                float v0 = value[0].as<float>();
                float v1 = value[1].as<float>();
                mInstructions.push_back([=]() { gl->uniform2f(index, v0, v1); });
            }
            else if (type == "vec3")
            {
                float v0 = value[0].as<float>();
                float v1 = value[1].as<float>();
                float v2 = value[2].as<float>();
                mInstructions.push_back([=]() { gl->uniform3f(index, v0, v1, v2); });
            }
            else if (type == "vec4")
            {
                float v0 = value[0].as<float>();
                float v1 = value[1].as<float>();
                float v2 = value[2].as<float>();
                float v3 = value[3].as<float>();
                mInstructions.push_back([=]() { gl->uniform4f(index, v0, v1, v2, v3); });
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
                        gl->uniform1i(index, unit);

                        gl->activeTexture(GL_TEXTURE0 + unit);
                        gl->bindTexture(GL_TEXTURE_2D, textureId);

                        // Set the parameters on the texture unit
                        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFilter);
                        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFilter);
                        gl->enable(GL_TEXTURE_2D);
                    });
                }
                else if (bUseFBO)
                {
                    mInstructions.push_back([this,index,pRasterizer]() {
                        const auto textureId = pRasterizer->mContext.sourceFBOTexture;
                        const auto unit = pRasterizer->mContext.textureUnit++;

                        // Set the shader uniform to the *texture unit* containing the texture (NOT
                        // the GL id of the texture)
                        gl->uniform1i(index, unit);

                        gl->activeTexture(GL_TEXTURE0 + unit);
                        gl->bindTexture(GL_TEXTURE_2D, textureId);

                        // Set the parameters on the texture unit
                        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        gl->enable(GL_TEXTURE_2D);
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
                        gl->uniform1i(index, unit);

                        gl->activeTexture(GL_TEXTURE0 + unit);
                        gl->bindTexture(GL_TEXTURE_CUBE_MAP, texId);
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
