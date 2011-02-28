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

#include "main.hpp"
#include "rasterizergl.hpp"
#include "terrain.hpp"

using namespace lx0::core;
using namespace lx0::prototype;
using namespace lx0::canvas::platform;

Camera             gCamera;

//===========================================================================//

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

    virtual void onUpdate (DocumentPtr spDocument)
    {
        const float terrainHeight = drop(gCamera.mPosition.x, gCamera.mPosition.y);
        const float deltaZ = (terrainHeight + 2.0f) - gCamera.mPosition.z;
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

        std::vector<ElementPtr> elems;
        elems.swap( mspDocument->getElements() );
        for (auto it = elems.begin(); it != elems.end(); ++it)
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
    RasterizerGL                rasterizer;
};

//===========================================================================//

class Controller2
{
public:
    virtual                 ~Controller2() {}

    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard) = 0;
};

class CameraController : public Controller2
{
public:
    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard);
};

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
    mpHostView = pHostView;
    mspDocument = pDocument->shared_from_this();
    mRenderer.mspDocument = mspDocument;
    mspWin->show();
}

void 
LxCanvasImp::updateFrame (DocumentPtr spDocument) 
{
    mController.updateFrame(mpHostView->shared_from_this(), mspWin->keyboard());
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
    else
    {
        lx_warn("Unhandled event '%s'", evt.c_str());
        bInvalidate = false;
    }

    if (bInvalidate)
        mspWin->invalidate();
}

//===========================================================================//

class Sprite : public Renderable
{
public:
    virtual void generate(ElementPtr spElement,
                      RasterizerGL& rasterizer,
                      Camera& cam1,
                      RasterizerGL::CameraPtr spCamera, 
                      RasterizerGL::LightSetPtr spLightSet, 
                      std::vector<RasterizerGL::ItemPtr>& list)
    {
        if (!mspItem)
        {
            lxvar attrPos = spElement->attr("position");
            point3 pos( attrPos.at(0).asFloat(), attrPos.at(1).asFloat(), 0.0f);
            pos.z = spElement->document()->getComponent<PhysicsSubsystem>("physics2")->drop(pos.x, pos.y);

            const float kSize = 16.0f;

            std::vector<point3> positions;
            positions.push_back( point3(0, 0, 0) );
            positions.push_back( point3(kSize, 0, 0) );
            positions.push_back( point3(kSize, 0, kSize) );
            positions.push_back( point3(0, 0, kSize) );

            std::vector<vector3> normals;
            normals.push_back( vector3(0, 1, 0) );
            normals.push_back( vector3(0, 1, 0) );
            normals.push_back( vector3(0, 1, 0) );
            normals.push_back( vector3(0, 1, 0) );

            std::vector<vector3> colors;
            colors.push_back( vector3(0, 1, 0) );
            colors.push_back( vector3(1, 1, 0) );
            colors.push_back( vector3(1, 0, 0) );
            colors.push_back( vector3(0, 0, 0) );

            std::vector<unsigned short> indicies;
            indicies.push_back(0);
            indicies.push_back(1);
            indicies.push_back(2);
            indicies.push_back(3);

            auto spGeom = rasterizer.createQuadList(indicies, positions, normals, colors);

            auto spMat = rasterizer.createMaterial("media2/shaders/glsl/fragment/solid_texture1.frag");
            spMat->mBlend = true;
            spMat->mFilter = GL_NEAREST;
            spMat->mTextures[0] = rasterizer.createTexture("media2/textures/icons/rltile-modified/tree_01.png");

            auto pItem = new RasterizerGL::Item;
            pItem->spCamera   = spCamera;
            pItem->spLightSet = spLightSet;
            pItem->spMaterial = spMat;
            pItem->spTransform = rasterizer.createTransform(pos.x, pos.y, pos.z);
            pItem->spGeometry = spGeom;
            
            mspItem.reset(pItem);
        }

        list.push_back(mspItem);
    }

protected:
    RasterizerGL::ItemPtr           mspItem;
};

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
        spEngine->addElementComponent("Terrain", "runtime", [](ElementPtr spElem) { return new Terrain::Runtime(spElem); }); 
        spEngine->addElementComponent("Terrain", "renderable", [](ElementPtr spElem) { return new Terrain::Render; });
        spEngine->addElementComponent("Sprite", "renderable", [](ElementPtr spElem) { return new Sprite; });
        
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
