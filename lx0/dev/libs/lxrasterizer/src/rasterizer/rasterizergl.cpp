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

#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <lx0/prototype/misc.hpp>
#include <lx0/util/misc/util.hpp>
#include <lx0/extensions/rasterizer.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>

#include <glm/gtc/matrix_inverse.hpp>

using namespace lx0::subsystem::rasterizer_ns;
using namespace glgeom;

lx0::OpenGlApi3_2* gl;

struct Profile
{
    Profile() { ::memset(this, 0, sizeof(*this)); }
                    
    int     checkError;
    int     acquireMaterial;
    int     rasterizeList;
    int     rasterizeItem;
    int     beginFrame;
    int     endFrame;

    void registerCounters()
    {
        auto pEngine = lx0::Engine::acquire().get();
        pEngine->registerProfileCounter("Rasterizer checkError",        &checkError);
        pEngine->registerProfileCounter("Rasterizer acquireMaterial",   &acquireMaterial);
        pEngine->registerProfileCounter("Rasterizer rasterizeList",   &rasterizeList);
        pEngine->registerProfileCounter("Rasterizer rasterizeItem",   &rasterizeItem);
        pEngine->registerProfileCounter("Rasterizer beginFrame",   &beginFrame);
        pEngine->registerProfileCounter("Rasterizer endFrame",   &endFrame);

        pEngine->addProfileRelationship("Engine run",               "Rasterizer rasterizeList");
        pEngine->addProfileRelationship("Rasterizer rasterizeList", "Rasterizer beginFrame");
        pEngine->addProfileRelationship("Rasterizer rasterizeList", "Rasterizer rasterizeItem");
        pEngine->addProfileRelationship("Rasterizer rasterizeList", "Rasterizer checkError");
        pEngine->addProfileRelationship("Rasterizer rasterizeItem", "Rasterizer checkError");
    }
} profile;

void lx0::subsystem::rasterizer_ns::check_glerror()
{
    lx0::ProfileSection section(profile.checkError);

	GLenum errCode = gl->getError();
    
    if (errCode != GL_NO_ERROR)
    {
        //
        // Don't want to include glu.h, so manually convert the error to a string
        //
        std::string errString;
        switch (errCode)
        {
        case GL_NO_ERROR:           errString = "GL_NO_ERROR";          break;
        case GL_INVALID_ENUM:       errString = "GL_INVALID_ENUM";      break;
        case GL_INVALID_VALUE:      errString = "GL_INVALID_VALUE";     break;
        case GL_INVALID_OPERATION:  errString = "GL_INVALID_OPERATION"; break;
        case GL_OUT_OF_MEMORY:      errString = "GL_OUT_OF_MEMORY";     break;

        default:                    errString = "<unknown error>";      break;
        }        
        
        lx_warn("glError() detected.");
        lx_warn("Error Code = %1%", errCode);
        lx_warn("Error String = %1%", errString);
        throw lx_error_exception("glError detected: %s", errString);
    }
}

FrameBuffer::FrameBuffer(FrameBuffer::Type type, int width, int height)
    : mHandle        (0)
    , mWidth         (0)
    , mHeight        (0)
    , mTextureHandle (0)
{
    check_glerror();
    if (type == eCreateFrameBuffer)
    {
        GLint prevFBO;
        gl->getIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevFBO);

        gl->genFramebuffers(1, &mHandle);
        gl->bindFramebuffer(GL_FRAMEBUFFER, mHandle);
        lx_check_error(mHandle != GL_NONE);

        mWidth = width;
        mHeight = height;
        
        
        //
        // Generate the color texture
        //
        GLuint renderTex;
        gl->genTextures(1, &renderTex);
        gl->activeTexture(GL_TEXTURE0); // Use texture unit 0
        gl->bindTexture(GL_TEXTURE_2D, renderTex);
        
        gl->texImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);        
        gl->texParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        gl->texParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        gl->framebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);
        
        lx_check_error(renderTex != GL_NONE);
        mTextureHandle = renderTex;

        //
        // Generate the depth buffer
        // 
        GLuint depthBuf;
        gl->genRenderbuffers(1, &depthBuf);
        gl->bindRenderbuffer(GL_RENDERBUFFER, depthBuf);
        gl->renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mWidth, mHeight);
        gl->framebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
        lx_check_error(depthBuf != GL_NONE);
        mDepthHandle = depthBuf;

        lx_check_error( gl->checkFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE );
    
        check_glerror();
        gl->bindTexture(GL_TEXTURE_2D, 0);
        gl->bindRenderbuffer(GL_RENDERBUFFER, 0);
        gl->bindFramebuffer(GL_FRAMEBUFFER, prevFBO);
        check_glerror();
    }
    else
    {
        GLint hCurrentFBO;
        gl->getIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &hCurrentFBO);
        lx_check_error(hCurrentFBO == 0);

        GLint viewport[4];
	    gl->getIntegerv(GL_VIEWPORT, viewport);

        mHandle = 0;
        mWidth = viewport[2];
        mHeight = viewport[3];
    }
}

FrameBuffer::~FrameBuffer()
{
    // Ensure FBO is not current
    GLint currentFBO;
    gl->getIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentFBO);
    if (currentFBO == mHandle)
        gl->bindFramebuffer(GL_FRAMEBUFFER, 0);

    //
    // Delete FBO and associated objects
    //
    gl->deleteTextures(1, &mTextureHandle);
    gl->deleteRenderbuffers(1, &mDepthHandle);
    gl->deleteFramebuffers(1, &mHandle);
}

