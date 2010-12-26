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




        
//===========================================================================//



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
    return noise3d_perlin(8 * s, 8 * t, .4f);
}

float calc4(float s, float t)
{
    return 8 * noise3d_perlin(s, t, .1f) 
        + (3.2 * noise3d_perlin(8 * s, 8 * t, .4f) * noise3d_perlin(4 * s, 4 * t, .38f));
}


void 
generateHeightMap(int regionX, int regionY)
{
    gHMap.resize(64, 64);
    for (int y = 0; y < gHMap.sizeY(); y ++)
    {
        for (int x = 0; x < gHMap.sizeX(); x++)
        {
            float s = float(x) / gHMap.sizeX() + regionX;
            float t = float(y) / gHMap.sizeY() + regionY;
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
        gCamera.mFar = 4000.0f; // 4 km

        rasterizer.initialize();

        spCamera = rasterizer.createCamera(gCamera.mFov, gCamera.mNear, gCamera.mFar, view_matrix(gCamera));
        spLightSet = rasterizer.createLightSet();

        for (int regionY = -2; regionY <= 2; regionY++)
        {
            for (int regionX = -2; regionX <= 2; regionX++)
            {
                // Create a vertex buffer to store the data for the vertex array
                generateHeightMap(-regionX, -regionY);
                std::vector<point3> positionData;
                positionData.reserve(gHMap.sizeX() * gHMap.sizeY() * 4);
                for (int y = 0; y < gHMap.sizeY(); ++y)
                {
                    for (int x = 0; x < gHMap.sizeX() ; ++x)
                    {
                        float wy0 = 2 * regionY * -500.0f + 1000.0f * float(y + 0) / float(gHMap.sizeY());
                        float wy1 = 2 * regionY * -500.0f + 1000.0f * float(y + 1) / float(gHMap.sizeY());
                        float wx0 = 2 * regionX * -500.0f + 1000.0f * float(x + 0) / float(gHMap.sizeX());
                        float wx1 = 2 * regionX * -500.0f + 1000.0f * float(x + 1) / float(gHMap.sizeX());
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

                auto pItem = new RasterizerGL::Item;
                pItem->spCamera   = spCamera;
                pItem->spLightSet = spLightSet;
                pItem->spMaterial = rasterizer.createMaterial();
                pItem->spTransform = rasterizer.createTransform(matrix4());
                pItem->spGeometry = rasterizer.createQuadList(positionData);

                itemList.push_back(RasterizerGL::ItemPtr(pItem));
            }
        }
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
        rasterizer.rasterizeList(itemList);
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
