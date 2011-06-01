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

#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>

#include <boost/filesystem.hpp>

#include <lx0/prototype/misc.hpp>
#include <lx0/util/misc/util.hpp>
#include "glrasterizer.hpp"
#include <lx0/subsystem/rasterizer.hpp>
#include <glgeom/glgeom.hpp>

using namespace lx0::subsystem::rasterizer;
using namespace glgeom;

void RasterizerGL::initialize()
{
    lx_log("%s", __FUNCTION__);

    // Initialization
    //
    lx_log("Using OpenGL v%s", (const char*)glGetString(GL_VERSION));
    lx_log("Using GLSL v%s", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    glClearColor(0.09f, 0.09f, 0.11f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.01f);
}

void RasterizerGL::shutdown()
{
    lx_log("%s", __FUNCTION__);
}

CameraPtr       
RasterizerGL::createCamera (float fov, float nearDist, float farDist, glm::mat4& viewMatrix)
{
    CameraPtr spCamera (new Camera);
    spCamera->fov = fov;
    spCamera->nearDist = nearDist;
    spCamera->farDist = farDist;
    spCamera->viewMatrix = viewMatrix;
    return spCamera;
}

void
Camera::activate()
{
    lx_check_error( glGetError() == GL_NO_ERROR );

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

    lx_check_error( glGetError() == GL_NO_ERROR );
}

Texture::Texture()
    : mFileTimestamp    (0)
    , mId               (0)
{
}

void Texture::unload()
{
    glDeleteTextures(1, &mId);
    mId = 0;
}

void Texture::load()
{
    using namespace lx0::prototype;
    namespace fs = boost::filesystem;

    fs::path path(mFilename);
    if (!fs::exists(path))
    {
        lx_warn("Texture file '%s' does not exist.  Cannot load.", mFilename.c_str());
        mId = 0;
    }
    else
    {
        // Track when the file was last written to so that the file can 
        // (optionally) be automatically reloaded from disk, if changed.
        mFileTimestamp = fs::last_write_time(path);

        Image4b img;
        load_png(img, mFilename.c_str());

        GLuint id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.mWidth, img.mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.mData.get());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        mId = id;
    }
}

TexturePtr
RasterizerGL::createTexture (const char* filename)
{
    lx_check_error(filename != nullptr);

    TexturePtr spTex(new Texture);
    spTex->mFilename = filename;
    spTex->mId = 0;

    spTex->load();

    if (spTex->mId == 0)
        lx_error("Failed to load texture from file '%s'", filename);
    
    mResources.push_back(spTex);
    mTextures.push_back(spTex);
    return spTex;
}

MaterialPtr 
RasterizerGL::createMaterial (std::string fragShader)
{
    GLuint prog = _createProgram(fragShader);
    MaterialPtr spMat(new Material(prog));
    spMat->mShaderFilename = fragShader;
    return spMat;
}

MaterialPtr 
RasterizerGL::createPhongMaterial (const glgeom::material_phong_f& mat)
{
    GLuint prog = _createProgram("media2/shaders/glsl/fragment/phong2.frag");
    auto pPhong = new PhongMaterial(prog);
    pPhong->mShaderFilename = "media2/shaders/glsl/fragment/phong2.frag";
    pPhong->mPhong = mat;
    return MaterialPtr(pPhong);
}

GLuint 
RasterizerGL::_createProgram   (std::string fragShader)
{
    auto it = mCachePrograms.find(fragShader);
    if (it != mCachePrograms.end())
    {
        return it->second;
    }
    else
    {
        GLuint id = _createProgram2(fragShader);
        mCachePrograms.insert(std::make_pair(fragShader, id));
        return id;
    }
}