void 
FrameBuffer::activate()
{
    check_glerror();

    const bool bValidFbo = mHandle == 0 || gl->isFramebuffer(mHandle);
    lx_check_error(bValidFbo);

    gl->bindFramebuffer(GL_FRAMEBUFFER, mHandle);
    gl->viewport(0, 0, mWidth, mHeight);
    
    GLenum fboStatus = gl->checkFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    switch (fboStatus)
    {
    case GL_FRAMEBUFFER_COMPLETE:
        // Nothing to do - everything's fine
        break;
    case GL_FRAMEBUFFER_UNDEFINED:
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
    case GL_FRAMEBUFFER_UNSUPPORTED:
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
    default:
        throw lx_error_exception("Framebuffer incomplete!");
    }  

    check_glerror();    

    GLint handle;
    gl->getIntegerv(GL_FRAMEBUFFER_BINDING, &handle);
    lx_check_error(handle == mHandle);
}

/*!
    Creates the RasterizerGL in an uninitialized state.

    RasterizerGL::initialize() should be called once the GL context is
    available and made current.
 */
RasterizerGL::RasterizerGL()
    : mInited   (false)
    , mShutdown (false)
    , mFrameNum (0)
    , mStandardParameterValues (lx0::lxvar::map())
{
    profile.registerCounters();

    mStandardParameterValues["unifDiffuse"] = lx0::lxvar(0.80f, 0.75f, 0.75f);
    mStandardParameterValues["unifSpecular"] = lx0::lxvar(0.95f, 0.95f, 0.75f);
}

RasterizerGL::~RasterizerGL()
{
    if (!mShutdown)
        lx_warn("RasterizerGL::shutdown() not called!");
}

void RasterizerGL::initialize()
{
    lx_log("%s", __FUNCTION__);
    mInited = true;
    mStats.tmLifetime.start();

    //
    // Initialization
    //
    gl3_2.reset(new lx0::OpenGlApi3_2);
    gl = gl3_2.get();                   // Global pointer for now...
    gl->initialize();
    
    lx_log("Using OpenGL v%s",  (const char*)gl->getString(GL_VERSION));
    lx_log("Using GLSL v%s",    (const char*)gl->getString(GL_SHADING_LANGUAGE_VERSION));

    gl->clearColor(0.09f, 0.09f, 0.11f, 1.0f);
    gl->enable(GL_DEPTH_TEST);

    mspFBOScreen.reset( new FrameBuffer(FrameBuffer::eDefaultFrameBuffer) );
    mspFBOScreen->activate();
}

static void log_timer(const char* name, const lx0::Timer& timer, const lx0::Timer& base)
{
    if (timer.totalMs() > 0)
    {
        lx_log("\t%-24s %-6u ms  %-9.2lf avg  %6.2lf%%  (%u)", 
            name, 
            timer.totalMs(), timer.averageMs(), 
            100.0 * double(timer.totalMs()) / double(base.totalMs()),
            timer.count()
            );
    }
}

void RasterizerGL::shutdown()
{
    mStats.tmLifetime.stop();

    lx_log("%s", __FUNCTION__);
    log_timer("Lifetime",           mStats.tmLifetime,          mStats.tmLifetime);
    log_timer("scene",              mStats.tmScene,             mStats.tmLifetime);
    log_timer("rasterizeList",      mStats.tmRasterizeList,     mStats.tmScene);
    log_timer("rasterizeItem",      mStats.tmRasterizeItem,     mStats.tmScene);
    log_timer("activate Material",  mStats.tmMaterialActivate,  mStats.tmScene);
    log_timer("activate Geometry",  mStats.tmGeometryActivate,  mStats.tmScene);
    log_timer("activate LightSet",  mStats.tmLightSetActivate,  mStats.tmScene);
    log_timer("activate Transform", mStats.tmTransformActivate, mStats.tmScene);

    mShutdown = true;
    mInited = false;
}

FrameBufferPtr  
RasterizerGL::acquireFrameBuffer (std::string name, int width, int height)
{
    auto& spFrameBuffer = mFrameBuffers[name];
    if (!spFrameBuffer)
    {
        spFrameBuffer.reset( new FrameBuffer(FrameBuffer::eCreateFrameBuffer, width, height) );
    }

    lx_check_error(spFrameBuffer->width() == width);
    lx_check_error(spFrameBuffer->height() == height);
    return spFrameBuffer;
}

CameraPtr       
RasterizerGL::createCamera (glgeom::radians fov, float nearDist, float farDist, glm::mat4& viewMatrix)
{
    CameraPtr spCamera (new Camera);
    spCamera->fov = fov;
    spCamera->nearDist = nearDist;
    spCamera->farDist = farDist;
    spCamera->viewMatrix = viewMatrix;
    return spCamera;
}

Texture::Texture()
    : mFileTimestamp    (0)
    , mId               (0)
{
}

void Texture::unload()
{
    gl->deleteTextures(1, &mId);
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
        gl->genTextures(1, &id);
        if (img.mHeight == 1)
        {
            gl->bindTexture(GL_TEXTURE_1D, id);
            gl->texImage1D(GL_TEXTURE_1D, 0, GL_RGBA, img.mWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.mData.get());
            gl->texParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	        gl->texParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            gl->bindTexture(GL_TEXTURE_2D, id);
            gl->texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.mWidth, img.mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.mData.get());
            gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        mId = id;
    }
}

TexturePtr
RasterizerGL::createTexture3f (const glgeom::image3f& image)
{
    TexturePtr spTex(new Texture);

    gl->genTextures(1, &spTex->mId);
    gl->bindTexture(GL_TEXTURE_2D, spTex->mId);
    gl->texImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(), 0, GL_RGB, GL_FLOAT, image.ptr());
    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return spTex;
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
        throw lx_error_exception("Failed to load texture from file '%s'", filename);

    mResources.push_back(spTex);
    mTextures.push_back(spTex);
    return spTex;
}

