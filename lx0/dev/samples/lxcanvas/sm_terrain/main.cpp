//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010 athile@athile.net (http://www.athile.net)

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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

// Standard headers
#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

// Library headers
#include <boost/program_options.hpp>

// Lx0 headers
#include <lx0/core.hpp>
#include <lx0/matrix.hpp>
#include <lx0/util.hpp>
#include <lx0/canvas/canvas.hpp>
#include <lx0/prototype/prototype.hpp>

#include <gl/glew.h>
#include <windows.h>        // Unfortunately must be included on Windows for GL.h to work
#include <gl/GL.h>

using namespace lx0::core;
using namespace lx0::prototype;
using namespace lx0::canvas::platform;


class HeightMap
{
public:
    HeightMap() : mSizeX(0), mSizeY(0) {}
    void resize(int x, int y) { mSizeX = x; mSizeY = y; mHeight.resize(mSizeX * mSizeY); }

    int sizeX() { return mSizeX; }
    int sizeY() { return mSizeY; }

    float& operator() (int x, int y) { 
        lx_check_error(x >= 0 && y >= 0);
        lx_check_error(x < mSizeX && y < mSizeY);
        return mHeight[y * mSizeX + x]; 
    }

    int                mSizeX;
    int                mSizeY;
    std::vector<float> mHeight;
};

Camera             gCamera;
HeightMap          gHMap;

//===========================================================================//

class RasterizerGL
{
public:
    struct Camera
    {
        virtual void activate();
        float   fov;
        float   nearDist;
        float   farDist;
        matrix4 viewMatrix;
    };
    typedef std::shared_ptr<Camera> CameraPtr;

    struct Material
    {
        virtual void activate() {}
    };
    typedef std::shared_ptr<Material> MaterialPtr;

    struct Geometry
    {
        virtual void activate() = 0;
    };
    typedef std::shared_ptr<Geometry> GeometryPtr;

    struct QuadList : public Geometry
    {
        virtual void activate();

        GLuint vao[1];
    };

    struct LightSet
    {
        virtual void activate() {}
    };
    typedef std::shared_ptr<LightSet> LightSetPtr;

    struct Transform
    {
        virtual void activate() {}
        matrix4 mat;
    };
    typedef std::shared_ptr<Transform> TransformPtr;

    class Item
    {
    public:
        virtual void rasterize();

        //weak_ptr<Target> wpTarget;
        CameraPtr    spCamera;
        TransformPtr spTransform;
        MaterialPtr  spMaterial;
        GeometryPtr  spGeometry;
        LightSetPtr  spLightSet;
    };
    typedef std::shared_ptr<Item> ItemPtr;

    void            initialize      (void);
    void            shutdown        (void);

    CameraPtr       createCamera    (float fov, float nearDist, float farDist, matrix4& viewMatrix);
    LightSetPtr     createLightSet  (void);
    MaterialPtr     createMaterial  (void);
    TransformPtr    createTransform (matrix4& mat);
    GeometryPtr     createQuadList  (std::vector<point3>& quads);

    void            beginScene      (void);
    void            endScene        (void);

    void            rasterizeList   (std::vector<std::shared_ptr<Item>>& list);
    void            rasterize       (std::shared_ptr<Item> spItem);

protected:
    GLuint  createShader    (char* filename, GLuint type);
};

void RasterizerGL::initialize()
{
    // Initialization
    //
    lx_log("Using OpenGL v%s", (const char*)glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);
}

void RasterizerGL::shutdown()
{
}


RasterizerGL::CameraPtr       
RasterizerGL::createCamera (float fov, float nearDist, float farDist, matrix4& viewMatrix)
{
    CameraPtr spCamera (new Camera);
    spCamera->fov = fov;
    spCamera->nearDist = nearDist;
    spCamera->farDist = farDist;
    spCamera->viewMatrix = viewMatrix;
    return spCamera;
}

