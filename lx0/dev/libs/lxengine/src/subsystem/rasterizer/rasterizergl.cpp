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
#include <boost/format.hpp>

#include <lx0/prototype/misc.hpp>
#include <lx0/util/misc/util.hpp>
#include <lx0/subsystem/rasterizer.hpp>

#include <glm/gtc/matrix_inverse.hpp>

#include <gl/glu.h>

using namespace lx0::subsystem::rasterizer_ns;
using namespace glgeom;

void lx0::subsystem::rasterizer_ns::check_glerror()
{
	GLenum errCode = glGetError();
    lx_check_error( errCode == GL_NO_ERROR, "glError() == %d: %s", errCode, gluErrorString(errCode) );
}

RasterizerGL::RasterizerGL()
    : gl        (new GLInterface)
    , mInited   (false)
    , mShutdown (false)
    , mFrameNum (0)
{
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

    // Initialization
    //
    lx_log("Using OpenGL v%s", (const char*)glGetString(GL_VERSION));
    lx_log("Using GLSL v%s", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    glClearColor(0.09f, 0.09f, 0.11f, 1.0f);

    glEnable(GL_DEPTH_TEST);
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

TexturePtr      
RasterizerGL::createTextureCubeMap (const char* xpos, const char* xneg, const char* ypos, const char* yneg, const char* zpos, const char* zneg)
{
    check_glerror();

    glEnable(GL_TEXTURE_CUBE_MAP);

    //
    // The filenames[] array order needs to match the expected OpenGL target sequence
    //
    GLuint              id;
    const char*         filenames[] = { xpos, xneg, ypos, yneg, zpos, zneg };
    GLenum              target      = GL_TEXTURE_CUBE_MAP_POSITIVE_X;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    for (int i = 0; i < 6; ++i, ++target)
    {
        Image4b img;
        lx0::load_png(img, filenames[i]);
        
        glTexImage2D(target, 0, GL_RGBA, img.mWidth, img.mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.mData.get());
    }

    auto pTexture = new Texture;
    pTexture->mId = id;

    check_glerror();

    return TexturePtr(pTexture);
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
        lx_error("Load of DDS stream failed");
    
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
RasterizerGL::createMaterial (std::string fragShader)
{
    GLuint prog = _createProgramFromFile(fragShader);
    MaterialPtr spMat(new Material(prog));
    spMat->mShaderFilename = fragShader;
    return spMat;
}

MaterialPtr 
RasterizerGL::createMaterial (std::string name, std::string fragmentSource, lx0::lxvar parameters)
{
    GLuint prog = _createProgram(name, fragmentSource);
    
    auto pMat = new GenericMaterial(prog);
    pMat->mShaderFilename = name;
    pMat->mParameters = parameters.clone();
 
    return MaterialPtr(pMat);
}

MaterialPtr     
RasterizerGL::createSolidColorMaterial (const color3f& rgb)
{
    GLuint prog = _createProgramFromFile("media2/shaders/glsl/fragment/solid.frag");

    auto pMat = new SolidColorMaterial(prog);
    pMat->mShaderFilename = "media2/shaders/glsl/fragment/solid.frag";
    pMat->mColor = rgb;
    
    return MaterialPtr(pMat);
}

MaterialPtr
RasterizerGL::createVertexColorMaterial (void)
{
    GLuint prog = _createProgramFromFile("media2/shaders/glsl/fragment/vertexColor.frag");

    auto pMat = new VertexColorMaterial(prog);
    pMat->mShaderFilename = "media2/shaders/glsl/fragment/vertexColor.frag";
    
    return MaterialPtr(pMat);
}

MaterialPtr 
RasterizerGL::createPhongMaterial (const glgeom::material_phong_f& mat)
{
    GLuint prog = _createProgramFromFile("media2/shaders/glsl/fragment/phong2.frag");

    auto pMat = new PhongMaterial(prog);
    pMat->mShaderFilename = "media2/shaders/glsl/fragment/phong2.frag";
    pMat->mPhong = mat;
    
    return MaterialPtr(pMat);
}

GLuint 
RasterizerGL::_createProgramFromFile  (std::string filename)
{
    return _createProgram(filename, lx0::string_from_file(filename));
}

GLuint 
RasterizerGL::_createProgram   (std::string uniqueId, std::string& source)
{
    static int s_anonymousId = 0;
    if (uniqueId.empty())
        uniqueId = boost::str( boost::format("_anonymous%04d") % s_anonymousId );

    auto it = mCachePrograms.find(uniqueId);
    if (it != mCachePrograms.end())
    {
        return it->second;
    }
    else
    {
        lx_debug("Creating program for shader '%s'", uniqueId.c_str());

        GLuint id = _createProgram2(source);
        mCachePrograms.insert(std::make_pair(uniqueId, id));
        return id;
    }
}

GLuint 
RasterizerGL::_createProgram2  (std::string fragmentSource)
{
    //
    // An empty shader is definitely not valid.  A problem has occurred upstream in the code.
    // 
    lx_check_error(!fragmentSource.empty());

    check_glerror();

    // Create the shader program
    //
    GLuint vs = _createShader("media2/shaders/glsl/vertex/basic_01.vert", GL_VERTEX_SHADER);
    GLuint gs = _createShader("media2/shaders/glsl/geometry/basic_01.geom", GL_GEOMETRY_SHADER);
    GLuint fs = _createShader2(fragmentSource, GL_FRAGMENT_SHADER);

    GLuint prog = glCreateProgram();
    {
        glAttachShader(prog, vs);
        glAttachShader(prog, gs);
        glAttachShader(prog, fs);
        
        check_glerror();
            
        glBindAttribLocation(prog, 0, "inPosition");
    }
    check_glerror();

    _linkProgram(prog, fragmentSource.c_str());
    glUseProgram(prog);

    return prog;
}

void 
RasterizerGL::_linkProgram (GLuint prog, const char* pszSource)
{
    check_glerror();

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
            lx_debug(boost::str(boost::format("%s") % source));
        }

        const char* text = &log[0];
        lx_error("Shader compilation error! See lxengine_log.html for details.  '%s'", &log[0]);            
    }
}

GLuint 
RasterizerGL::_createShader(const char* filename, GLuint type)
{
    std::string shaderText = lx0::string_from_file(filename);
    if (shaderText.empty())
        lx_error("Could not load shader '%s' (file exists = %s)", filename, lx0::file_exists(filename) ? "true" : "false");
   
    return _createShader2(shaderText, type);
}

GLuint 
RasterizerGL::_createShader2 (std::string& shaderText, GLuint type)
{
    GLuint shaderHandle = 0; 
    if (!shaderText.empty())
    {
        shaderHandle = glCreateShader(type);

        const GLchar* text = shaderText.c_str();
        glShaderSource(shaderHandle, 1, &text, 0);

        glCompileShader(shaderHandle);
    }
    else
        lx_error("Shader source empty!");

    return shaderHandle;
}

GeometryPtr
RasterizerGL::createQuadList (std::vector<glgeom::point3f>& quadPositions, 
                              std::vector<glgeom::color3f>& quadColors)
{
    lx_check_error(!quadPositions.empty());
    lx_check_error(quadPositions.size() == quadColors.size());

    check_glerror();

    // Convert the quads to triangles - OpenGL 3.2+ doesn't support quads natively
    std::vector<glgeom::point3f> positions ((quadPositions.size() * 3) / 2);
    std::vector<glgeom::color3f> colors ((quadColors.size() * 3) / 2);
    {
        const auto* pSrcP = &quadPositions[0];
        const auto* pSrcC = &quadColors[0];
        auto* pDstP = &positions[0];
        auto* pDstC = &colors[0];

        for (size_t i = 0; i < quadPositions.size(); i += 4)
        {
            pDstP[0] = pSrcP[0];
            pDstP[1] = pSrcP[1];
            pDstP[2] = pSrcP[2];
            pDstP[3] = pSrcP[0];
            pDstP[4] = pSrcP[2];
            pDstP[5] = pSrcP[3];

            pDstC[0] = pSrcC[0];
            pDstC[1] = pSrcC[1];
            pDstC[2] = pSrcC[2];
            pDstC[3] = pSrcC[0];
            pDstC[4] = pSrcC[2];
            pDstC[5] = pSrcC[3];

            pDstP += 6;
            pDstC += 6;
            pSrcP += 4;
            pSrcC += 4;
        }
    }


    // Create a vertex array to store the vertex data
    //
    GLuint vao;
    gl->genVertexArrays(1, &vao);
    gl->bindVertexArray(vao);

    GLuint vboPositions;
    gl->genBuffers(1, &vboPositions);
    gl->bindBuffer(GL_ARRAY_BUFFER, vboPositions);
    gl->bufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]), &positions[0], GL_STATIC_DRAW);
    
    GLuint vboColors;
    gl->genBuffers(1, &vboColors);
    gl->bindBuffer(GL_ARRAY_BUFFER, vboColors);
    gl->bufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors[0]), &colors[0], GL_STATIC_DRAW);

    check_glerror();

    // Create the cache to encapsulate the created OGL resources
    //
    auto pGeom = new GeomImp;
    pGeom->mtbFlatShading = true;
    pGeom->mType        = GL_TRIANGLES;
    pGeom->mVao         = vao;
    pGeom->mCount       = positions.size();
    pGeom->mVboPosition = vboPositions;
    pGeom->mVboColors   = vboColors;
    return GeometryPtr(pGeom);
}

