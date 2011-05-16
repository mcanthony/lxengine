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
//  T O D O   L I S T
//===========================================================================//
/*!
    - Clean-up code
    - Reduce dependencies
    - Add Controller object that maps UI to Engine events
 */

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

// Standard headers
#define NOMINMAX 
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
#include <glgeom/glgeom.hpp>

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/rasterizer.hpp>
#include <lx0/subsystem/canvas.hpp>
#include <lx0/prototype/misc.hpp>

#include <lx0/blendreader/blendreader.hpp>

#include "main.hpp"
#include "terrain.hpp"

using namespace lx0;
using namespace lx0::core;
using namespace lx0::prototype;

lx0::prototype::Camera             gCamera;

//===========================================================================//

void
RenderList::push_back (int layer, ItemPtr spItem)
{
    mLayers[layer].list.push_back(spItem);
}

//===========================================================================//

class PhysicsSubsystem : public DocumentComponent
{
public: 
    virtual void onElementAdded (DocumentPtr spDocument, ElementPtr spElem) 
    {
        if (spElem->tagName() == "Terrain")
        {
            mElems.insert(std::make_pair(spElem.get(), spElem));
        }
    }
    virtual void onElementRemoved (Document*   pDocument, ElementPtr spElem) 
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

    virtual void onUpdate (DocumentPtr spDocument)
    {
        const float terrainHeight = drop(gCamera.mPosition.x, gCamera.mPosition.y);
        const float deltaZ = (terrainHeight + 32.0f) - gCamera.mPosition.z;
        gCamera.mPosition.z += deltaZ;
        gCamera.mTarget.z += deltaZ;

        if (deltaZ > 0.001)
            spDocument->view(0)->sendEvent("redraw", lxvar::undefined());
    }

    std::map<Element*, ElementPtr> mElems;
};


class Renderer
{
public:
    Renderer()
        : mViewMode (0)
    {
    }

    void initialize()
    {
        gCamera.mPosition = glgeom::point3f(20, 20, 32);
        gCamera.mTarget = glgeom::point3f(0, 0, 0);
        gCamera.mWorldUp = glgeom::vector3f(0, 0, 1);
        gCamera.mFov = 60.0f;
        gCamera.mNear = 0.01f;  // 1 cm
        gCamera.mFar = 2000.0f; // 2 km

        gCamera.mPosition.z = 0.0f;
        //gCamera.mTarget.z = gCamera.mPosition.z;

        mRasterizer.initialize();

        spCamera = mRasterizer.createCamera(gCamera.mFov, gCamera.mNear, gCamera.mFar, view_matrix(gCamera));
        spLightSet = mRasterizer.createLightSet();
    }  

    void 
    resize (int width, int height)
    {

    }

    void
    update (void)
    {
        std::vector<ElementPtr> elems;
        elems.swap( mspDocument->getElements() );
        for (auto it = elems.begin(); it != elems.end(); ++it)
        {
            auto spRenderable = (*it)->getComponent<Renderable>("renderable");
            if (spRenderable)
            {
                spRenderable->update(*it);
            }
        }
    }

    void 
    _generateItems (RenderList& items)
    {
        std::vector<ElementPtr> elems;
        elems.swap( mspDocument->getElements() );
        for (auto it = elems.begin(); it != elems.end(); ++it)
        {
            auto spRenderable = (*it)->getComponent<Renderable>("renderable");
            if (spRenderable)
            {
                spRenderable->generate(*it, mRasterizer, gCamera, spCamera, spLightSet, items);
            }
        }
    }

    void
    cycleViewMode (void)
    {
        mViewMode = (mViewMode + 1) % 3;
    }