TexturePtr      
RasterizerGL::createTextureCubeMap (const glgeom::cubemap3f& cubemap)
{
    check_glerror();

    gl->enable(GL_TEXTURE_CUBE_MAP);

    //
    // The filenames[] array order needs to match the expected OpenGL target sequence
    //
    GLuint              id;
    GLenum              target      = GL_TEXTURE_CUBE_MAP_POSITIVE_X;

    gl->genTextures(1, &id);
    gl->bindTexture(GL_TEXTURE_CUBE_MAP, id);

    gl->texParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl->texParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gl->texParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
    gl->texParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->texParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    for (int i = 0; i < 6; ++i, ++target)
    {
        gl->texImage2D(target, 0, GL_RGB, cubemap.width(), cubemap.height(), 0, GL_RGB, GL_FLOAT, cubemap.mImage[i].ptr());
    }

    auto pTexture = new Texture;
    pTexture->mId = id;

    check_glerror();

    return TexturePtr(pTexture);
}

TexturePtr      
RasterizerGL::createTextureCubeMap (const char* xpos, const char* xneg, const char* ypos, const char* yneg, const char* zpos, const char* zneg)
{
    const char*         filenames[] = { xpos, xneg, ypos, yneg, zpos, zneg };

    glgeom::cubemap3f cubemap;
    for (int i = 0; i < 6; ++i)
        lx0::load_png(cubemap.mImage[i], filenames[i]);
    cubemap._resetDimensions();


    return createTextureCubeMap(cubemap);
}

TexturePtr
RasterizerGL::createTextureDDS (std::istream& stream)
{
    TexturePtr spTex(new Texture);
    spTex->mFilename = "";
    spTex->mId = 0;

    GLuint _loadDDS(std::istream& stream);
    spTex->mId = _loadDDS(stream);

    if (spTex->mId == 0)
        throw lx_error_exception("Load of DDS stream failed");
    
    mResources.push_back(spTex);
    mTextures.push_back(spTex);
    return spTex;
}

void 
RasterizerGL::cacheTexture (std::string name, TexturePtr spTexture)
{
    mTextureCache.insert( std::make_pair(name, spTexture) );
}

MaterialPtr 
RasterizerGL::acquireMaterial (std::string className, std::string instanceName)
{
    lx0::ProfileSection section(profile.acquireMaterial);

    std::string materialName = className + "." + instanceName;

    //
    // Create a material instance using all the material class defaults
    //
    MaterialPtr& spMaterial = mMaterials[materialName];
    if (!spMaterial)
    {
        lx_log("Loading material '%1%'", materialName);

        //
        // Add some local helpers
        //
        std::string base = "media2/shaders/lx/materials";
        
        auto conditionalLoad = [](std::string base, std::string name, std::string file) -> std::string {
            std::string filename = base + "/" + name + "/" + file;
            if (lx0::file_exists(filename))
                return lx0::string_from_file(filename);
            else
                return "";
        };

        //
        // Create a material class from a set of shaders and default parameters to the
        // uniforms.
        //
        MaterialClassPtr& spMaterialClass = mMaterialClasses[className];
        if (!spMaterialClass)
        {
            std::string graphSource = conditionalLoad(base, className, "shader.graph");
            std::string vertSource  = conditionalLoad(base, className, "shader.vert");
            std::string geomSource  = conditionalLoad(base, className, "shader.geom");
            std::string fragSource  = conditionalLoad(base, className, "shader.frag");
            
            GLuint     prog = GL_NONE;
            lx0::lxvar params = lxvar::map();

            if (graphSource.empty())
            {
                //
                // Create the shader from the source strings.  Note that it's okay
                // for the geometry shader to be empty.
                //
                prog = _createProgramVGF(vertSource, geomSource, fragSource);

                //
                // Create the default instance parameters.
                //
                std::string json = conditionalLoad(base, className, "material.json");
                if (!json.empty())
                {
                    lxvar material = lx0::lxvar::parse(json.c_str());            
                    params = material["parameters"];
                }
            }
            else
            {
                //
                // The generated shader builder material contains the fragment shader
                // source code, a name uniquely identifying the graph setup (effectively
                // a unique hash to determine duplicate graphs), and a set of parameters
                // for the shader instance.
                //
                lx0::ShaderBuilder::Material material;
                auto json = lx0::lxvar::parse(graphSource.c_str());
                mShaderBuilder.buildShaderGLSL(material, json["graph"]);

                //
                // ShaderBuilder GLSL use fixed vertex and geometry shaders
                //
                prog = _createProgramVGF(
                    lx0::string_from_file("media2/shaders/glsl/vertex/basic_01.vert"),
                    lx0::string_from_file("media2/shaders/glsl/geometry/basic_01.geom"),
                    material.source
                );
                params = material.parameters;

                //
                // Due to differences between the material system and the ShaderBuilder
                // (written at different times), the shader builder parameters also include
                // the parameter data types - which are not needed and should be stripped
                // out before being set on the MaterialClass.
                //
                for (auto it = params.begin(); it != params.end(); ++it)
                    params[it.key()] = (*it)[1];

            }
            lx_check_error(prog != GL_NONE);
            
            spMaterialClass.reset( new MaterialClass(prog) );
            spMaterialClass->mName = className;
            spMaterialClass->mDefaults = params;
        }
        
        //
        // Unfortunately, the "state" is currently stored on the Material,
        // not the MaterialClass - so material.json needs to be loaded again.
        //
        lx0::lxvar state = lxvar::undefined();
        std::string json = conditionalLoad(base, className, "material.json");
        if (!json.empty())
        {
            lxvar material = lx0::lxvar::parse(json.c_str());            
            state = material["state"];            
        }        
        
        lx0::lxvar params = lxvar::map();
        std::string instanceJson = conditionalLoad(base, className, lx0::_lx_format("instance-%1%.json", instanceName));
        if (!instanceJson.empty())
        {
            lxvar material = lx0::lxvar::parse(instanceJson.c_str());            
            params = material["parameters"];            
        }        
        
        spMaterial = spMaterialClass->createInstance(params);
        spMaterial->mName = materialName;
        spMaterial->mParameters = params;

        if (state.is_defined())
        {
            for (auto it = state.begin(); it != state.end(); ++it)
            {
                if (it.key() == "blend")
                    spMaterial->mBlend = (*it).as<bool>();
                else if (it.key() == "ztest")
                    spMaterial->mZTest = (*it).as<bool>();
            }
        }
    }

    return spMaterial;
}

