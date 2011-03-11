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
#include <limits>

// Lx0 headers
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/core/core.hpp>
#include <lx0/core/math/matrix4.hpp>
#include <lx0/core/util/util.hpp>
#include <lx0/prototype/prototype.hpp>

#include "terrain.hpp"
#include "rasterizergl.hpp"

//===========================================================================//
//   N A M E S P A C E S
//===========================================================================//

using namespace lx0::core;
using namespace lx0::prototype;

extern Camera gCamera;

//===========================================================================//
//   F I L E   I M P L E M E N T A T I O N 
//===========================================================================//

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

namespace Terrain
{

    Runtime::Runtime (ElementPtr spElem)
    {
    }


    tuple3 
    Runtime::calcColor(float s, float t)
    {
        tuple3 c;
        c.x = 1.0f - noise3d_perlin(s / 2.0f, t / 2.0f, .212f);
        c.y = 0.0f;
        c.z = 0.0f;
        return c;
    }

    float 
    Runtime::calcHeight(float s, float t)
    {
        float base = 120 * noise3d_perlin(s / 200.0f, t / 200.0f, .5f);
        float mid = 3 * noise3d_perlin(s / 40.0f, t / 30.0f, .1f)
                    + 3 * noise3d_perlin(s / 45.0f, t / 60.0f, .6f);
        mid *= mid;
     
        return base + mid;
    }
    
    Render::Render()
    {
        lx_debug("Terrain::Render ctor");
    }

    void Render::generate(ElementPtr spElement,
                    RasterizerGL& rasterizer,
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
                        spTile = _buildTile(spElement, rasterizer, spCamera, spLightSet, grid.first, grid.second);
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


    RasterizerGL::GeometryPtr 
    Render::_buildTileGeom2 (ElementPtr spElement, RasterizerGL& rasterizer,  int regionX, int regionY)
    {
        // Eventually the renderable should be a component of the Element.
        // The component has direct access to the Element, removing the need for any
        // getElementsByTagName() call.
        //
        if (!mspTerrain)
            mspTerrain = spElement->getComponent<Terrain::Runtime>("runtime");

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
                heights(x, y) = mspTerrain->calcHeight(tx + x, ty + y);
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

        std::vector<tuple3> colors (101 * 101);
        for (int y = 0; y <= 100; ++y)
        {
            for (int x = 0; x <= 100; ++x)
            {
                colors[y * 101 + x] = mspTerrain->calcColor(tx + x, ty + y);
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
    Render::_ensureMaterial (RasterizerGL& rasterizer)
    {
        if (mwpMaterial.expired())
        {
            auto spTextureGrass = rasterizer.createTexture("media2/textures/seamless/grass/grass_yofrankie01/grass_0.png");
            auto spTextureDirt = rasterizer.createTexture("media2/textures/seamless/dirt/dirt000/dirt000.png");

            auto spMat = rasterizer.createMaterial("media2/shaders/glsl/fragment/checker_world_xy10.frag");
            spMat->mTextures[0] = spTextureGrass;
            spMat->mTextures[1] = spTextureDirt;

            mwpMaterial = spMat;
            return spMat;           // Be sure to return *before* spMat goes out of scope
        }
        else
            return mwpMaterial.lock();
    }

    RasterizerGL::ItemPtr Render::_buildTile (ElementPtr spElement, 
                                        RasterizerGL& rasterizer, 
                                        RasterizerGL::CameraPtr spCamera, 
                                        RasterizerGL::LightSetPtr spLightSet, 
                                        int regionX, int regionY)
    {
        auto pItem = new RasterizerGL::Item;
        pItem->spCamera   = spCamera;
        pItem->spLightSet = spLightSet;
        pItem->spMaterial = _ensureMaterial(rasterizer);
        pItem->spTransform = rasterizer.createTransform(regionX * 100.0f, regionY * 100.0f, 0.0f);
        pItem->spGeometry = _buildTileGeom2(spElement, rasterizer, regionX, regionY);
        return RasterizerGL::ItemPtr(pItem);
    }
}

