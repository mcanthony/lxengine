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

    float& operator() (int x, int y) { return mHeight[y * mSizeX + x]; }

    int                mSizeX;
    int                mSizeY;
    std::vector<float> mHeight;
};

Camera             gCamera;
HeightMap          gHMap;

void 
generateHeightMap()
{
    gHMap.resize(64, 64);
    for (int y = 0; y < gHMap.sizeY(); y ++)
    {
        for (int x = 0; x < gHMap.sizeX(); x++)
        {
            float h = float( 25 * (sin(x * 6.28318531 / 32) + cos(y * 6.28318531 / 16)) );
            gHMap(x, y) = h;
        }
    }
}

//===========================================================================//

class RasterizerGL
{
public:
    struct Camera
    {
        virtual void activate();
        matrix4 projMatrix;
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

    CameraPtr       createCamera    (matrix4& viewMatrix);
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
}

void RasterizerGL::shutdown()
{
}


RasterizerGL::CameraPtr       
RasterizerGL::createCamera (matrix4& viewMatrix)
{
    CameraPtr spCamera (new Camera);
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
    gluPerspective(60.0f, (GLfloat)vp[2]/(GLfloat)vp[3], 0.01f, 2000.0f);


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

class Renderer
{
public:
    void initialize()
    {
        set(gCamera.mPosition, 750, 750, 900);
        set(gCamera.mTarget, 0, 0, 0);
        set(gCamera.mWorldUp, 0, 0, 1);

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

                float z0 = (x > 0 && y > 0) ? gHMap(x-1, y-1) : z;
                float z1 = (y > 0) ? gHMap(x, y-1) : z;
                float z2 = z;
                float z3 = (x > 0) ? gHMap(x-1,y) : z;

                positionData.push_back( point3(wx0, wy0, z0) );
                positionData.push_back( point3(wx1, wy0, z1) );
                positionData.push_back( point3(wx1, wy1, z2) );
                positionData.push_back( point3(wx0, wy1, z3) );
            }
        }

        spCamera = rasterizer.createCamera(view_matrix(gCamera));
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
        rotate_horizontal(gCamera, ms.deltaX() * 3.14f / 1000.0f );
        rotate_vertical(gCamera, ms.deltaY() * 3.14f / 1000.0f );
        pWin->invalidate(); 
    };
    pWin->show();

    bool bDone = false;
    do {
        bDone = host.processEvents();
    } while (!bDone);

    return exitCode;
}