GeometryPtr
RasterizerGL::acquireGeometry (std::string name)
{
    GeometryPtr& spGeometry = mGeometryCache[name];
    if (!spGeometry)
    {
        std::string filename = "media2/geometry/" + name + ".json";
        lx0::lxvar json = lx0::lxvar::parse( lx0::string_from_file(filename).c_str() );

        primitive_buffer prim = json.convert();
        spGeometry = createGeometry(prim);
    }
    return spGeometry;
}

/*
    This is somewhat of a reminant of old code that doesn't quite fit, but hasn't
    outlived its usefulness.

    The idea is that many fragment shaders can share the same standard vertex/geometry
    shaders, so it should be possible to create a material from the fragment shader 
    only.  This also hides the notion of a MaterialClass versus a Material from the
    caller - which is useful too in many cases (i.e. the common 1:1 class <-> instance 
    case).

    It might be better if this eventually takes an lxvar descriptor.
 */
MaterialPtr 
RasterizerGL::createMaterial (std::string name, std::string fragmentSource, lx0::lxvar parameters)
{
    GLuint prog = _createProgramVGF(
        lx0::string_from_file("media2/shaders/glsl/vertex/basic_01.vert"),
        lx0::string_from_file("media2/shaders/glsl/geometry/basic_01.geom"),
        fragmentSource
    );
    
    MaterialClassPtr spMatClass(new MaterialClass(prog));
    return spMatClass->createInstance(parameters);
}

MaterialPtr
RasterizerGL::createVertexColorMaterial (void)
{
    MaterialClassPtr& spMatClass = mMaterialClasses["VertexColor"];
    if (!spMatClass)
    {
        GLuint prog = _createProgramVGF(
            lx0::string_from_file("media2/shaders/glsl/vertex/basic_01.vert"),
            lx0::string_from_file("media2/shaders/glsl/geometry/basic_01.geom"),
            lx0::string_from_file("media2/shaders/glsl/fragment/vertexColor.frag")
        );
        spMatClass.reset(new MaterialClass(prog));
    }

    lxvar params = lxvar::map();
    return spMatClass->createInstance(params);
}

MaterialPtr 
RasterizerGL::createPhongMaterial (const glgeom::material_phong_f& mat)
{
    MaterialClassPtr& spMatClass = mMaterialClasses["Phong"];
    if (!spMatClass)
    {
        GLuint prog = _createProgramVGF(
            lx0::string_from_file("media2/shaders/glsl/vertex/basic_01.vert"),
            lx0::string_from_file("media2/shaders/glsl/geometry/basic_01.geom"),
            lx0::string_from_file("media2/shaders/glsl/fragment/phong2.frag")
        );
        spMatClass.reset(new MaterialClass(prog));
    }
    
    lxvar params = lxvar::map();
    params["unifMaterialDiffuse"] = lxvar(mat.diffuse.r, mat.diffuse.g, mat.diffuse.b);
    params["unifMaterialSpecular"] = lxvar(mat.specular.r, mat.specular.g, mat.specular.b);
    params["unifMaterialSpecularEx"] = mat.specular_n;
    return spMatClass->createInstance(params);
}

GLuint      
RasterizerGL::_createProgramVGF (std::string vertShaderSource, std::string geomShaderSource, std::string fragShaderSource)
{
    //
    // An empty shader is definitely not valid.  A problem has occurred upstream in the code.
    // 
    lx_check_error(!fragShaderSource.empty());
    check_glerror();

    // Create the shader program
    //
    GLuint vs = vertShaderSource.empty() ? 0 : _createShader(vertShaderSource, GL_VERTEX_SHADER);
    GLuint gs = geomShaderSource.empty() ? 0 : _createShader(geomShaderSource, GL_GEOMETRY_SHADER);
    GLuint fs = fragShaderSource.empty() ? 0 : _createShader(fragShaderSource, GL_FRAGMENT_SHADER);

    GLuint prog = gl->createProgram();
    {
        if (vs)
            gl->attachShader(prog, vs);
        if (gs)
            gl->attachShader(prog, gs);
        if (fs)
            gl->attachShader(prog, fs);
        
        check_glerror();
            
        gl->bindAttribLocation(prog, 0, "inPosition");
    }
    check_glerror();

    _linkProgram(prog, fragShaderSource.c_str());
    gl->useProgram(prog);

    return prog;
}

void 
RasterizerGL::_linkProgram (GLuint prog, const char* pszSource)
{
    check_glerror();

    gl->linkProgram(prog);

    GLint success;
    gl->getProgramiv(prog, GL_LINK_STATUS, &success);    
    if (success != GL_TRUE)
    {
        std::vector<char> log;
        GLint maxSize;
        GLint size;

        gl->getProgramiv(prog, GL_INFO_LOG_LENGTH, &maxSize);
        log.resize(maxSize);

        gl->getProgramInfoLog(prog, maxSize, &size, &log[0]);
        log[size] = '\0';

        //
        // If the source was passed in, then output it to the log file to
        // help diagnose the error.
        //
        if (pszSource)
        {
            const char* p = pszSource;
            std::string source;
            int line = 1;
            
            source += boost::str(boost::format("%04u: ") % line);
            
            while (*p)
            {
                while (*p && *p != '\n')
                    source += *p++;
                if (*p == '\n')
                {
                    source += boost::str(boost::format("\n%04u: ") % ++line);
                    p++;
                }
            }

            lx_debug("Writing shader source to log file.");
            lx_log("%s", source);
        }

        //
        // Report the error
        //
        lx_debugger_message(std::string(&log[0]));
        lx0::error_exception e(__FILE__, __LINE__);
        e.detail("GLSL shader compilation error. See lxengine_log.html for full details.");
        e.detail("\n%s", &log[0]);
        throw e;
    }
}