//
// OpenGL 3.2 Core Profile does not support GL_QUADS, so convert the indices to a triangle list
//
static void
createTriangleIndices (std::vector<lx0::uint16>& quadIndices, std::vector<lx0::uint16>& triIndices)
{
    lx_check_error(quadIndices.size() % 4 == 0);
    lx_check_error(!quadIndices.empty());

    triIndices.resize( (quadIndices.size() * 3) / 2);
    
    lx0::uint16* pDst = &triIndices[0];
    lx0::uint16* pSrc = &quadIndices[0];
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

GeometryPtr
RasterizerGL::createQuadList (std::vector<lx0::uint16>&     quadIndices,
                              std::vector<glgeom::point3f>& positions)
{
    std::vector<lx0::uint16> triIndices;
    createTriangleIndices(quadIndices, triIndices);

    check_glerror();

    // Create a vertex array to store the vertex data
    //
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vio;
    glGenBuffers(1, &vio);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triIndices.size() * sizeof(triIndices[0]), &triIndices[0], GL_STATIC_DRAW);

    GLuint vboPositions;
    glGenBuffers(1, &vboPositions);
    glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]), &positions[0], GL_STATIC_DRAW);
    
    check_glerror();

    // Create the cache to encapsulate the created OGL resources
    //
    auto pGeom = new GeomImp;
    pGeom->mtbFlatShading = true;
    pGeom->mType          = GL_TRIANGLES;
    pGeom->mVao           = vao;
    pGeom->mVboIndices    = vio;
    pGeom->mCount         = triIndices.size();
    pGeom->mVboPosition   = vboPositions;
    return GeometryPtr(pGeom);

}