    void 
    _generateRenderAlgorithm (RenderAlgorithm& algorithm)
    {
        algorithm.mClearColor = glgeom::color4f(0.09f, 0.09f, 0.11f, 1.0f);

        GlobalPass pass[4];
        switch (mViewMode)
        {
        default:
            lx_error("Invalid view mode %d", mViewMode);
        case 0:
            // Use a single pass with all the default settings
            algorithm.mPasses.push_back(pass[0]);
            break;
        case 1:
            pass[0].bOverrideWireframe = true;
            pass[0].bWireframe = true;
            algorithm.mPasses.push_back(pass[0]);
            break;
        case 2:
            pass[0].bOverrideMaterial = true;
            pass[0].spMaterial = mRasterizer.createMaterial("media2/shaders/glsl/fragment/solid.frag");
            algorithm.mPasses.push_back(pass[0]);
            break;
        }  
    }

    void 
    _generateSelectAlgorithm (RenderAlgorithm& algorithm)
    {
        algorithm.mClearColor = glgeom::color4f(0.0f, 0.0f, 0.0f, 0.0f);

        // The solid.frag is not sufficient since the alpha mask needs to be read from RGBA textures
        // to ensure only the right pixels are actually written to the pixel buffer.
        //
        GlobalPass pass[4];
        pass[0].bOverrideMaterial = true;
        pass[0].spMaterial = mRasterizer.createMaterial("media2/shaders/glsl/fragment/solid.frag");
        algorithm.mPasses.push_back(pass[0]); 
    }

    void
    _renderImp (RenderAlgorithm& algorithm, RenderList& items)
    {
        spCamera->viewMatrix = view_matrix(gCamera);

        mRasterizer.beginScene(algorithm);

        _generateItems(items);
        for (auto it = items.begin(); it != items.end(); ++it)
        {
            mRasterizer.rasterizeList(algorithm, it->second.list);
        }

        mRasterizer.endScene();
    }

    void 
    render (void)	
    {
        RenderAlgorithm algorithm;
        RenderList items;

        _generateRenderAlgorithm(algorithm);
        _renderImp(algorithm, items);

        mRasterizer.refreshTextures();
    }

    ItemPtr
    select (int x, int y)
    {
        RenderAlgorithm algorithm;
        _generateSelectAlgorithm(algorithm);

        // Draw to the backbuffer
        RenderList items;
        _renderImp(algorithm, items);
        unsigned int id = mRasterizer.readPixel(x, y);
        return items.getItem(id);
    }

    DocumentPtr                 mspDocument;

protected:
    CameraPtr     spCamera;       // Camera shared by all items
    LightSetPtr   spLightSet;
    RasterizerGL                mRasterizer;
    int                         mViewMode;
};

//===========================================================================//

class Controller2
{
public:
    virtual                 ~Controller2() {}

    virtual     void        onLClick        (ViewPtr spView, const MouseState&, const ButtonState&, KeyModifiers) {}
    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard) = 0;
};

class CameraController : public Controller2
{
public:
    virtual     void        onLClick        (ViewPtr spView, const MouseState&, const ButtonState&, KeyModifiers);
    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard);
};

void
CameraController::onLClick (ViewPtr spView, const MouseState& mouse, const ButtonState&, KeyModifiers)
{
    spView->sendEvent("select_object", lxvar(mouse.x, mouse.y));
}

void
CameraController::updateFrame (ViewPtr spView, const KeyboardState& keyboard)
{
    const float kStep = 2.0f;

    if (keyboard.bDown[KC_ESCAPE])
        Engine::acquire()->sendMessage("quit");

    if (keyboard.bDown[KC_W])
        spView->sendEvent("move_forward", kStep);
    if (keyboard.bDown[KC_S])
        spView->sendEvent("move_backward", kStep);
    if (keyboard.bDown[KC_A])
        spView->sendEvent("move_left", kStep);
    if (keyboard.bDown[KC_D])
        spView->sendEvent("move_right", kStep);
    if (keyboard.bDown[KC_R])
        spView->sendEvent("move_up", kStep);
    if (keyboard.bDown[KC_F])
        spView->sendEvent("move_down", kStep);

    if (keyboard.bDown[KC_M])
        spView->sendEvent("cycle_viewmode", lxvar());
}

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

    virtual     void        handleEvent     (std::string evt, lx0::core::lxvar params);