GLuint 
RasterizerGL::_createShader (std::string& shaderText, GLuint type)
{
    GLuint shaderHandle = 0; 
    if (!shaderText.empty())
    {
        shaderHandle = gl->createShader(type);

        const GLchar* text = shaderText.c_str();
        gl->shaderSource(shaderHandle, 1, &text, 0);
        gl->compileShader(shaderHandle);
    }
    else
        throw lx_error_exception("Shader source empty!");

    return shaderHandle;
}

//
// OpenGL 3.2 Core Profile does not support GL_QUADS, so convert the indices to a triangle list
//
static void
createTriangleIndices (const std::vector<lx0::uint16>& quadIndices, 
                       std::vector<lx0::uint16>& triIndices)
{
    lx_check_error(quadIndices.size() % 4 == 0);
    lx_check_error(!quadIndices.empty());

    triIndices.resize( (quadIndices.size() * 3) / 2);
    
    const lx0::uint16* pSrc = &quadIndices[0];
    lx0::uint16*       pDst = &triIndices[0];    
    for (size_t i = 0; i < quadIndices.size(); i += 4)
    {
        pDst[0] = pSrc[0];  // Triangle 1
        pDst[1] = pSrc[1];
        pDst[2] = pSrc[2];
        pDst[3] = pSrc[0];  // Triangle 2
        pDst[4] = pSrc[2];
        pDst[5] = pSrc[3];

        pDst += 6;
        pSrc  += 4;
    }
}

template <typename T>
static 
GLuint 
_genArrayBuffer (GLenum target, std::vector<T>& data)
{
    GLuint id = 0;
    if (!data.empty())
    {
        gl->genBuffers(1, &id);
        gl->bindBuffer(target, id);
        gl->bufferData(target, data.size() * sizeof(T), &data.front(), GL_STATIC_DRAW);
    }
    return id;
}

template <typename T>
static
std::vector<T>
_expandQuadsToTris (std::vector<T>& quads)
{
    std::vector<T> tris;
    tris.reserve( quads.size() * 3 / 2);
        
    auto it = quads.begin(); 
    while (it != quads.end())
    {
        auto v0 = *it++;
        auto v1 = *it++;
        auto v2 = *it++;
        auto v3 = *it++;

        tris.push_back(v0);
        tris.push_back(v1);
        tris.push_back(v2);
        tris.push_back(v0);
        tris.push_back(v2);
        tris.push_back(v3);
    }
    return tris;
}

GeometryPtr
RasterizerGL::createGeometry (glgeom::primitive_buffer& primitive)
{
    check_glerror();

    auto pGeom = new Geometry;

    // Create a vertex array to store the vertex data
    //
    GLuint vao;
    gl->genVertexArrays(1, &vao);
    gl->bindVertexArray(vao);
    
    check_glerror();

    //
    // Create the cache to encapsulate the created OGL resources
    //
    if (primitive.type == "none")
    {
        //
        // Special case of "empty" geometry.  Can be useful as a stub to send
        // through the pipeline.
        //
        pGeom->mType          = GL_NONE;
        pGeom->mCount         = 0;
    }
    else if (primitive.type == "triangles")
    {
        pGeom->mType          = GL_TRIANGLES;
        pGeom->mVao           = vao;
        pGeom->mVboIndices    = _genArrayBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive.indices);
        pGeom->mCount         = primitive.indices.size();
        pGeom->mVboPosition   = _genArrayBuffer(GL_ARRAY_BUFFER, primitive.vertex.positions);
        pGeom->mVboNormal     = _genArrayBuffer(GL_ARRAY_BUFFER, primitive.vertex.normals);
        pGeom->mVboColors     = _genArrayBuffer(GL_ARRAY_BUFFER, primitive.vertex.colors);
        for (int i = 0; i < 8 && i < (int)primitive.vertex.uv.size(); ++i)
            pGeom->mVboUVs[i] = _genArrayBuffer(GL_ARRAY_BUFFER, primitive.vertex.uv[i]);

        pGeom->mtbFlatShading = (pGeom->mVboNormal == 0);

        lx_check_error(pGeom->mCount > 0, "Only primitives of type 'none' should be empty");
    }
    else if (primitive.type == "points")
    {
        pGeom->mType          = GL_POINTS;
        pGeom->mVao           = vao;
        pGeom->mVboPosition   = _genArrayBuffer(GL_ARRAY_BUFFER, primitive.vertex.positions);
        pGeom->mCount         = primitive.vertex.positions.size();
        pGeom->mVboColors     = _genArrayBuffer(GL_ARRAY_BUFFER, primitive.vertex.colors);

        lx_check_error(pGeom->mCount > 0, "Only primitives of type 'none' should be empty");
    }
    else if (primitive.type == "lines")
    {
        pGeom->mType          = GL_LINES;
        pGeom->mVao           = vao;
        pGeom->mVboPosition   = _genArrayBuffer(GL_ARRAY_BUFFER, primitive.vertex.positions);
        pGeom->mCount         = primitive.vertex.positions.size();
        pGeom->mVboColors     = _genArrayBuffer(GL_ARRAY_BUFFER, primitive.vertex.colors);

        lx_check_error(pGeom->mCount > 0, "Only primitives of type 'none' should be empty");
    }
    else if (primitive.type == "quads")
    {
        pGeom->mType          = GL_TRIANGLES;
        pGeom->mVao           = vao;
        
        if (primitive.indices.empty())
        {
            pGeom->mVboPosition   = _genArrayBuffer(GL_ARRAY_BUFFER, _expandQuadsToTris(primitive.vertex.positions));
            pGeom->mCount         = primitive.vertex.positions.size() * 3 / 2;
            pGeom->mVboColors     = _genArrayBuffer(GL_ARRAY_BUFFER, _expandQuadsToTris(primitive.vertex.colors));

            for (int i = 0; i < 8 && i < (int)primitive.vertex.uv.size(); ++i)
                pGeom->mVboUVs[i] = _genArrayBuffer(GL_ARRAY_BUFFER, _expandQuadsToTris(primitive.vertex.uv[i]));
        }
        else
        {
            pGeom->mVboIndices    = _genArrayBuffer(GL_ELEMENT_ARRAY_BUFFER, _expandQuadsToTris(primitive.indices));
            pGeom->mCount         = primitive.indices.size() * 3 / 2;

            pGeom->mVboPosition   = _genArrayBuffer(GL_ARRAY_BUFFER, primitive.vertex.positions);
            pGeom->mVboColors     = _genArrayBuffer(GL_ARRAY_BUFFER, primitive.vertex.colors);
            for (int i = 0; i < 8 && i < (int)primitive.vertex.uv.size(); ++i)
                pGeom->mVboUVs[i] = _genArrayBuffer(GL_ARRAY_BUFFER, primitive.vertex.uv[i]);
        }

        lx_check_error(pGeom->mCount > 0, "Only primitives of type 'none' should be empty");
    }
    else
        throw lx_error_exception("Unknown primitive type '%s'", primitive.type);

    check_glerror();
    
    return GeometryPtr(pGeom);
}