GLuint 
RasterizerGL::_createProgram2  (std::string fragShader)
{
    lx_debug("Creating program for shader '%s'", fragShader.c_str());

    // Create the shader program
    //
    GLuint vs = _createShader("media2/shaders/glsl/vertex/basic_01.vert", GL_VERTEX_SHADER);
    GLuint gs = _createShader("media2/shaders/glsl/geometry/basic_01.geom", GL_GEOMETRY_SHADER);
    GLuint fs = _createShader(fragShader.c_str(), GL_FRAGMENT_SHADER);

    GLuint prog = glCreateProgram();
    {
        glAttachShader(prog, vs);
        glAttachShader(prog, gs);
        glAttachShader(prog, fs);
        
        // OpenGL 3.2 spec states that geometry shaders must output strips for
        // lines and triangles.
        if (gs)
        {
            GLenum geometryInput = GL_TRIANGLES;
            GLenum geometryOutput = GL_TRIANGLE_STRIP;
            int maxPrimitives = 3;
                
            glProgramParameteriEXT(prog, GL_GEOMETRY_INPUT_TYPE_EXT,    geometryInput);
            glProgramParameteriEXT(prog, GL_GEOMETRY_OUTPUT_TYPE_EXT,   geometryOutput);
            glProgramParameteriEXT(prog, GL_GEOMETRY_VERTICES_OUT_EXT,  maxPrimitives);
        }
            
        glBindAttribLocation(prog, 0, "inPosition");
    }

    _linkProgram(prog);
    glUseProgram(prog);

    return prog;
}

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
    // Activate the shader
    glUseProgram(mId);

    //
    // Pass in additional transform information
    // 
    {
        GLint unifIndex = glGetUniformLocation(mId, "unifViewMatrix");
        if (unifIndex != -1)
        {
            glUniformMatrix4fv(unifIndex, 1, GL_FALSE, glm::value_ptr(*pRasterizer->mContext.viewMatrix));
        }
    }
    
    //
    // Set up lights
    //
    {
        auto& spLightSet = pRasterizer->mContext.spItem->spLightSet;
        int lightCount = (int)spLightSet->mLights.size();

        {
            GLint unifIndex = glGetUniformLocation(mId, "unifAmbient");
            if (unifIndex != -1)
            {
                glUniform3f(unifIndex, 0.1f, 0.1f, 0.1f);
            }
        }

        {
            GLint unifIndex = glGetUniformLocation(mId, "unifLightCount");
            if (unifIndex != -1)
            {
                glUniform1i(unifIndex, lightCount);
            }
        }


        if (lightCount > 0)
        {
            {
                GLint idx = glGetUniformLocation(mId, "unifLightPosition[0]");
                if (idx != -1)
                {
                    std::vector<point3f> positions;
                    positions.reserve(lightCount);

                    for (int i = 0; i < lightCount; ++i)
                        positions.push_back( spLightSet->mLights[i]->position );

                    glUniform3fv(idx, lightCount, &positions[0].x);
                }
            }
            {
                GLint idx = glGetUniformLocation(mId, "unifLightColor[0]");
                if (idx != -1)
                {
                    std::vector<color3f> colors;
                    colors.reserve(lightCount);

                    for (int i = 0; i < lightCount; ++i)
                        colors.push_back( spLightSet->mLights[i]->color );

                    glUniform3fv(idx, lightCount, &colors[0].r);
                }
            }
        }
    }

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
                // Set the shader uniform to the *texture unit* containing the texture
                glUniform1i(unifIndex, i);

                // Activate the corresponding texture unit and set *that* to the GL id
                glActiveTexture(GL_TEXTURE0 + i);
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

void 
RasterizerGL::_linkProgram (GLuint prog)
{
    lx_check_error(glGetError() == GL_NO_ERROR, "GL error status detected!");

    glLinkProgram(prog);

    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);    
    if (success != GL_TRUE)
    {
        std::vector<char> log;
        GLint maxSize;
        GLint size;

        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &maxSize);
        log.resize(maxSize);

        glGetProgramInfoLog(prog, maxSize, &size, &log[0]);
        log[size] = '\0';

        const char* text = &log[0];
        lx_error("Shader compilation error: '%s'", text);            
    }
}

GLuint 
RasterizerGL::_createShader(const char* filename, GLuint type)
{
    GLuint shaderHandle = 0; 

    std::string shaderText = lx0::lx_file_to_string(filename);
    if (!shaderText.empty())
    {
        shaderHandle = glCreateShader(type);

        const GLchar* text = shaderText.c_str();
        glShaderSource(shaderHandle, 1, &text, 0);
        shaderText.swap(std::string());

        glCompileShader(shaderHandle);
    }
    else
        lx_error("Could not load shader '%s' (file exists = %s)", filename, lx0::lx_file_exists(filename) ? "true" : "false");

    return shaderHandle;
}