protected:
    View*                   mpHostView;
    DocumentPtr             mspDocument;
    CanvasHost              mHost;
    std::auto_ptr<CanvasGL> mspWin;
    Renderer                mRenderer;
    CameraController        mController;
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

    mspWin.reset( new CanvasGL("Terrain Sample (OpenGL 3.2)", 16, 16, width, height, false) );
    handle = mspWin->handle();

    mRenderer.initialize();
    mRenderer.resize(width, height);

    mspWin->slotRedraw += [&]() { mRenderer.render(); };
    mspWin->slotLMouseClick += [&](const MouseState& ms, const ButtonState& bs, KeyModifiers km) { 
        mController.onLClick(mpHostView->shared_from_this(), ms, bs, km);
    };
    mspWin->slotLMouseDrag += [&](const MouseState& ms, const ButtonState& bs, KeyModifiers km) {
        
        // Rotate horizontal
        rotate_horizontal(gCamera, ms.deltaX() * -3.14f / 1000.0f );

        // Rotate vertical..
        //@todo:  only if not going to cause the camera to be staring straight up or straight down
        float vertAngle = ms.deltaY() * -3.1415f / 1000.0f;
        rotate_vertical(gCamera, vertAngle);
        
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
    mpHostView = pHostView;
    mspDocument = pDocument->shared_from_this();
    mRenderer.mspDocument = mspDocument;
    mspWin->show();
}

void 
LxCanvasImp::updateFrame (DocumentPtr spDocument) 
{
    mController.updateFrame(mpHostView->shared_from_this(), mspWin->keyboard());
    mRenderer.update();

    mspWin->invalidate(); 
}

void 
LxCanvasImp::handleEvent (std::string evt, lx0::core::lxvar params)
{
    bool bInvalidate = true;

    if (evt == "redraw")
        bInvalidate = true;
    else if (evt == "move_forward")
        move_forward(gCamera, params.asFloat());
    else if (evt == "move_backward")
        move_backward(gCamera, params.asFloat());
    else if (evt == "move_left")
        move_left(gCamera, params.asFloat());
    else if (evt == "move_right")
        move_right(gCamera, params.asFloat());
    else if (evt == "move_up")
        move_vertical(gCamera, params.asFloat());
    else if (evt == "move_down")
        move_vertical(gCamera, -params.asFloat());
    else if (evt == "select_object")
    {
        auto& spItem = mRenderer.select( params.at(0).asInt(), params.at(1).asInt() );
        auto spElement = spItem->getData<ElementPtr>();
        std::string name = spElement
            ? spElement->attr("image").query("unknown").c_str()
            : "no associated element";
        printf("Select: %s (%s)\n", spItem->spMaterial->mShaderFilename.c_str(), name.c_str());
    }
    else if (evt == "cycle_viewmode")
        mRenderer.cycleViewMode();
    else
    {
        lx_warn("Unhandled event '%s'", evt.c_str());
        bInvalidate = false;
    }

    if (bInvalidate)
        mspWin->invalidate();
}

//===========================================================================//

class SkyMap : public Renderable
{
public:
    virtual void update(ElementPtr spElement)
    {
        mRotation += glgeom::two_pi() / 30.0f;
    }

