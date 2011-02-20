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
#include <lx0/core/core.hpp>
#include <lx0/core/math/matrix4.hpp>
#include <lx0/core/util/util.hpp>
#include <lx0/canvas/canvas.hpp>
#include <lx0/prototype/prototype.hpp>
#include <lx0/view.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>

#include "rasterizergl.hpp"

using namespace lx0::core;
using namespace lx0::prototype;
using namespace lx0::canvas::platform;

Camera             gCamera;

//===========================================================================//

vector3 calcColor(float s, float t)
{
    vector3 c;
    c.x = 1.0f - noise3d_perlin(s / 2.0f, t / 2.0f, .212f);
    c.y = 0.0f;
    c.z = 0.0f;
    return c;
}

float calcHeight(float s, float t)
{
    float base = 120 * noise3d_perlin(s / 200.0f, t / 200.0f, .5f);
    float mid = 3 * noise3d_perlin(s / 40.0f, t / 30.0f, .1f)
              + 3 * noise3d_perlin(s / 45.0f, t / 60.0f, .6f);
    mid *= mid;
     
    return base + mid;
}

class Terrain
{
public:
    void generate(RasterizerGL& rasterizer,
                  Camera& cam1,
                  RasterizerGL::CameraPtr spCamera, 
                  RasterizerGL::LightSetPtr spLightSet, 
                  std::vector<RasterizerGL::ItemPtr>& list)
    {
        const int bx = int(cam1.mPosition.x / 100.0f);
        const int by = int(cam1.mPosition.y / 100.0f);

        const int dist = 10;
        for (int gy = -dist; gy <= dist; gy++)
        {
            for (int gx = -dist; gx <= dist; gx++)
            {
                std::pair<short, short> grid;
                grid.first = bx + gx;
                grid.second = by + gy;

                RasterizerGL::ItemPtr spTile;
                auto it = mMap.find(grid);
                if (it == mMap.end())
                {
                    if (abs(gx) < 5 && abs(gy) < 5) 
                    {
                        spTile = _buildTile(rasterizer, spCamera, spLightSet, grid.first, grid.second);
                        mMap.insert(std::make_pair(grid, spTile));
                    }
                }
                else
                    spTile = it->second;

                if (spTile.get() != nullptr)
                    list.push_back(spTile);
            }
        }
    }

protected:
    template <typename T>
    struct BorderArray2d
    {
        BorderArray2d (int w, int h, int b)
        {
            mOffset = b;
            mRowSpan = w + 2 * b;
            mData.reset(new T[mRowSpan * (h + 2 * b)]);
        }

        T&  operator() (int x, int y)
        {
            return mData[ (y + mOffset) * mRowSpan + (x + mOffset) ];
        }

        int            mOffset;
        int            mRowSpan;
        std::unique_ptr<T[]> mData;
    };

    RasterizerGL::GeometryPtr 
    _buildTileGeom2 (RasterizerGL& rasterizer,  int regionX, int regionY)
    {
        const float kRegionSize = 100.0f;

        float tx = regionX * kRegionSize;
        float ty = regionY * kRegionSize;

        //
        // Compute all the heights first.   All interior heights are used
        // 4 times, therefore it's quicker to compute them once and store
        // the value than recompute each time.
        //
        BorderArray2d<float> heights (100, 100, 2);
        for (int y = -1; y <= 101; ++y)
        {
            for (int x = -1; x <= 101; ++x)
            {
                heights(x, y) = calcHeight(tx + x, ty + y);
            }
        }

        std::vector<point3> positions (101 * 101);
        for (int y = 0; y <= 100; ++y)
        {
            for (int x = 0; x <= 100; ++x)
            {
                point3 p;
                p.x = float(x);
                p.y = float(y);
                p.z = heights(x, y);
                positions[y * 101 + x] = p;
            }
        }

        std::vector<vector3> normals (101 * 101);
        for (int y = 0; y <= 100; ++y)
        {
            for (int x = 0; x <= 100; ++x)
            {
                vector3 dx;
                dx.x = 2;
                dx.y = 0;
                dx.z = heights(x - 1, y) - heights(x + 1, y);
                
                vector3 dy;
                dy.x = 0;
                dy.y = 2;
                dy.z = heights(x, y - 1) - heights(x, y + 1);
 
                normals[y * 101 + x] = normalize( cross( normalize(dx), normalize(dy) ) );
            }
        }

        std::vector<vector3> colors (101 * 101);
        for (int y = 0; y <= 100; ++y)
        {
            for (int x = 0; x <= 100; ++x)
            {
                colors[y * 101 + x] = calcColor(tx + x, ty + y);
            }
        }

        // Create a vertex buffer to store the data for the vertex array
        //
        //@todo Switch to index buffer
        //@todo Add normals
        std::vector<unsigned short> indices;
        indices.reserve(100 * 100 * 4);
        for (int y = 0; y < 100; ++y)
        {
            for (int x = 0; x < 100; ++x)
            {
                indices.push_back( (y + 0) * 101 + (x + 0) );
                indices.push_back( (y + 0) * 101 + (x + 1) );
                indices.push_back( (y + 1) * 101 + (x + 1) );
                indices.push_back( (y + 1) * 101 + (x + 0) );
            }
        }

        return rasterizer.createQuadList(indices, positions, normals, colors);
    }

    RasterizerGL::MaterialPtr 
    _acquireMaterial (RasterizerGL& rasterizer)
    {
        if (mwpMaterial.expired())
        {
            auto spTextureGrass = rasterizer.createTexture("media2/textures/seamless/grass/grass_yofrankie01/grass_0.png");
            auto spTextureDirt = rasterizer.createTexture("media2/textures/seamless/dirt/dirt000/dirt000.png");

            auto spMat = rasterizer.createMaterial();
            spMat->mTextures[0] = spTextureGrass;
            spMat->mTextures[1] = spTextureDirt;

            mwpMaterial = spMat;
            return spMat;           // Be sure to return *before* spMat goes out of scope
        }
        else
            return mwpMaterial.lock();
    }