void
RasterizerGL::Camera::activate()
{
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
    // the 16 element array is the second element in the first row); matrix4 uses
    // column-major ordering.  Therefore load it as a tranpose to swap the order.
    //
    // Note: it's also not necessarily faster to send the non-transpose, as the
    // OpenGL API does not necessarily match the underlying hardware representation.
    //
    glMatrixMode(GL_MODELVIEW);
    glLoadTransposeMatrixf(viewMatrix.data);
}

RasterizerGL::MaterialPtr 
RasterizerGL::createMaterial (void)
{
    // Create the shader program
    //
    GLuint vs = createShader("media/shaders/glsl/vertex/basic_00.vert", GL_VERTEX_SHADER);
    GLuint gs = createShader("media/shaders/glsl/geometry/basic_00.geom", GL_GEOMETRY_SHADER);
    GLuint fs = createShader("media2/shaders/glsl/fragment/checker_world_xy10.frag", GL_FRAGMENT_SHADER);

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

    glLinkProgram(prog);
    glUseProgram(prog);

    return MaterialPtr(new Material);
}

GLuint 
RasterizerGL::createShader( char* filename, GLuint type)
{
    GLuint shaderHandle = 0; 

    std::string shaderText = lx0::util::lx_file_to_string(filename);
    if (!shaderText.empty())
    {
        shaderHandle = glCreateShader(type);

        const GLchar* text = shaderText.c_str();
        glShaderSource(shaderHandle, 1, &text, 0);
        shaderText.swap(std::string());

        glCompileShader(shaderHandle);
    }
    else
        lx_error("Could not load shader '%s' (file exists = %s)", filename, lx0::util::lx_file_exists(filename) ? "true" : "false");

    return shaderHandle;
}


RasterizerGL::GeometryPtr 
RasterizerGL::createQuadList (std::vector<point3>& positionData)
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * positionData.size(), &positionData[0], GL_STATIC_DRAW);
        
    // TODO: The position input to the vertex shader is apparently always 0.  Find out where
    // this is documented.
    const int positionIndex = 0;   // glGetAttribLocation(prog, "gl_Vertex");
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionIndex);
    lx_check_error( glGetError() == GL_NO_ERROR, "OpenGL error detected." );

    auto pQuadList = new RasterizerGL::QuadList;
    pQuadList->vao[0] = vao[0];
    return GeometryPtr(pQuadList);
}

void RasterizerGL::QuadList::activate()
{
    glBindVertexArray(vao[0]);

    glDrawArrays(GL_QUADS, 0, 4 * gHMap.sizeX() * gHMap.sizeY()); 
}

RasterizerGL::LightSetPtr     
RasterizerGL::createLightSet (void)
{
    return LightSetPtr(new LightSet);
}

RasterizerGL::TransformPtr 
RasterizerGL::createTransform (matrix4& mat)
{
    return TransformPtr(new Transform);
}

void RasterizerGL::beginScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RasterizerGL::endScene()
{
}

void RasterizerGL::rasterize(std::shared_ptr<Item> spItem)
{
    spItem->rasterize();
}

void 
RasterizerGL::Item::rasterize()
{
    spCamera->activate();
    spLightSet->activate();
    spMaterial->activate();
    spTransform->activate();
    spGeometry->activate();     
}
        
//===========================================================================//

static int p[] =
{ 
    151,
    160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,
    21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
    74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,
    230,220,105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,
    80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,
    164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,
    118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,
    183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
    218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,
    145,235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,204,176,
    115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,
    141,128,195,78,66,215,61,156,180,
    151
};
  

float interpolate (float t, float a, float b)
{ 
    return a + t * (b - a); 
}