GeometryPtr 
RasterizerGL::createQuadList (std::vector<glgeom::point3f>& positionData)
{
    GLuint vao[1];

    // Create a vertex array to store the vertex data
    //
    glGenVertexArrays(1, &vao[0]);
    glBindVertexArray(vao[0]);

    lx_check_error( glGetError() == GL_NO_ERROR, "OpenGL error detected." );

    GLuint vbo[1];
    glGenBuffers(1, &vbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glgeom::point3f) * positionData.size(), &positionData[0], GL_STATIC_DRAW);
        
    lx_check_error( glGetError() == GL_NO_ERROR, "OpenGL error detected." );

    auto pQuadList = new QuadList;
    pQuadList->vbo[0] = vbo[0];
    pQuadList->vao[0] = vao[0];
    pQuadList->size = positionData.size();
    return GeometryPtr(pQuadList);
}



void QuadList::activate(GlobalPass& pass)
{
    glBindVertexArray(vao[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

    // TODO: The position input to the vertex shader is apparently always 0.  Find out where
    // this is documented.
    const int positionIndex = 0;   // glGetAttribLocation(prog, "gl_Vertex");
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionIndex);

    glDrawArrays(GL_QUADS, 0, size); 
}

GeometryPtr
RasterizerGL::createQuadList (std::vector<unsigned short>& indices, 
                              std::vector<glgeom::point3f>& positions, 
                              std::vector<glgeom::vector3f>& normals,
                              std::vector<glgeom::color3f>& colors)
{
    std::vector<lx0::uint8> flags(indices.size() / 4);
    std::fill(flags.begin(), flags.end(), 0);

    return createQuadList(indices, flags, positions, normals, colors);
}

GeometryPtr
RasterizerGL::createQuadList (std::vector<unsigned short>& indices, 
                              std::vector<lx0::uint8>& faceFlags,
                              std::vector<glgeom::point3f>& positions, 
                              std::vector<glgeom::vector3f>& normals,
                              std::vector<glgeom::color3f>& colors)
{
    lx_check_error( glGetError() == GL_NO_ERROR );

    lx_check_error(indices.size() == faceFlags.size() * 4, 
        "Expected a single flag per quad.  Count mismatch (%u indicies, %u flags).",
        indices.size(), faceFlags.size());

    // Create a vertex array to store the vertex data
    //
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vio;
    glGenBuffers(1, &vio);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);

    GLuint vboPositions;
    glGenBuffers(1, &vboPositions);
    glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]), &positions[0], GL_STATIC_DRAW);
    
    GLuint vboNormals;
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), &normals[0], GL_STATIC_DRAW);

    GLuint vboColors;
    glGenBuffers(1, &vboColors);
    glBindBuffer(GL_ARRAY_BUFFER, vboColors);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors[0]), &colors[0], GL_STATIC_DRAW);

    GLuint vboFlags;
    glGenBuffers(1, &vboFlags);
    glBindBuffer(GL_ARRAY_BUFFER, vboFlags);
    glBufferData(GL_ARRAY_BUFFER, faceFlags.size() * sizeof(faceFlags[0]), &faceFlags[0], GL_STATIC_DRAW);

    lx_check_error( glGetError() == GL_NO_ERROR );

    // Create the cache to encapsulate the created OGL resources
    //
    auto pGeom = new GeomImp;
    pGeom->mType        = GL_QUADS;
    pGeom->mVao         = vao;
    pGeom->mVboIndices  = vio;
    pGeom->mCount       = indices.size();
    pGeom->mVboPosition = vboPositions;
    pGeom->mVboNormal   = vboNormals;
    pGeom->mVboColors   = vboColors;
    pGeom->mVboFlags    = vboFlags;
    return GeometryPtr(pGeom);
}

