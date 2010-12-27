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
#include <lx0/core/core.hpp>
#include <lx0/core/math/matrix4.hpp>
#include <lx0/core/util/util.hpp>
#include <lx0/canvas/canvas.hpp>
#include <lx0/prototype/prototype.hpp>

#include "rasterizergl.hpp"

using namespace lx0::core;
using namespace lx0::prototype;
using namespace lx0::canvas::platform;

Camera             gCamera;

//===========================================================================//

float calcHeight(float s, float t)
{
    float base = 90 * noise3d_perlin(s / 200.0f, t / 200.0f, .5f);
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
    RasterizerGL::ItemPtr _buildTile (RasterizerGL& rasterizer, 
                                     RasterizerGL::CameraPtr spCamera, 
                                     RasterizerGL::LightSetPtr spLightSet, 
                                     int regionX, int regionY)
    {
        const float kRegionSize = 100.0f;

        float tx = regionX * kRegionSize;
        float ty = regionY * kRegionSize;

        //
        // Compute all the heights first.   All interior heights are used
        // 4 times, therefore it's quicker to compute them once and store
        // the value than recompute each time.
        //
        std::unique_ptr<float[]> heights(new float[102 * 102]);
        for (int y = 0; y < 102; ++y)
        {
            for (int x = 0; x < 102; ++x)
            {
                heights[y * 102 + x] = calcHeight(tx + x, ty + y);
            }
        }

        std::unique_ptr<vector3[]> normals(new vector3[101 * 101]);
        for (int y = 1; y < 101; ++y)
        {
            for (int x = 1; x < 101; ++x)
            {
                vector3 dx;
                dx.x = 2;
                dx.y = 0;
                dx.z = heights[y * 102 + x - 1] - heights[y * 102 + x + 1];
                
                vector3 dy;
                dy.x = 0;
                dy.y = 2;
                dy.z = heights[(y - 1) * 102 + x] - heights[(y + 1) * 102 + x];
                
                
                vector3 normal = normalize( cross( normalize(dx), normalize(dy) ) );
                normals[(y - 1) * 101 + (x - 1)] = normal;
            }
        }

        // Create a vertex buffer to store the data for the vertex array
        //
        //@todo Switch to index buffer
        //@todo Add normals
        //@todo Add offset-able 2d array for above calculations
        std::vector<point3> positionData;
        positionData.reserve(100 * 100 * 4);
        for (int y = 0; y < 100; ++y)
        {
            for (int x = 0; x < 100; ++x)
            {
                point3 w00;
                w00.x = x + 0;
                w00.y = y + 0;
                w00.z = heights[(y + 1) * 102 + (x + 1)];
                        
                point3 w11;
                w11.x = x + 1;
                w11.y = y + 1;
                w11.z = heights[(y + 2) * 102 + (x + 2)];

                point3 w10;
                w10.x = w11.x;
                w10.y = w00.y;
                w10.z = heights[(y + 1) * 102 + (x + 2)];

                point3 w01;
                w01.x = w00.x;
                w01.y = w11.y;
                w01.z = heights[(y + 2) * 102 + (x + 1)];

                positionData.push_back( w00 );
                positionData.push_back( w10 );
                positionData.push_back( w11 );
                positionData.push_back( w01 );
            }
        }

        auto pItem = new RasterizerGL::Item;
        pItem->spCamera   = spCamera;
        pItem->spLightSet = spLightSet;
        pItem->spMaterial = rasterizer.createMaterial();
        pItem->spTransform = rasterizer.createTransform(tx, ty, 0.0f);
        pItem->spGeometry = rasterizer.createQuadList(positionData);

        return RasterizerGL::ItemPtr(pItem);
    }

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
        const float kStep = 2.0f;
        bDone = host.processEvents();

        if (pWin->keyboard().bDown[KC_W])
            move_forward(gCamera, kStep);
        if (pWin->keyboard().bDown[KC_S])
            move_backward(gCamera, kStep);
        if (pWin->keyboard().bDown[KC_A])
            move_left(gCamera, kStep);
        if (pWin->keyboard().bDown[KC_D])
            move_right(gCamera, kStep);
        if (pWin->keyboard().bDown[KC_R])
            move_up(gCamera, kStep);
        if (pWin->keyboard().bDown[KC_F])
            move_down(gCamera, kStep);

        float deltaZ = (calcHeight(gCamera.mPosition.x, gCamera.mPosition.y) + 2.0f) - gCamera.mPosition.z;
        gCamera.mPosition.z += deltaZ;
        gCamera.mTarget.z += deltaZ;

        pWin->invalidate(); 

    } while (!bDone);

    return exitCode;
}