vector3 gradient (int hash) 
{
    //
    // Perlin's 2002 improvement to algorithm reduces the gradient
    // value possibilities to a vectors from the cube center
    // to one of the twelve edge centers on the cube.  The previous
    // incarnation of the algorithn used a random gradient.
    //
    // Each corner of the cube has been assigned a random, but
    // consistent hash value.   Since the hash values should be
    // uniform, the module should maintain a uniform distribution.
    //
    switch (hash % 12)
    {
    default:    lx_error("Unreachable code");

    case 0:  return vector3( 1,  1, 0);
    case 1:  return vector3(-1,  1, 0);
    case 2:  return vector3( 1, -1, 0);
    case 3:  return vector3( 1, -1, 0);
    
    case 4:  return vector3( 1,  0, 1);
    case 5:  return vector3(-1,  0, 1);
    case 6:  return vector3( 1,  0,-1);
    case 7:  return vector3( 1,  0,-1);
    
    case 8:  return vector3( 0,  1, 1);
    case 9:  return vector3( 0, -1, 1);
    case 10: return vector3( 0,  1, -1);
    case 11: return vector3( 0,  1, -1);
    }
}

float 
weight (int hash, float x, float y, float z)
{
    return dot(vector3(x, y, z), gradient(hash));
}
  
float smoothInterpolant (float t)
{
    //
    // Use the smoothing function from Perlin's algorithm:
    //    6t^5 - 15t^4 + 10t^3
    //  = t^3 * (6t^2 - 15t + 10)
    //
    return t * t * t * (6 * t * t - 15 * t + 10);
}

/*!
    Perlin Noise

    Perlin noise is essentially a random noise function and an interpolation function.
    The noise function provides incoherent data points while the interpolation function
    smoothes them out into a pseudo-random visually appealing effect.

    The random noise function needs to be repeatably random: f(x) = y, always for the
    same x.  Otherwise, the Perlin noise function will not produce smooth results 
    (the look-ups to adjacent cells will not return consistent values).

    Note that the random noise function is at regular sampling points and is not used
    as a continuous function.

    Perlin noise then interpolates between the random sampling points to createa 
    a piece-wise smooth function - which is the resulting perlin noise pseudo-random 
    pattern.

    Noise Function

    The noise function is a random number generator that will return repeatable results.
    
    The easiest such function for the integer domain is a simple look-up table 
    pre-populated with random results.

    Another such choice is the Mersenne Twister pseudorandom number generator.  This is
    a simple, fast generator that produces an almost uniform distribution for integers.

    Interpolation Function

    The interpolation function is split into a "fade" function and a simple linear
    interpolation.  The fade function manipulates the input 't' value to smooth ease-in
    and ease-out the value transition from each random sample point.

 */