    virtual void generate(ElementPtr spElement,
                      RasterizerGL& rasterizer,
                      lx0::prototype::Camera& cam1,
                      CameraPtr spCamera, 
                      LightSetPtr spLightSet, 
                      RenderList& list)
    {
        if (!mspItem)
        {
            lx0::blendreader::BlendReader reader;
            reader.open("media2/models/unit_hemisphere-000.blend");
            
            auto meshBlocks = reader.getBlocksByType("Mesh");
            for (auto it = meshBlocks.begin(); it != meshBlocks.end(); ++it)
            {
                auto spBlock = *it;
                auto spMesh = reader.readObject( spBlock->address );
                const auto totalVertices = spMesh->field<int>("totvert");
                const auto totalFaces = spMesh->field<int>("totface");

                std::vector<glgeom::point3f>  positions;
                std::vector<glgeom::vector3f> normals;
                std::vector<glgeom::color3f>  colors;
                std::vector<unsigned short> indicies;

                positions.reserve(totalVertices);
                normals.reserve(totalVertices);
                colors.reserve(totalVertices);
                indicies.reserve(totalFaces * 4);

                auto spVerts = reader.readObject( spMesh->field<unsigned __int64>("mvert") );
                for (int i = 0; i < totalVertices; ++i)
                {
                    glgeom::point3f p;
                    p.x = spVerts->field<float>("co", 0);
                    p.y = spVerts->field<float>("co", 1);
                    p.z = spVerts->field<float>("co", 2);
                    p.vec *= 200;
                    positions.push_back(p);

                    glgeom::vector3f n;
                    n.x = spVerts->field<short>("no", 0) / float(std::numeric_limits<short>::max());
                    n.y = spVerts->field<short>("no", 1) / float(std::numeric_limits<short>::max());
                    n.z = spVerts->field<short>("no", 2) / float(std::numeric_limits<short>::max());
                    normals.push_back(n);

                    colors.push_back( glgeom::color3f(1, 1, 1) );

                    spVerts->next();
                }

                auto spFaces = reader.readObject( spMesh->field<unsigned __int64>("mface") );
                for (int i = 0; i < totalFaces; ++i)
                {
                    int vi[4];
                    vi[0] = spFaces->field<int>("v1");
                    vi[1] = spFaces->field<int>("v2");
                    vi[2] = spFaces->field<int>("v3");
                    vi[3] = spFaces->field<int>("v4");

                    // Convert tris into degenerate quads
                    if (vi[3] == 0)
                        vi[3] = vi[2];

                    for (int j = 0; j < 4; ++j)
                        indicies.push_back(vi[j]);

                    spFaces->next();
                }
                 
                glgeom::point3f pos( 0.0f, 0.0f, 0.0f);
                pos.z = spElement->document()->getComponent<PhysicsSubsystem>("physics2")->drop(pos.x, pos.y);
                pos.z -= 20.0f;
                mPosition = pos;

                auto spGeom = rasterizer.createQuadList(indicies, positions, normals, colors);

                auto spMat = rasterizer.createMaterial("media2/shaders/glsl/fragment/skymap.frag");
                spMat->mBlend = false;
                spMat->mWireframe = false;
                spMat->mZTest = false;
                spMat->mZWrite = false;
                spMat->mTextures[0] = rasterizer.createTexture("media2/textures/skymaps/polar/bluesky_grayclouds.png");

                auto pItem = new Item;
                pItem->spCamera   = spCamera;
                pItem->spLightSet = spLightSet;
                pItem->spMaterial = spMat;
                pItem->spTransform = rasterizer.createTransformEye(pos.x, pos.y, pos.z, glgeom::radians(0.0f));
                pItem->spGeometry = spGeom;
            
                mspItem.reset(pItem);
            }
        }

        mspItem->spTransform = rasterizer.createTransformEye(mPosition.x, mPosition.y, mPosition.z, mRotation);
        list.push_back(0, mspItem);
    }

protected:
    glgeom::point3f                 mPosition;
    glgeom::radians                 mRotation;
    ItemPtr           mspItem;
};

//===========================================================================//

class SpriteShared
{
public:
    MaterialPtr _ensureMaterial (RasterizerGL& rasterizer, std::string image)
    {
        if (!mspMaterial)
        {
            auto spMat = rasterizer.createMaterial("media2/shaders/glsl/fragment/texture1_fog.frag");
            spMat->mBlend = true;
            spMat->mFilter = GL_NEAREST;
            spMat->mTextures[0] = rasterizer.createTexture(image.c_str());
            mspMaterial = spMat;
        }
        return mspMaterial;
    }