    RasterizerGL::ItemPtr _buildTile (RasterizerGL& rasterizer, 
                                     RasterizerGL::CameraPtr spCamera, 
                                     RasterizerGL::LightSetPtr spLightSet, 
                                     int regionX, int regionY)
    {
        auto pItem = new RasterizerGL::Item;
        pItem->spCamera   = spCamera;
        pItem->spLightSet = spLightSet;
        pItem->spMaterial = _acquireMaterial(rasterizer);
        pItem->spTransform = rasterizer.createTransform(regionX * 100.0f, regionY * 100.0f, 0.0f);
        pItem->spGeometry = _buildTileGeom2(rasterizer, regionX, regionY);
        return RasterizerGL::ItemPtr(pItem);
    }

    RasterizerGL::MaterialWPtr                               mwpMaterial;
    std::map<std::pair<short, short>, RasterizerGL::ItemPtr> mMap;
};

Terrain gTerrain;

class Renderer
{
public:
    void initialize()
    {
        set(gCamera.mPosition, 10, 10, 15);
        set(gCamera.mTarget, 0, 0, 0);
        set(gCamera.mWorldUp, 0, 0, 1);
        gCamera.mFov = 60.0f;
        gCamera.mNear = 0.01f;  // 1 cm
        gCamera.mFar = 2000.0f; // 2 km

        gCamera.mPosition.z = calcHeight(gCamera.mPosition.x, gCamera.mPosition.y) + 2.0f;
        gCamera.mTarget.z = gCamera.mPosition.z;

        rasterizer.initialize();

        spCamera = rasterizer.createCamera(gCamera.mFov, gCamera.mNear, gCamera.mFar, view_matrix(gCamera));
        spLightSet = rasterizer.createLightSet();
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
        std::vector<RasterizerGL::ItemPtr> items;
        gTerrain.generate(rasterizer, gCamera, spCamera, spLightSet, items);
        rasterizer.rasterizeList(items);
        rasterizer.endScene();
    }

protected:
    RasterizerGL::CameraPtr     spCamera;       // Camera shared by all items
    RasterizerGL::LightSetPtr   spLightSet;
    std::vector<RasterizerGL::ItemPtr> itemList;
    RasterizerGL                rasterizer;
};

//===========================================================================//

class LxCanvasImp : public ViewImp
{
public:
    virtual void        createWindow    (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height);
    virtual void        destroyWindow   (void);
    virtual void        show            (View* pHostView, Document* pDocument);

    virtual     void        _onElementAdded             (ElementPtr spElem) {}
    virtual     void        _onElementRemoved           (ElementPtr spElem) {}

    virtual     void        updateBegin     (void) {}
    virtual     void        updateFrame     (void);
    virtual     void        updateEnd       (void) {}

protected:
    std::auto_ptr<CanvasGL> mspWin;
    CanvasHost              mHost;
    Renderer                mRenderer;
};

void 
LxCanvasImp::createWindow (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height)
{
    width = 800;
    height = 400;

    mspWin.reset( new CanvasGL("Terrain Sample (OpenGL 3.2)", width, height, false) );
    handle = mspWin->handle();

    mRenderer.initialize();
    mRenderer.resize(width, height);

    mspWin->slotRedraw += [&]() { mRenderer.render(); };
    mspWin->slotLMouseDrag += [&](const MouseState& ms, const ButtonState& bs, KeyModifiers km) {
        rotate_horizontal(gCamera, ms.deltaX() * -3.14f / 1000.0f );
        rotate_vertical(gCamera, ms.deltaY() * -3.14f / 1000.0f );
        mspWin->invalidate(); 
    };
}

void
LxCanvasImp::destroyWindow (void)
{
    mspWin->destroy();
}

void 
LxCanvasImp::show (View* pHostView, Document* pDocument)
{
    mspWin->show();
}

void 
LxCanvasImp::updateFrame (void) 
{
    const float kStep = 2.0f;

    if (mspWin->keyboard().bDown[KC_ESCAPE])
        Engine::acquire()->sendMessage("quit");

    if (mspWin->keyboard().bDown[KC_W])
        move_forward(gCamera, kStep);
    if (mspWin->keyboard().bDown[KC_S])
        move_backward(gCamera, kStep);
    if (mspWin->keyboard().bDown[KC_A])
        move_left(gCamera, kStep);
    if (mspWin->keyboard().bDown[KC_D])
        move_right(gCamera, kStep);
    if (mspWin->keyboard().bDown[KC_R])
        move_up(gCamera, kStep);
    if (mspWin->keyboard().bDown[KC_F])
        move_down(gCamera, kStep);

    float deltaZ = (calcHeight(gCamera.mPosition.x, gCamera.mPosition.y) + 2.0f) - gCamera.mPosition.z;
    gCamera.mPosition.z += deltaZ;
    gCamera.mTarget.z += deltaZ;

    mspWin->invalidate(); 
}

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        EnginePtr   spEngine   = Engine::acquire();
        spEngine->addViewPlugin("LxCanvas", [] (View* pView) { return new LxCanvasImp; });
        
        DocumentPtr spDocument = spEngine->loadDocument("media2/appdata/sm_terrain/scene.xml");
        ViewPtr     spView     = spDocument->createView("LxCanvas", "view");
        spView->show();

        exitCode = spEngine->run();
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