void 
GeomImp::activate(GlobalPass& pass)
{
    lx_check_error( glGetError() == GL_NO_ERROR );

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

    // Set per-primitive flags
    {
        GLint idx = glGetAttribLocation(shaderProgram, "primFlag");
        if (idx != -1)
        {
            if (mVboFlags)
            {
                glBindBuffer(GL_ARRAY_BUFFER, mVboFlags);
                glVertexAttribPointer(idx, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(idx);
            }
            else
                glDisableVertexAttribArray(idx);
        }
    }

    // Options
    {
        GLint loc = glGetUniformLocation(shaderProgram, "unifFlatNormals");
        if (loc != -1)
        {
            glUniform1i(loc,  (pass.tbFlatShading == true) ? 0 : 1);
        }
    }

    lx_check_error( glGetError() == GL_NO_ERROR );

    glDrawElements(mType, mCount, GL_UNSIGNED_SHORT, 0);

    lx_check_error( glGetError() == GL_NO_ERROR );
}

LightPtr  
RasterizerGL::createLight (void)
{
    return LightPtr(new Light);
}

LightSetPtr     
RasterizerGL::createLightSet (void)
{
    return LightSetPtr(new LightSet);
}

TransformPtr 
RasterizerGL::createTransform (glm::mat4& mat)
{
    auto spTransform = TransformPtr(new Transform);
    spTransform->mat = mat;
    return spTransform;
}

TransformPtr 
RasterizerGL::createTransform (float tx, float ty, float tz)
{
    TransformPtr spTransform(new Transform);
    spTransform->mat = glm::translate(glm::mat4(1.0f), glm::vec3(tx, ty, tz));
    return spTransform;
}

TransformPtr 
RasterizerGL::createTransform (const glgeom::vector3f& scale, const glgeom::point3f& center)
{
    TransformPtr spTransform(new Transform);
    spTransform->mat = glm::translate(glm::mat4(1.0f), center.vec);
    spTransform->mat = glm::scale(spTransform->mat, scale.vec);
    return spTransform;
}

struct EyeTransform : public Transform
{
    EyeTransform (float tx, float ty, float tz, glgeom::radians zangle)
        : pos (tx, ty, tz)
        , z_angle(zangle)
    {
    }

    virtual void activate(CameraPtr spCamera)
    {
        // The transformation intends to keep the object coordinates relative to the camera,
        // rather than relative to the world origin.   The intent is to therefore offset
        // the object coordinate system by the opposite of the camera offset - i.e. moving
        // the object coordinate system to be at the camera origin.
        //
        // The camera translation is stored in the 3rd column of the view matrix.  The translation
        // is in eye coordinates, therefore to come up with the world coordinate equivalent,
        // those translation values are multipled by the basis vectors (i.e. the first 3 rows
        // of the view matrix).
        //
        auto& view = spCamera->viewMatrix;

        const glgeom::vector3f t(view[3][0], view[3][1], view[3][2]);
        const auto camX = glm::row(view, 0);
        const auto camY = glm::row(view, 1);
        const auto camZ = glm::row(view, 2);

        glgeom::vector3f translation;
        translation.x = t.x * camX.x + t.y * camY.x + t.z * camZ.x;
        translation.y = t.x * camX.y + t.y * camY.y + t.z * camZ.y;
        translation.z = t.x * camX.z + t.y * camY.z + t.z * camZ.z;

        mat = glm::translate(glm::mat4(1.0f), -translation.vec);
        
        glMatrixMode(GL_MODELVIEW);
        glMultMatrixf(glm::value_ptr(mat));
        glRotatef(z_angle.value, 0.0f, 0.0f, 1.0f);
    }

    glgeom::point3f  pos;
    glgeom::radians z_angle;
};

TransformPtr
RasterizerGL::createTransformEye (float tx, float ty, float tz, glgeom::radians z_angle)
{
    return TransformPtr(new EyeTransform(tx, ty, tz, z_angle));
}

struct BillboardTransform : public Transform
{
    virtual void activate(CameraPtr spCamera)
    {
        // The first column of the view matrix contains the "right" vector
        // of the camera: that is to say, the x-axis (1, 0, 0) of the camera
        // in terms of world space.  Therefore, check the angle between that
        // vector and world x to know how much to rotate the model to ensure
        // the model x is always aligned to the camera x.  I.e. rotate the
        // model by the same amount as the camera about the z-axis.
        //
        const auto& cameraX = glm::row(spCamera->viewMatrix, 0);   
        const float radians = atan2(cameraX.y, cameraX.x);

        glMatrixMode(GL_MODELVIEW);   
        glMultMatrixf(glm::value_ptr(mat));
        glRotatef(radians * 180.0f / 3.1415926f, 0.0f, 0.0f, 1.0f);
    }
};

TransformPtr
RasterizerGL::createTransformBillboardXY (float tx, float ty, float tz)
{
    TransformPtr spTransform(new BillboardTransform);
    spTransform->mat = glm::translate(glm::mat4(1.0f), glm::vec3(tx, ty, tz));
    return spTransform;
}

TransformPtr
RasterizerGL::createTransformBillboardXYS (float tx, float ty, float tz, float sx, float sy, float sz)
{
    TransformPtr spTransform(new BillboardTransform);
    spTransform->mat = glm::translate(glm::mat4(1.0f), glm::vec3(tx, ty, tz));

    glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, sz));

    spTransform->mat = spTransform->mat * s;
    return spTransform;
}

