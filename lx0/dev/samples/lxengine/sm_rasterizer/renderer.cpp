//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

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

#include <lx0/subsystem/rasterizer.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>
#include <lx0/util/blendload.hpp>
#include <lx0/prototype/misc.hpp>
#include "renderer.hpp"

using namespace lx0;

//===========================================================================//

class Scene : public Document::Component
{
public:
    friend class Renderer;

    Scene (std::shared_ptr<RasterizerGL> spRast)
        : mspRasterizer( spRast )
    {
    }

    virtual void onAttached (DocumentPtr spDocument) 
    {
        lx0::Camera2             gCamera;
        gCamera.mPosition = glgeom::point3f(4, 4, 4);
        gCamera.mTarget = glgeom::point3f(0, 0, 0);
        gCamera.mWorldUp = glgeom::vector3f(0, 0, 1);
        gCamera.mFov = 60.0f;
        gCamera.mNear = 0.1f;  
        gCamera.mFar = 100.0f; 

        mspCamera = mspRasterizer->createCamera(60.0f, 0.1f, 2000.0f, view_matrix(gCamera));
        mspLightSet = mspRasterizer->createLightSet();

        spDocument->iterateElements([&](ElementPtr spElem) -> bool { 
            _onElementAddRemove(spElem, true); 
            return false; 
        });
    }

    virtual void onElementAdded (DocumentPtr spDocument, ElementPtr spElem) 
    {
        lx_debug("Element '%s' added", spElem->tagName().c_str());
        _onElementAddRemove(spElem, true); 
    }

protected:
    void _onElementAddRemove (ElementPtr spElem, bool bAdd)
    {
        const std::string tag = spElem->tagName();

        if (tag == "Sphere")
        {
            glgeom::point3f center = spElem->value().find("center").convert();
            float radius = spElem->value().find("radius").convert(.5f);
            glgeom::vector3f scale(radius / .5f, radius / .5f, radius / .5f);

            auto pItem = new Item;
            pItem->spCamera   = mspCamera;
            pItem->spLightSet = mspLightSet;
            pItem->spMaterial = mspRasterizer->createMaterial("media2/shaders/glsl/fragment/diffuse_gray.frag");;
            pItem->spTransform = mspRasterizer->createTransform(center.x, center.y, center.z);
            pItem->spGeometry = lx0::quadlist_from_blendfile(*mspRasterizer.get(), "media2/models/unit_sphere-000.blend");

            //pGeom->setMaterial(spElem, spElem->attr("material").query(""));
            
            mGeometry.push_back(ItemPtr(pItem));
        }
        else 
        { 
        }
    }

    std::shared_ptr<RasterizerGL> mspRasterizer;


    lx0::CameraPtr       mspCamera;       // Camera shared by all items
    lx0::LightSetPtr     mspLightSet;

    std::vector<ItemPtr> mGeometry;
};

//===========================================================================//

class Renderer : public View::Component
{
public:
    virtual void initialize(ViewPtr spView)
    {
        // This is a bit awkward: the view needs to grab the document component
        // and only initialize it after the view is initialized.  This should
        // be simplified: the Renderer internally should add a Document::Component
        // to the document being viewed.
        //
        mspRasterizer.reset( new RasterizerGL );
        mspRasterizer->initialize();

        mpScene = new Scene(mspRasterizer);
        spView->document()->attachComponent("scene", mpScene);
    }  

    virtual void render (void)	
    {
        RenderAlgorithm algorithm;
        algorithm.mClearColor = glgeom::color4f(0.0f, 0.3f, 0.32f, 1.0f);
        GlobalPass pass[4];
        pass[0].bOverrideMaterial = false;
        pass[0].spMaterial = mspRasterizer->createMaterial("media2/shaders/glsl/fragment/solid.frag");
        algorithm.mPasses.push_back(pass[0]);

        RenderList items;
        for (auto it = mpScene->mGeometry.begin(); it != mpScene->mGeometry.end(); ++it)
            items.push_back(0, *it);

        mspRasterizer->beginScene(algorithm);

        for (auto it = items.begin(); it != items.end(); ++it)
        {
            mspRasterizer->rasterizeList(algorithm, it->second.list);
        }

        mspRasterizer->endScene();
    }

protected:
    Scene*                        mpScene;
    std::shared_ptr<RasterizerGL> mspRasterizer;
};

//===========================================================================//

class ControllerImp : public lx0::UIController
{
public:

    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard)
    {
        if (keyboard.bDown[KC_ESCAPE])
            Engine::acquire()->sendMessage("quit");
        if (keyboard.bDown[KC_W])
            spView->sendEvent("redraw", lxvar());
    }
};

lx0::View::Component*   create_renderer()       { return new Renderer; }
lx0::UIController*      create_controller()     { return new ControllerImp; }