float perlin_noise(float x, float y, float z) 
{
    //
    // Perlin noise starts by essentially doing a look-up into an 
    // "infinite" cubemap.  Find the cube being looked at and the
    // uvw within that cube.
    //
    int cubeX = (int)floor(x);
    int cubeY = (int)floor(y);
    int cubeZ = (int)floor(z);

    vector3 uvw = vector3(x, y, z);
    uvw.x -= cubeX;
    uvw.y -= cubeY;
    uvw.z -= cubeZ;

    //
    // Next, the "infinite" cubemap is actually a finite, repeated
    // (i.e. tiled) cubemap.  This works since the properties of the
    // noise function are such that once any repeated pattern would
    // be discernible, generally the view distance is sufficiently
    // far that the detail is lost anyway.
    //
    cubeX &= 255;
    cubeY &= 255;
    cubeZ &= 255;

    //
    // For each corner of the cube, generate a "random" but consistent
    // value for each of the corners of the cube.  This is the "hash"
    // of each corner and is used to select a gradient value.
    //
    const int i000 = p[ p[ p[cubeX  ] + cubeY  ] + cubeZ  ];
    const int i100 = p[ p[ p[cubeX+1] + cubeY  ] + cubeZ  ];
    const int i010 = p[ p[ p[cubeX  ] + cubeY+1] + cubeZ  ];
    const int i110 = p[ p[ p[cubeX+1] + cubeY+1] + cubeZ  ];
    const int i001 = p[ p[ p[cubeX  ] + cubeY  ] + cubeZ+1];
    const int i101 = p[ p[ p[cubeX+1] + cubeY  ] + cubeZ+1];
    const int i011 = p[ p[ p[cubeX  ] + cubeY+1] + cubeZ+1];
    const int i111 = p[ p[ p[cubeX+1] + cubeY+1] + cubeZ+1];

    //
    // At each corner point of the cube, take the dot product
    // of the random gradient for that point and the uvw coordinate
    // within the cube.
    //
    const float g000 = weight(i000,   uvw.x,   uvw.y,   uvw.z);
    const float g100 = weight(i100, uvw.x-1,   uvw.y,   uvw.z);
    const float g010 = weight(i010,   uvw.x,   uvw.y-1, uvw.z);
    const float g110 = weight(i110, uvw.x-1,   uvw.y-1, uvw.z);
    const float g001 = weight(i001,   uvw.x,   uvw.y,   uvw.z-1);
    const float g101 = weight(i101, uvw.x-1,   uvw.y,   uvw.z-1);
    const float g011 = weight(i011,   uvw.x,   uvw.y-1, uvw.z-1);
    const float g111 = weight(i111, uvw.x-1,   uvw.y-1, uvw.z-1);

    //
    // Smooth the interpolant values.  This essentially smooths the linear
    // interpolation to a curve with ease-in / ease-out properties.
    // Without this, the result is discontinuities (i.e. hard 'edges') between
    // cells which adds a noticable tiling to the pattern.
    // 
    vector3 t;
    t.x = smoothInterpolant(uvw.x);
    t.y = smoothInterpolant(uvw.y);
    t.z = smoothInterpolant(uvw.z);

    //
    // Tri-linear interpolation
    //
    // Interpolate across all four edges of X.  Then interpolate the resulting
    // four points across the remaining two edges in Y.  Then interpolate the
    // remaining two points across the sole edge in Z.
    //
    float blend_x00 = interpolate(t.x, g000, g100);
    float blend_x10 = interpolate(t.x, g010, g110);
    float blend_x01 = interpolate(t.x, g001, g101);
    float blend_x11 = interpolate(t.x, g011, g111);

    float blend_y0  = interpolate(t.y, blend_x00, blend_x10);
    float blend_y1  = interpolate(t.y, blend_x01, blend_x11);

    float blend_z   = interpolate(t.z, blend_y0, blend_y1);

    return blend_z;
}
  

//===========================================================================//


float calc1(float s, float t)
{
    return (sinf(2 * t * 6.28318531f) + cosf(4 * s * 6.28318531f)) / 2.0f;
}

float calc2(float s, float t)
{
    return float(rand() % 256) / 255.0f;
}

float calc3(float s, float t)
{
    return perlin_noise(8 * s, 8 * t, .4f);
}

float calc4(float s, float t)
{
    return 0.25f * perlin_noise(32 * s, 32 * t, .4f) + 4 * perlin_noise(4 * s, 4 * t, .1f);
}


void 
generateHeightMap()
{
    gHMap.resize(256, 256);
    for (int y = 0; y < gHMap.sizeY(); y ++)
    {
        for (int x = 0; x < gHMap.sizeX(); x++)
        {
            float s = float(x) / gHMap.sizeX();
            float t = float(y) / gHMap.sizeY();
            gHMap(x, y) = 50 * calc4(s, t);
        }
    }
}