GeometryPtr
RasterizerGL::createQuadList (const std::vector<unsigned short>& quadIndices, 
                              const std::vector<lx0::uint8>& faceFlags,
                              const std::vector<glgeom::point3f>& positions, 
                              const std::vector<glgeom::vector3f>& normals,
                              const std::vector<glgeom::color3f>& colors)
{
    check_glerror();

    lx_check_error(!positions.empty(), "Invalid request to create empty quad list");
    lx_check_error(quadIndices.size() == faceFlags.size() * 4 || faceFlags.empty(), 
        "Expected a single flag per quad.  Count mismatch (%u indicies, %u flags).",
        quadIndices.size(), faceFlags.size());

    // Create a vertex array to store the vertex data
    //
    GLuint vao;
    gl->genVertexArrays(1, &vao);
    gl->bindVertexArray(vao);

    // Create the index array
    std::vector<lx0::uint16> triIndices;
    createTriangleIndices(quadIndices, triIndices);

    GLuint vio;
    gl->genBuffers(1, &vio);
    gl->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio);
    gl->bufferData(GL_ELEMENT_ARRAY_BUFFER, triIndices.size() * sizeof(triIndices[0]), &triIndices[0], GL_STATIC_DRAW);

    GLuint vboPositions;
    gl->genBuffers(1, &vboPositions);
    gl->bindBuffer(GL_ARRAY_BUFFER, vboPositions);
    gl->bufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]), &positions[0], GL_STATIC_DRAW);
    
    GLuint vboNormals;
    gl->genBuffers(1, &vboNormals);
    gl->bindBuffer(GL_ARRAY_BUFFER, vboNormals);
    gl->bufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), &normals[0], GL_STATIC_DRAW);

    GLuint vboColors;
    gl->genBuffers(1, &vboColors);
    gl->bindBuffer(GL_ARRAY_BUFFER, vboColors);
    gl->bufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors[0]), &colors[0], GL_STATIC_DRAW);

    check_glerror();

    //
    // Convert the face flags.  Need to double the number of faces (since going from a quad to tri)
    // and force the flag to a 0 or 255 value to indicate flat or smooth shading; eventually these
    // flags might have additional purposes.
    //
    GLuint texFlags;
    GLuint faceCount;

    if (!faceFlags.empty() && faceFlags.size() <= 2048)
    {
        std::vector<lx0::uint8> flags(faceFlags.size() * 2);
        for (size_t i = 0; i < faceFlags.size(); ++i)
        {
            const auto v = (faceFlags[i] != 0) ? 255 : 0; 
            flags[i * 2 + 0] = v;
            flags[i * 2 + 1] = v;
        }

        gl->genTextures(1, &texFlags);
        gl->bindTexture(GL_TEXTURE_1D, texFlags);
        gl->texImage1D(GL_TEXTURE_1D, 0, 1, flags.size(), 0, GL_RED, GL_UNSIGNED_BYTE, &flags[0]);
        check_glerror();
        gl->texParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    gl->texParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        gl->texParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    gl->texParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        check_glerror();

        faceCount = flags.size();
    }
    else
    {
        //@todo Handle face flags properly
        /*
            This is essentially laziness: the face flags should be opt-in, not
            determined by the overal face count.  Also, if the face count is
            too high, a 2D texture should be used.
         */
        lx_warn("Skipping face flags since the data size is too large!");
        texFlags = 0;
        faceCount = 0;
    }

    // Create the cache to encapsulate the created OGL resources
    //
    auto pGeom = new Geometry;
    pGeom->mType        = GL_TRIANGLES;
    pGeom->mVao         = vao;
    pGeom->mVboIndices  = vio;
    pGeom->mCount       = triIndices.size();
    pGeom->mVboPosition = vboPositions;
    pGeom->mVboNormal   = vboNormals;
    pGeom->mVboColors   = vboColors;
    pGeom->mTexFlags    = texFlags;
    pGeom->mFaceCount   = faceCount;
    
    lx_check_error(pGeom->mType != GL_INVALID_ENUM);    
    return GeometryPtr(pGeom);
}

LightPtr  
RasterizerGL::createLight (void)
{
    return LightPtr(new Light);
}

