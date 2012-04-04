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

#include <lx0/extensions/rasterizer.hpp>
#include <glm/gtc/matrix_inverse.hpp>

using namespace lx0;
using namespace glgeom;

extern OpenGlApi3_2* gl;

//===========================================================================//
//   M A T E R I A L T Y P E
//===========================================================================//

MaterialClass::MaterialClass (GLuint id)
    : mGeometryType (GL_TRIANGLES)
    , mProgram    (id)
    , mVertShader (0)
    , mGeomShader (0)
    , mFragShader (0)
    , mDefaults   (lx0::lxvar::map())
{
    lx_check_error( gl->isProgram(id) );
}

//---------------------------------------------------------------------------//

MaterialClass::~MaterialClass()
{
}

//---------------------------------------------------------------------------//

MaterialPtr 
MaterialClass::createInstance (lx0::lxvar& parameters)
{
    return MaterialPtr( new Material(shared_from_this(), parameters) );
}

//---------------------------------------------------------------------------//

void 
MaterialClass::iterateUniforms (std::function<void(const Uniform& uniform)> f)
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

//---------------------------------------------------------------------------//

void 
MaterialClass::iterateAttributes (std::function<void(const Attribute& attribute)> f)
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

//---------------------------------------------------------------------------//

void    
MaterialClass::activate (RasterizerGL* pRasterizer, GlobalPass& pass)
{
    pRasterizer->mFrameData.shaderProgramActivations++;

    // Activate the shader
    gl->useProgram(mProgram);
}

//===========================================================================//
//   M A T E R I A L I N S T A N C E
//===========================================================================//

Material::Material (MaterialClassPtr spMaterialClass, lx0::lxvar& parameters)
    : mspMaterialClass (spMaterialClass)
    , mParameters   (parameters.clone())
    , mBlend        (false)
    , mZTest        (true)
    , mZWrite       (true)
    , mWireframe    (false)
    , mFilter       (GL_LINEAR)
    , mbDirty       (true)
{
}

//---------------------------------------------------------------------------//

void    
Material::activate (RasterizerGL* pRasterizer, GlobalPass& pass)
{
    check_glerror();

    if (mbDirty)
        _compile(pRasterizer);

    mspMaterialClass->activate(pRasterizer, pass);

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

//---------------------------------------------------------------------------//

void    
Material::_compile (RasterizerGL* pRasterizer)
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
    auto& defaults = mspMaterialClass->mDefaults;
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

    mspMaterialClass->iterateAttributes([&](const Attribute& attribute) {
        lx0::lxvar specifiedValue = findSpecifiedValue(attribute.name);
        auto instr = _generateInstruction(pRasterizer, attribute, specifiedValue);
        if (instr)
            mInstructions.push_back(instr);
    });

    mspMaterialClass->iterateUniforms([&](const Uniform& uniform) {
        lx0::lxvar specifiedValue = findSpecifiedValue(uniform.name);
        auto instr = _generateInstruction(pRasterizer, uniform, specifiedValue);
        if (instr)
            mInstructions.push_back(instr);
    });

    auto gl = ::gl;
    {
        auto value = findSpecifiedValue("pointSize").as<float>();
        mInstructions.push_back([gl, value]() {
            gl->pointSize(value);
        });
    }

    mbDirty = false;
}

//---------------------------------------------------------------------------//

/*
    For consistency, it might make sense to store all these as parameters:
    hide the fact that they result in GL calls rather than shader parameters.
 */
std::function<void()>   
Material::_generateBaseInstruction (RasterizerGL* pRasterizer)
{
    return [this, pRasterizer]() {

        check_glerror();

        auto& pass = *pRasterizer->mContext.pGlobalPass;

        //
        // Activate Vertex Array Object
        //
        // Some of the other instructions may activate array buffers of the
        // VAO, so ensure the current geometry's VAO is active.
        //
        gl->bindVertexArray(pRasterizer->mContext.spGeometry->mVao);

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

//---------------------------------------------------------------------------//

std::function<void()>   
Material::_generateInstruction(RasterizerGL* pRasterizer, const Attribute& attribute, lx0::lxvar& value)
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
        };
    }

    lx_warn("No instruction generated for attribute '%1%'", attribute.name);
    return std::function<void()>();
}

//---------------------------------------------------------------------------//

std::function<void()>   
Material::_generateInstruction (RasterizerGL* pRasterizer, const Uniform& uniform, lx0::lxvar& value)
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
        else if (uniform.type == GL_INT)
        {
            switch (uniform.size)
            {
            case 1:
                {
                    int v0 = value.as<int>();
                    return [=]() { 
                        gl->uniform1i(loc, v0); 
                        check_glerror();
                    };
                }
            }
        }
        else if (uniform.type == GL_SAMPLER_1D)
        {
            GLuint textureId = GL_NONE;

            auto name = value.as<std::string>();
            if (name[0] != '@')
            {
                TexturePtr& spTexture = pRasterizer->mTextureCache[name];
                if (!spTexture) 
                    spTexture = pRasterizer->createTexture(name.c_str());
                textureId = spTexture->mId;
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
                    gl->bindTexture(GL_TEXTURE_1D, textureId);

                    // Set the parameters on the texture unit
                    gl->texParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, mFilter);
                    gl->texParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, mFilter);
                    gl->texParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	                gl->texParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    gl->enable(GL_TEXTURE_1D);
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
        else if (uniform.name == "unifFaceCount")
        {
            return [=]() {
                gl->uniform1i(loc, pRasterizer->mContext.spGeometry->mFaceCount);
                check_glerror();
            };
        }
        else if (uniform.name == "unifFaceFlags")
        {
            return [=]() {
                bool texture1dUsed = false;
                auto texFlags = pRasterizer->mContext.spGeometry->mTexFlags;
                if (texFlags)
                {
                    const auto unit = pRasterizer->mContext.textureUnit++;

                    // Set the shader uniform to the *texture unit* containing the texture
                    gl->uniform1i(loc, unit);

                    // Activate the corresponding texture unit and set *that* to the GL id
                    gl->activeTexture(GL_TEXTURE0 + unit);
                    gl->bindTexture(GL_TEXTURE_1D, texFlags);

                    texture1dUsed = true;
                }

                if (texture1dUsed)
                    gl->enable(GL_TEXTURE_1D);
                else
                    gl->disable(GL_TEXTURE_1D);
            };
        }
        else if (uniform.name == "unifBBoxMin")
        {
            return [=]() {
                auto& bbox = pRasterizer->mContext.spGeometry->mBBox;
                gl->uniform3f(loc, bbox.min.x, bbox.min.y, bbox.min.z);
            };
        }
        else if (uniform.name == "unifBBoxMax")
        {
            return [=]() {
                auto& bbox = pRasterizer->mContext.spGeometry->mBBox;
                gl->uniform3f(loc, bbox.max.x, bbox.max.y, bbox.max.z);
            };
        }
        else if (uniform.name == "unifCurrentTime")
        {
            return [=]() {
                float time = float(pRasterizer->mContext.frameStart);
                gl->uniform1f(loc, time);
            };
        }
    }

    lx_warn("No instruction generated for uniform '%1%'", uniform.name);
    return std::function<void()>();
}

//---------------------------------------------------------------------------//

void        
Material::trimParameterTypes  (void)
{
    if (mParameters.is_defined())
    {
        for (auto it = mParameters.begin(); it != mParameters.end(); ++it)
        {
            mParameters[it.key()] = (*it)[1];
        }
    }
}