void
Transform::activate (CameraPtr)
{
    glMatrixMode(GL_MODELVIEW);
    glMultMatrixf(glm::value_ptr(mat));
}

/*!
    Compare the timestamp of all loaded textures versus the file on disk (if known)
    for that texture.  If the file has been modified since the texture was loaded,
    reload it.
 */ 
void 
RasterizerGL::refreshTextures (void)
{
    namespace fs = boost::filesystem;

    for (auto it = mTextures.begin(); it != mTextures.end(); ++it)
    {
        auto spTex = *it;

        fs::path path(spTex->mFilename);
        std::time_t timestamp = fs::last_write_time(path);

        if (timestamp > spTex->mFileTimestamp)
        {
            // Check that the file is not still open; if the file was just saved and
            // not yet closed, this could fail while the file is still being written
            // to.
            if (!lx0::lx_file_is_open(spTex->mFilename))
            {
                spTex->unload();
                spTex->load();
            }
        }
    }
}

void 
RasterizerGL::beginScene (RenderAlgorithm& algorithm)
{
    lx_check_error( glGetError() == GL_NO_ERROR );

    // Should the clear actually be part of the GlobalPass?  Additionally to this?
    const auto& color = algorithm.mClearColor;
    glClearColor(color.r, color.g, color.b, color.a);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RasterizerGL::endScene()
{
    lx_check_error( glGetError() == GL_NO_ERROR );
}

void 
RasterizerGL::rasterizeList (RenderAlgorithm& algorithm, std::vector<std::shared_ptr<Item>>& list)
{
    for (auto pass = algorithm.mPasses.begin(); pass != algorithm.mPasses.end(); ++pass)
    {
        mContext.itemId = 0;
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            auto spItem = *it;
            if (spItem)
            {
                rasterize(*pass, *it);
            }
            else
            {
                lx_error("Null Item in rasterization list");
            }
            mContext.itemId ++;
        }
    }
}

void 
RasterizerGL::rasterize(GlobalPass& pass, std::shared_ptr<Item> spItem)
{
    lx_check_error(spItem.get() != nullptr);

    mContext.spItem = spItem;

    spItem->spCamera->activate();
    mContext.viewMatrix = &spItem->spCamera->viewMatrix;

    spItem->spLightSet->activate();

    // Activate the material
    MaterialPtr spMaterial = (pass.bOverrideMaterial) 
        ? pass.spMaterial
        : spItem->spMaterial;
    spMaterial->activate(this, pass);

    spItem->spTransform->activate(spItem->spCamera);
    spItem->spGeometry->activate(pass);    

    mContext.spItem = nullptr;
}


/*!
    Dev Note: 
    
    This is not a very good API, as it implicitly relies on the backbuffer
    being properly being written to (which is not necessarily the case after a 
    SwapBuffers() call).  This should likely take a Target as an argument, so it is
    explicit and obvious where the pixel is being read from.

    Secondly, returning an unsigned int is an obvious problem - as it represents
    an assumption this is being used by the selection ID system.

    * Should window coordinates adopt OpenGL's 0 at the bottom standard?  Probably?
 */
unsigned int
RasterizerGL::readPixel (int x, int y)
{
    // Flip y since window coordinates differ from OpenGL's 0 at the bottom convention 
    GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
    y = viewport[3] - y;
	
    // Read the pixel
    glReadBuffer(GL_BACK);
    GLubyte pixel[4];
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)pixel);

    unsigned int id = (pixel[0] << 16) + (pixel[1] << 8) + pixel[2];
    return id;
}

//===========================================================================//

void
RenderList::push_back (int layer, ItemPtr spItem)
{
    mLayers[layer].list.push_back(spItem);
}

ItemPtr 
RenderList::getItem (unsigned int id)
{
    auto it = mLayers.begin();

    while (it->second.list.size() < id)
    {
        id -= it->second.list.size();
        it++;
    }
    return it->second.list[id];
}