LightPtr  
RasterizerGL::createLight (const glgeom::point_light_f& light)
{
    return LightPtr(new Light(light));
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
RasterizerGL::createTransform (glm::mat4& projMat, glm::mat4& viewMat, glm::mat4& modelMat)
{
    struct Class : public Transform
    {
        Class (glm::mat4& projMat, glm::mat4& viewMat, glm::mat4& modelMat)
            : mProjMat  (projMat)
            , mViewMat  (viewMat)
            , mModelMat (modelMat)
        {
        }

        virtual void activate(RasterizerGL* pRasterizer, CameraPtr spCamera)
        {
            //
            //@todo Separate View and Model matrices in uniforms!
            //
            auto& uniforms = pRasterizer->mContext.uniforms;            
            uniforms.spProjMatrix.reset(new glm::mat4(mProjMat));
            uniforms.spViewMatrix.reset(new glm::mat4(mViewMat * mModelMat));            
        }

        glm::mat4 mProjMat;
        glm::mat4 mViewMat;
        glm::mat4 mModelMat;
    };

    TransformPtr spTransform(new Class(projMat, viewMat, modelMat));
    spTransform->mat = glm::mat4();
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

    virtual void activate(RasterizerGL* pRasterizer, CameraPtr spCamera)
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
        
        glm::mat4 viewMatrix(*pRasterizer->mContext.uniforms.spViewMatrix);
        viewMatrix = viewMatrix * mat;
        viewMatrix = glm::rotate(viewMatrix, glgeom::degrees(z_angle).value, glm::vec3(0.0f, 0.0f, 1.0f));
        pRasterizer->mContext.uniforms.spViewMatrix.reset(new glm::mat4(viewMatrix));
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
    ///@todo Move billboarding function into glgeom:
    /// mat = billboard_z_axis(view_matrix);
    virtual void activate(RasterizerGL* pRasterizer, CameraPtr spCamera)
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

        glm::mat4 viewMatrix(*pRasterizer->mContext.uniforms.spViewMatrix);
        viewMatrix = viewMatrix * mat;
        viewMatrix = glm::rotate(viewMatrix, radians * 180.0f / 3.1415926f, glm::vec3(0.0f, 0.0f, 1.0f));
        pRasterizer->mContext.uniforms.spViewMatrix.reset(new glm::mat4(viewMatrix));
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
Transform::activate (RasterizerGL* pRasterizer, CameraPtr)
{
    auto m = (*pRasterizer->mContext.uniforms.spViewMatrix) * mat;
    pRasterizer->mContext.uniforms.spViewMatrix.reset(new glm::mat4(m));
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
            if (!lx0::file_is_open(spTex->mFilename))
            {
                spTex->unload();
                spTex->load();
            }
        }
    }
}