class Renderer
{
public:
    void initialize()
    {
        set(gCamera.mPosition, 500, 500, 250);
        set(gCamera.mTarget, 0, 0, 0);
        set(gCamera.mWorldUp, 0, 0, 1);
        gCamera.mFov = 60.0f;
        gCamera.mNear = 0.01f;  // 1 cm
        gCamera.mFar = 2000.0f; // 2 km

        rasterizer.initialize();

        // Create a vertex buffer to store the data for the vertex array
        generateHeightMap();
        std::vector<point3> positionData;
        positionData.reserve(gHMap.sizeX() * gHMap.sizeY() * 4);
        for (int y = 0; y < gHMap.sizeY(); ++y)
        {
            for (int x = 0; x < gHMap.sizeX() ; ++x)
            {
                float wy0 = -500.0f + 1000.0f * float(y + 0) / float(gHMap.sizeY());
                float wy1 = -500.0f + 1000.0f * float(y + 1) / float(gHMap.sizeY());
                float wx0 = -500.0f + 1000.0f * float(x + 0) / float(gHMap.sizeX());
                float wx1 = -500.0f + 1000.0f * float(x + 1) / float(gHMap.sizeX());
                float z = gHMap(x,y);

                float z1 = (y > 0) ? gHMap(x, y-1) : z;
                float z2 = z;
                float z3 = (x > 0) ? gHMap(x-1,y) : z;

                float z0;
                if (x > 0 && y > 0) 
                    z0 = gHMap(x-1, y-1);
                else if (x > 0)
                    z0 = z3;
                else if (y > 0)
                    z0 = z1;
                else
                    z0 = z;

                positionData.push_back( point3(wx0, wy0, z0) );
                positionData.push_back( point3(wx1, wy0, z1) );
                positionData.push_back( point3(wx1, wy1, z2) );
                positionData.push_back( point3(wx0, wy1, z3) );
            }
        }

        spCamera = rasterizer.createCamera(gCamera.mFov, gCamera.mNear, gCamera.mFar, view_matrix(gCamera));
        spLightSet = rasterizer.createLightSet();

        spItem.reset(new RasterizerGL::Item);
        spItem->spCamera   = spCamera;
        spItem->spLightSet = spLightSet;
        spItem->spMaterial = rasterizer.createMaterial();
        spItem->spTransform = rasterizer.createTransform(matrix4());
        spItem->spGeometry = rasterizer.createQuadList(positionData);
    }  

    void 
    resize (int width, int height)
    {

    }

    void 
    render (void)	
    {
        spCamera->viewMatrix = view_matrix(gCamera);

        rasterizer.beginScene();
        rasterizer.rasterize(spItem);
        rasterizer.endScene();
    }

protected:
    RasterizerGL::CameraPtr     spCamera;       // Camera shared by all items
    RasterizerGL::LightSetPtr   spLightSet;
    RasterizerGL::ItemPtr       spItem;
    RasterizerGL                rasterizer;
};

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    int exitCode = 0;

    lx_init();

    CanvasHost host;
    Renderer renderer;

    auto pWin = new CanvasGL("Terrain Sample (OpenGL 3.2)", 800, 400, false);
    host.create(pWin, "canvas", false);
    renderer.initialize();
    renderer.resize(800, 400);
    pWin->slotRedraw += [&]() { renderer.render(); };
    pWin->slotLMouseDrag += [&](const MouseState& ms, const ButtonState& bs, KeyModifiers km) {
        rotate_horizontal(gCamera, ms.deltaX() * -3.14f / 1000.0f );
        rotate_vertical(gCamera, ms.deltaY() * -3.14f / 1000.0f );
        pWin->invalidate(); 
    };
    pWin->show();

    bool bDone = false;
    do {
        bDone = host.processEvents();

        if (pWin->keyboard().bDown[KC_W])
            move_forward(gCamera, 10.0f);
        if (pWin->keyboard().bDown[KC_S])
            move_backward(gCamera, 10.0f);
        if (pWin->keyboard().bDown[KC_A])
            move_left(gCamera, 10.0f);
        if (pWin->keyboard().bDown[KC_D])
            move_right(gCamera, 10.0f);
        if (pWin->keyboard().bDown[KC_R])
            move_up(gCamera, 10.0f);
        if (pWin->keyboard().bDown[KC_F])
            move_down(gCamera, 10.0f);

        pWin->invalidate(); 

    } while (!bDone);

    return exitCode;
}
