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
#include <lx0/element.hpp>

#include "rasterizergl.hpp"

using namespace lx0::core;
using namespace lx0::prototype;
using namespace lx0::canvas::platform;

Camera             gCamera;

//===========================================================================//

class Renderable : public Element::Component
{
public:
    virtual void generate(ElementPtr spElement,
                  RasterizerGL& rasterizer,
                  Camera& cam1,
                  RasterizerGL::CameraPtr spCamera, 
                  RasterizerGL::LightSetPtr spLightSet, 
                  std::vector<RasterizerGL::ItemPtr>& list) = 0;
};

namespace Terrain
{
    class Runtime : public Element::Component
    {
    public:
        vector3 
        calcColor(float s, float t)
        {
            vector3 c;
            c.x = 1.0f - noise3d_perlin(s / 2.0f, t / 2.0f, .212f);
            c.y = 0.0f;
            c.z = 0.0f;
            return c;
        }

        float 
        calcHeight(float s, float t)
        {
            float base = 120 * noise3d_perlin(s / 200.0f, t / 200.0f, .5f);
            float mid = 3 * noise3d_perlin(s / 40.0f, t / 30.0f, .1f)
                      + 3 * noise3d_perlin(s / 45.0f, t / 60.0f, .6f);
            mid *= mid;
     
            return base + mid;
        }
    };

    class Render : public Renderable
    {
    public:
        Render()
        {
            lx_debug("Terrain::Render ctor");
        }

        void generate(ElementPtr spElement,
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
        _buildTileGeom2 (ElementPtr spElement, RasterizerGL& rasterizer,  int regionX, int regionY)
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

            std::vector<vector3> colors (101 * 101);
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

        RasterizerGL::ItemPtr _buildTile (ElementPtr spElement, 
                                         RasterizerGL& rasterizer, 
                                         RasterizerGL::CameraPtr spCamera, 
                                         RasterizerGL::LightSetPtr spLightSet, 
                                         int regionX, int regionY)
        {
            auto pItem = new RasterizerGL::Item;
            pItem->spCamera   = spCamera;
            pItem->spLightSet = spLightSet;
            pItem->spMaterial = _acquireMaterial(rasterizer);
            pItem->spTransform = rasterizer.createTransform(regionX * 100.0f, regionY * 100.0f, 0.0f);
            pItem->spGeometry = _buildTileGeom2(spElement, rasterizer, regionX, regionY);
            return RasterizerGL::ItemPtr(pItem);
        }

        std::shared_ptr<Terrain::Runtime>                        mspTerrain;
        RasterizerGL::MaterialWPtr                               mwpMaterial;
        std::map<std::pair<short, short>, RasterizerGL::ItemPtr> mMap;
    };
}





class PhysicsSubsystem : public DocumentComponent
{
public: 
    virtual void    onElementAdded      (DocumentPtr spDocument, ElementPtr spElem) 
    {
        if (spElem->tagName() == "Terrain")
        {
            mElems.insert(std::make_pair(spElem.get(), spElem));
        }
    }
    virtual void    onElementRemoved    (Document*   pDocument, ElementPtr spElem) 
    {
        auto it = mElems.find(spElem.get());
        if (it != mElems.end())
            mElems.erase(it);
    }
    
    float drop (float x, float y)
    {
        float maxZ = std::numeric_limits<float>::min();
        for (auto it = mElems.begin(); it != mElems.end(); ++it)
        {
            auto spTerrain = it->second->getComponent<Terrain::Runtime>("runtime");
            maxZ = std::max(maxZ, spTerrain->calcHeight(x, y));
        }
        return maxZ; 
    }

    std::map<Element*, ElementPtr> mElems;
};


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

        gCamera.mPosition.z = 0.0f;
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

        std::vector<ElementPtr> mElems = mspDocument->getElementsByTagName("Terrain");
        for (auto it = mElems.begin(); it != mElems.end(); ++it)
        {
            auto spRenderable = (*it)->getComponent<Renderable>("renderable");
            if (spRenderable)
            {
                spRenderable->generate(*it, rasterizer, gCamera, spCamera, spLightSet, items);
            }
        }

        rasterizer.rasterizeList(items);

        rasterizer.endScene();

        rasterizer.refreshTextures();
    }

    DocumentPtr                 mspDocument;

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
                        LxCanvasImp();
                        ~LxCanvasImp();

    virtual void        createWindow    (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height);
    virtual void        destroyWindow   (void);
    virtual void        show            (View* pHostView, Document* pDocument);

    virtual     void        _onElementAdded             (ElementPtr spElem) {}
    virtual     void        _onElementRemoved           (ElementPtr spElem) {}

    virtual     void        updateBegin     (void) {}
    virtual     void        updateFrame     (DocumentPtr spDocument);
    virtual     void        updateEnd       (void) {}

protected:
    DocumentPtr             mspDocument;
    CanvasHost              mHost;
    std::auto_ptr<CanvasGL> mspWin;
    Renderer                mRenderer;
};

LxCanvasImp::LxCanvasImp()
{
    lx_debug("LxCanvasImp ctor");
}

LxCanvasImp::~LxCanvasImp()
{
    lx_debug("LxCanvasImp dtor");
}

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
    mspDocument = pDocument->shared_from_this();
    mRenderer.mspDocument = mspDocument;
    mspWin->show();
}

void 
LxCanvasImp::updateFrame (DocumentPtr spDocument) 
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

    const float terrainHeight = mspDocument->getComponent<PhysicsSubsystem>("physics2")->drop(gCamera.mPosition.x, gCamera.mPosition.y);
    const float deltaZ = (terrainHeight + 2.0f) - gCamera.mPosition.z;
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
        spEngine->addDocumentComponent("physics2", [] () { return new PhysicsSubsystem; } );
        spEngine->addViewPlugin("LxCanvas", [] (View* pView) { return new LxCanvasImp; });
        spEngine->addElementComponent("Terrain", "runtime", []() { return new Terrain::Runtime; }); 
        spEngine->addElementComponent("Terrain", "renderable", []() { return new Terrain::Render; });
        
        DocumentPtr spDocument = spEngine->loadDocument("media2/appdata/sm_terrain/scene.xml");
        ViewPtr     spView     = spDocument->createView("LxCanvas", "view");
        spView->show();

        exitCode = spEngine->run();
        spDocument->destroyView("view");
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