template <typename T>
static 
GLuint 
_genArrayBuffer (GLenum target, std::vector<T>& data)
{
    GLuint id = 0;
    if (!data.empty())
    {
        glGenBuffers(1, &id);
        glBindBuffer(target, id);
        glBufferData(target, data.size() * sizeof(T), &data.front(), GL_STATIC_DRAW);
    }
    return id;
}


GeometryPtr
RasterizerGL::createGeometry (glgeom::primitive_buffer& primitive)
{
    check_glerror();

    // Create a vertex array to store the vertex data
    //
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    check_glerror();

    // Create the cache to encapsulate the created OGL resources
    //
    auto pGeom = new GeomImp;
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
    check_glerror();

    return GeometryPtr(pGeom);
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
RasterizerGL::createQuadList (std::vector<unsigned short>& quadIndices, 
                              std::vector<lx0::uint8>& faceFlags,
                              std::vector<glgeom::point3f>& positions, 
                              std::vector<glgeom::vector3f>& normals,
                              std::vector<glgeom::color3f>& colors)
{
    check_glerror();

    lx_check_error(quadIndices.size() == faceFlags.size() * 4, 
        "Expected a single flag per quad.  Count mismatch (%u indicies, %u flags).",
        quadIndices.size(), faceFlags.size());

    // Create a vertex array to store the vertex data
    //
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create the index array
    std::vector<lx0::uint16> triIndices;
    createTriangleIndices(quadIndices, triIndices);

    GLuint vio;
    glGenBuffers(1, &vio);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triIndices.size() * sizeof(triIndices[0]), &triIndices[0], GL_STATIC_DRAW);

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

    check_glerror();

    //
    // Convert the face flags.  Need to double the number of faces (since going from a quad to tri)
    // and force the flag to a 0 or 255 value to indicate flat or smooth shading; eventually these
    // flags might have additional purposes.
    //
    GLuint texFlags;
    GLuint faceCount;

    if (faceFlags.size() <= 2048)
    {
        std::vector<lx0::uint8> flags(faceFlags.size() * 2);
        for (size_t i = 0; i < faceFlags.size(); ++i)
        {
            const auto v = (faceFlags[i] != 0) ? 255 : 0; 
            flags[i * 2 + 0] = v;
            flags[i * 2 + 1] = v;
        }

        glGenTextures(1, &texFlags);
        glBindTexture(GL_TEXTURE_1D, texFlags);
        glTexImage1D(GL_TEXTURE_1D, 0, 1, flags.size(), 0, GL_RED, GL_UNSIGNED_BYTE, &flags[0]);
        check_glerror();
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        check_glerror();

        faceCount = flags.size();
    }
    else
    {
        //@todo Handle face flags properly
        /*
            This is essentially laziness: the face flags should be opt-in, not
            determined by the overally face count.  Also, if the face count is
            too high, a 2D texture should be used.
         */
        lx_warn("Skipping face flags since the data size is too large!");
        texFlags = 0;
        faceCount = 0;
    }

    // Create the cache to encapsulate the created OGL resources
    //
    auto pGeom = new GeomImp;
    pGeom->mType        = GL_TRIANGLES;
    pGeom->mVao         = vao;
    pGeom->mVboIndices  = vio;
    pGeom->mCount       = triIndices.size();
    pGeom->mVboPosition = vboPositions;
    pGeom->mVboNormal   = vboNormals;
    pGeom->mVboColors   = vboColors;
    pGeom->mTexFlags    = texFlags;
    pGeom->mFaceCount   = faceCount;
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
    mStats.tmScene.start();
    
    mFrameNum++;
    mContext.frame = FrameContext();

    lx_check_error( glGetError() == GL_NO_ERROR );

    // Should the clear actually be part of the GlobalPass?  Additionally to this?
    const auto& color = algorithm.mClearColor;
    glClearColor(color.r, color.g, color.b, color.a);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RasterizerGL::endFrame()
{
    check_glerror();
    mStats.tmScene.stop();
}

void 
RasterizerGL::rasterizeList (RenderAlgorithm& algorithm, std::vector<std::shared_ptr<Instance>>& list)
{
    mStats.tmRasterizeList.start();

    for (auto pass = algorithm.mPasses.begin(); pass != algorithm.mPasses.end(); ++pass)
    {
        mContext.itemId = 0;
        mContext.pGlobalPass = &(*pass);

        for (auto it = list.begin(); it != list.end(); ++it)
        {
            auto spInstance = *it;
            if (spInstance)
            {
                rasterizeItem(*pass, *it);
            }
            else
            {
                lx_error("Null Instance in rasterization list");
            }
            mContext.itemId ++;
        }

        mContext.pGlobalPass = nullptr;
    }

    mStats.tmRasterizeList.stop();
}

void
RasterizerGL::Context::Uniforms::activate ()
{
    check_glerror();

    GLint progId;
    glGetIntegerv(GL_CURRENT_PROGRAM, &progId);

    //
    // Pass in additional transform information
    // 
    if (spProjMatrix)
    {
        GLint idx = glGetUniformLocation(progId, "unifProjMatrix");
        if (idx != -1)
        {
            glUniformMatrix4fv(idx, 1, GL_FALSE, glm::value_ptr(*spProjMatrix));
        }
    }

    //
    // Pass in additional transform information
    // 
    if (spViewMatrix)
    {
        GLint idx = glGetUniformLocation(progId, "unifViewMatrix");
        if (idx != -1)
        {
            glUniformMatrix4fv(idx, 1, GL_FALSE, glm::value_ptr(*spViewMatrix));
        }

        // The gl_NormalMatrix is the upper 3x3 of the inverse transpose of the model view matrix
        glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(*spViewMatrix));
        {
            GLint idx = glGetUniformLocation(progId, "unifNormalMatrix");
            if (idx != -1)
            {
                glUniformMatrix3fv(idx, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            }
        }
    }
    check_glerror();
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
    mStats.tmRasterizeItem.start();

    lx_check_error(spInstance.get() != nullptr);

    // Set up the context variables that have changed
    mContext.spInstance = spInstance;
    mContext.textureUnit = 0;
    mContext.uniforms.reset();

    // Fill in any unspecified or overridden item variables via the current context
    mContext.spCamera = (spInstance->spCamera) 
        ? spInstance->spCamera
        : pass.spCamera;
    mContext.spLightSet = (spInstance->spLightSet)
        ? spInstance->spLightSet
        : pass.spLightSet;
    mContext.spMaterial = (pass.bOverrideMaterial) 
        ? pass.spMaterial
        : spInstance->spMaterial;

    mContext.tbFlatShading = boost::indeterminate(spInstance->spGeometry->mtbFlatShading) 
        ? pass.tbFlatShading
        : spInstance->spGeometry->mtbFlatShading;

    //
    // Error checking
    //
    {
        if (!mContext.spCamera)
            lx_error("No camera set! Set a camera either on the InstancePtr or the GlobalPass.");
    }

    //
    // Activate the item
    //
    {
        check_glerror();

        mContext.spCamera->activate(this);
        
        _timeCall(mStats.tmMaterialActivate, [&](){  mContext.spMaterial->activate(this, pass);  });

        // Lights are optional; not all rendering algorithms require explicitly defined lights
        _timeCall(mStats.tmLightSetActivate, [&]() {
            if (mContext.spLightSet)
                mContext.spLightSet->activate(this);
        });
        
        _timeCall(mStats.tmTransformActivate, [&]() { spInstance->spTransform->activate(this, spInstance->spCamera); });

        mContext.uniforms.activate();
        
        _timeCall(mStats.tmGeometryActivate, [&]() { spInstance->spGeometry->activate(this, pass); });
    
        check_glerror();
    }

    mContext.spCamera = nullptr;
    mContext.spLightSet = nullptr;
    mContext.spMaterial = nullptr;
    mContext.spInstance = nullptr;

    mStats.tmRasterizeItem.stop();
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

void
RasterizerGL::readBackBuffer(glgeom::image3f& img)
{
    // Flip y since window coordinates differ from OpenGL's 0 at the bottom convention 
    GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	
    img = glgeom::image3f(viewport[2], viewport[3]);

    // Read the pixel
    glReadBuffer(GL_BACK);
    std::vector<GLubyte> pixel(4 * img.width() * img.height());
    glReadPixels(0, 0, img.width(), img.height(), GL_RGBA, GL_UNSIGNED_BYTE, (void *)&pixel[0]);

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
