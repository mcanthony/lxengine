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
#include <glgeom/prototype/camera.hpp>
#include "renderer.hpp"

using namespace lx0;

//===========================================================================//

class Renderer : public View::Component
{
public:
    virtual void initialize(ViewPtr spView)
    {
        mspRasterizer.reset( new RasterizerGL );
        mspRasterizer->initialize();

        mspLightSet = mspRasterizer->createLightSet();

        spView->document()->iterateElements([&](ElementPtr spElem) -> bool { 
            _onElementAddRemove(spElem, true); 
            return false; 
        });
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
        for (auto it = mGeometry.begin(); it != mGeometry.end(); ++it)
            items.push_back(0, *it);

        mspRasterizer->beginScene(algorithm);

        for (auto it = items.begin(); it != items.end(); ++it)
        {
            mspRasterizer->rasterizeList(algorithm, it->second.list);
        }

        mspRasterizer->endScene();
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
        else if (tag == "Camera")
        {
            glgeom::vector3f position = spElem->value().find("position").convert();
            glgeom::point3f target = spElem->value().find("look_at").convert();

            auto view = glm::lookAt(position.vec, target.vec, glm::vec3(0, 0, 1));
            mspCamera = mspRasterizer->createCamera(60.0f, 0.1f, 2000.0f, view);
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