    GeometryPtr  _ensureGeom (RasterizerGL& rasterizer)
    {
        if (!mspGeom)
        {
            const float kSize = 1.0f;

            std::vector<glgeom::point3f> positions;
            positions.push_back( glgeom::point3f(0, 0, 0) );
            positions.push_back( glgeom::point3f(kSize, 0, 0) );
            positions.push_back( glgeom::point3f(kSize, 0, kSize) );
            positions.push_back( glgeom::point3f(0, 0, kSize) );

            std::vector<glgeom::vector3f> normals;
            normals.push_back( glgeom::vector3f(0, 1, 0) );
            normals.push_back( glgeom::vector3f(0, 1, 0) );
            normals.push_back( glgeom::vector3f(0, 1, 0) );
            normals.push_back( glgeom::vector3f(0, 1, 0) );

            std::vector<glgeom::color3f> colors;
            colors.push_back( glgeom::color3f(0, 1, 0) );
            colors.push_back( glgeom::color3f(1, 1, 0) );
            colors.push_back( glgeom::color3f(1, 0, 0) );
            colors.push_back( glgeom::color3f(0, 0, 0) );

            std::vector<unsigned short> indicies;
            indicies.push_back(0);
            indicies.push_back(1);
            indicies.push_back(2);
            indicies.push_back(3);

            mspGeom = rasterizer.createQuadList(indicies, positions, normals, colors);
        }
        return mspGeom;
    }

    static std::shared_ptr<SpriteShared> acquire()
    {
        static std::weak_ptr<SpriteShared> s_wpSingleton;
        auto spSingleton = s_wpSingleton.lock();
        if (!spSingleton)
        {
            spSingleton.reset(new SpriteShared);
            s_wpSingleton = spSingleton;
        }
        return spSingleton;
    }

protected:
    MaterialPtr       mspMaterial;
    GeometryPtr       mspGeom;
};

class Sprite : public Renderable
{
public:
    virtual void generate(ElementPtr spElement,
                      RasterizerGL& rasterizer,
                      lx0::prototype::Camera& cam1,
                      CameraPtr spCamera, 
                      LightSetPtr spLightSet, 
                      RenderList& list)
    {
        if (!mspItem)
        {
            std::string image = spElement->attr("image").asString();

            auto pItem = new Item;
            pItem->setData<ElementPtr>(spElement);
            pItem->spCamera   = spCamera;
            pItem->spLightSet = spLightSet;
            pItem->spMaterial = SpriteShared::acquire()->_ensureMaterial(rasterizer, image);
            pItem->spGeometry = SpriteShared::acquire()->_ensureGeom(rasterizer);
            
            mspItem.reset(pItem);
        }

        lxvar attrPos = spElement->attr("position");
        glgeom::point3f pos( attrPos.at(0).asFloat(), attrPos.at(1).asFloat(), 0.0f);
        pos.z = spElement->document()->getComponent<PhysicsSubsystem>("physics2")->drop(pos.x, pos.y);

        float scale = spElement->attr("scale").query(1.0f);
        mspItem->spTransform = rasterizer.createTransformBillboardXYS(pos.x, pos.y, pos.z, scale, scale, scale);

        list.push_back(1, mspItem);
    }

protected:
    ItemPtr           mspItem;
};

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
        spEngine->addDocumentComponent("rasterizer", lx0::createIRasterizer);
        spEngine->addViewPlugin("LxCanvas", [] (View* pView) { return new LxCanvasImp; });
        spEngine->addElementComponent("Terrain", "runtime", [](ElementPtr spElem) { return new Terrain::Runtime(spElem); }); 
        spEngine->addElementComponent("Terrain", "renderable", [](ElementPtr spElem) { return new Terrain::Render; });
        spEngine->addElementComponent("Sprite", "renderable", [](ElementPtr spElem) { return new Sprite; });
        spEngine->addElementComponent("SkyMap", "renderable", [](ElementPtr spElem) { return new SkyMap; });
        
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