void 
RasterizerGL::beginFrame (RenderAlgorithm& algorithm)
{
    lx0::ProfileSection section(profile.beginFrame);

    mStats.tmScene.start();

    gl = this->gl3_2.get();
    
    mFrameNum++;
    mFrameData = FrameData();
    mContext.frame = FrameContext();

    lx_check_error( gl->getError() == GL_NO_ERROR );

    // Should the clear actually be part of the GlobalPass?  Additionally to this?
    const auto& color = algorithm.mClearColor;
    gl->clearColor(color.r, color.g, color.b, color.a);
    gl->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RasterizerGL::endFrame()
{
    lx0::ProfileSection section(profile.endFrame);

    check_glerror();
    mStats.tmScene.stop();
}

void 
RasterizerGL::rasterizeList (RenderAlgorithm& algorithm, std::vector<std::shared_ptr<Instance>>& list)
{
    try
    {
        lx0::ProfileSection section (profile.rasterizeList);
        TimeSection( mStats.tmRasterizeList );

        for (auto pass = algorithm.mPasses.begin(); pass != algorithm.mPasses.end(); ++pass)
        {
            auto spFBO = pass->spFrameBuffer ? pass->spFrameBuffer : mspFBOScreen;

            mContext.itemId = 0;
            mContext.pGlobalPass = &(*pass);
            mContext.spFBO = spFBO;
            mContext.sourceFBOTexture = GL_NONE;

            spFBO->activate();

            if (pass->spSourceFBO)
            {
                mContext.sourceFBOTexture = pass->spSourceFBO->textureId();

                //
                // Render a full screen quad textured with the given FBO
                //
                InstancePtr spInstance(new Instance);
                spInstance->spGeometry = acquireGeometry("basic2d/FullScreenQuad");
                spInstance->spMaterial = pass->spMaterial ? pass->spMaterial : acquireMaterial("BlitFBO");
                spInstance->spTransform = createTransform(glm::mat4(), glm::mat4(), glm::mat4());

                rasterizeItem(*pass, spInstance);
            }
            else
            {
                //
                // Is this not the default framebuffer?  Assume a clear is necessary -
                // eventually there should be a bClear option on the pass object.
                //
                if (spFBO->textureId() != 0)
                {
                    glgeom::color4f color;
                    if (pass->optClearColor.first)
                        color = pass->optClearColor.second;
                    else
                        color = algorithm.mClearColor;

                    gl->clearColor(color.r, color.g, color.b, color.a);
                    gl->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                }

                //
                // Render the list of Instances
                //
                for (auto it = list.begin(); it != list.end(); ++it)
                {
                    auto spInstance = *it;
                    if (spInstance)
                    {
                        rasterizeItem(*pass, spInstance);
                    }
                    else
                    {
                        throw lx_error_exception("Null Instance in rasterization list");
                    }
                    mContext.itemId ++;
                }
            }

            mContext.spFBO.reset();
            mContext.pGlobalPass = nullptr;
        }
    }
    catch (lx0::error_exception& e)
    {
        e.location(__FILE__, __LINE__);
        e.detail("Exception thrown during rasterizeList.  Rasterizer potentially in unknown state.");
        throw;
    }
}

__forceinline static
void _timeCall (lx0::Timer& timer, std::function<void (void)> f)
{
    timer.start();
    f();
    timer.stop();
}

void 
RasterizerGL::rasterizeItem (GlobalPass& pass, std::shared_ptr<Instance> spInstance)
{
    lx0::ProfileSection section (profile.rasterizeItem);
    TimeSection( mStats.tmRasterizeItem );

    lx_check_error(spInstance.get() != nullptr);

    // Set up the context variables that have changed
    mContext.spInstance = spInstance;
    mContext.spGeometry = spInstance->spGeometry;
    mContext.textureUnit = 0;
    mContext.uniforms.reset();

    //
    // Fill in any unspecified or overridden item variables via the current context
    //
    // As much of the resolution of what *will* be used should be done before any
    // of these objects are "activated" (i.e. the GL calls to set this up are made).
    // Knowing what we'll be attempting before attempting it helps make sure
    // everything can and will be done correctly.
    //
    mContext.spCamera = (spInstance->spCamera) 
        ? spInstance->spCamera
        : pass.spCamera;
    mContext.spLightSet = (spInstance->spLightSet)
        ? spInstance->spLightSet
        : pass.spLightSet;
    mContext.spMaterial = (pass.spMaterial) 
        ? pass.spMaterial
        : spInstance->spMaterial;
    mContext.spTransform = spInstance->spTransform;

    mContext.tbFlatShading = boost::indeterminate(spInstance->spGeometry->mtbFlatShading) 
        ? pass.tbFlatShading
        : spInstance->spGeometry->mtbFlatShading;

    //
    // Resolve incompatibilities by swapping in defaults for invalid parameters.
    // Add warnings and errors as appropriate if there is not a reasonable default
    // that can be substituted in.
    //
    lx_check_error(spInstance->spGeometry);
    lx_check_error(spInstance->spGeometry->mType != GL_INVALID_ENUM);

    // The requested material does not support the type of geometry that is being
    // rendered.  Use the default material for that geometry instead.
    switch (spInstance->spGeometry->mType)
    {
    case GL_TRIANGLES:
        if (!mContext.spMaterial)
            mContext.spMaterial = acquireMaterial("DefaultSurface");
        break;
    case GL_LINES:
        mContext.spMaterial = acquireMaterial("DefaultLine");
        break;
    case GL_POINTS:
        mContext.spMaterial = acquireMaterial("DefaultPoint");
        break;
    default:
        throw lx_error_exception("Incompatible material for geometry type.  Could not determine a alternate material to use.");
    };

    //
    // Error checking
    //
    {
        if (!mContext.spCamera)
            throw lx_error_exception("No camera set! Set a camera either on the InstancePtr or the GlobalPass.");
    }

    //
    // Activate the item
    //
    {
        check_glerror();
        lx_check_error( mContext.spCamera );
        lx_check_error( mContext.spMaterial );

        mContext.spCamera->activate(this);
        
        // Lights are optional; not all rendering algorithms require explicitly defined lights
        _timeCall(mStats.tmLightSetActivate, [&]() {
            if (mContext.spLightSet)
                mContext.spLightSet->activate(this);
        });
        
        _timeCall(mStats.tmTransformActivate, [&]() { 
            if (mContext.spTransform)
                mContext.spTransform->activate(this, spInstance->spCamera); 
        });

        _timeCall(mStats.tmMaterialActivate, [&]() {              
            mContext.spMaterial->activate(this, pass);
        });
        _timeCall(mStats.tmGeometryActivate, [&]() { mContext.spGeometry->activate(this, pass); });
    
        check_glerror();
    }

    mContext.spCamera.reset();
    mContext.spLightSet.reset();
    mContext.spMaterial.reset();
    mContext.spTransform.reset();
    mContext.spInstance.reset();
    mContext.spGeometry.reset();
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
	gl->getIntegerv(GL_VIEWPORT,viewport);
    y = viewport[3] - y;
	
    // Read the pixel
    gl->readBuffer(GL_BACK);
    GLubyte pixel[4];
    gl->readPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)pixel);

    unsigned int id = (pixel[0] << 16) + (pixel[1] << 8) + pixel[2];
    return id;
}

void
RasterizerGL::readFrontBuffer (glgeom::image3f& img)
{
    _readBuffer(GL_FRONT, img);
}

void
RasterizerGL::readBackBuffer (glgeom::image3f& img)
{
    _readBuffer(GL_BACK, img);
}

void        
RasterizerGL::_readBuffer (GLenum buffer, glgeom::image3f& img)
{
    check_glerror();

    // Flip y since window coordinates differ from OpenGL's 0 at the bottom convention 
    GLint viewport[4];
	gl->getIntegerv(GL_VIEWPORT,viewport);
	
    img = glgeom::image3f(viewport[2], viewport[3]);

    // Read the pixel
    gl->readBuffer(buffer);
    std::vector<GLubyte> pixel(4 * img.width() * img.height());
    gl->readPixels(0, 0, img.width(), img.height(), GL_RGBA, GL_UNSIGNED_BYTE, (void *)&pixel[0]);

    GLubyte* p = &pixel[0];
    for (int y = 0; y < img.height(); ++y)
    {
        for (int x = 0; x < img.width(); ++x)
        {
            // Invert the y-coord since the GL indexing is y = 0 = bottom
            glgeom::color3f color;
            color.r = (*p++) / 255.0f;
            color.g = (*p++) / 255.0f;
            color.b = (*p++) / 255.0f;
            img.set(x, img.height() - y - 1,  color);
            p++;
        }
    }

    check_glerror();
}

//===========================================================================//

void
RenderList::push_back (int layer, InstancePtr spInstance)
{
    mLayers[layer].list.push_back(spInstance);
}

InstancePtr 
RenderList::getInstance (unsigned int id)
{
    auto it = mLayers.begin();

    while (it->second.list.size() < id)
    {
        id -= it->second.list.size();
        it++;
    }
    return it->second.list[id];
}
