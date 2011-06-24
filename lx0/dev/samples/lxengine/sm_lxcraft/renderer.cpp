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

using namespace lx0;



//===========================================================================//

class CameraComp : public Element::Component
{
public:
    CameraComp (lx0::CameraPtr spCam) : mspCamera (spCam) {}

    virtual void    onValueChange       (ElementPtr spElem, lxvar value)
    {
        resetViewDirection(spElem);
    }

    void resetViewDirection (ElementPtr spElem)
    {
        auto value = spElem->value();
        glgeom::vector3f position = value.find("position").convert();
        glgeom::point3f target = value.find("look_at").convert();

        auto view = glm::lookAt(position.vec, target.vec, glm::vec3(0, 0, 1));
        mspCamera->viewMatrix = view;
    }

    lx0::CameraPtr       mspCamera; 
};

//===========================================================================//

class Renderable : public Element::Component
{
public:
    virtual void    generate (RenderList& list) = 0;
};

typedef std::shared_ptr<Renderable> RenderablePtr;

class RdVoxelCell : public Renderable
{
public:
    RdVoxelCell (RasterizerGL* pRasterizer, MeshCachePtr spMeshCache)
    {
        for (int z = 0; z < 4; ++z)
        {
            for (int y = 0; y < 16; ++y)
            {
                for (int x = 0; x < 16; ++x)
                {
                    glgeom::color3f color(.5f + x / 31.0f, .5f + y / 31.0f, .5f + z / 7.0f);
                    if ((x + y + z) % 2 == 1)
                        color = glgeom::color3f(.2f, .2f, .8f);
                    auto spMat = pRasterizer->createSolidColorMaterial(color);

                    ItemPtr spItem(new Item);
                    spItem->spMaterial = spMat;
                    spItem->spTransform = pRasterizer->createTransform(glgeom::vector3f(1, 1, 1), glgeom::point3f(x, y, z));
                    spItem->spGeometry = spMeshCache->acquire("media2/models/unit_cube-000.blend");

                    mBlocks[z * 16 * 16 + y * 16 + x] = spItem;
                }
            }
        }
    }

    virtual void onValueChange (ElementPtr spElem, lxvar value)
    {
    }

    virtual void generate (RenderList& list)
    {
        for (int i = 0; i < 16 * 16 * 4; ++i)
            list.push_back(0, mBlocks[i]);
    }

    ItemPtr mBlocks[16 * 16 * 4];
};

//===========================================================================//

class Renderer : public View::Component
{
public:
    virtual void initialize(ViewPtr spView)
    {
        mspRasterizer.reset( new RasterizerGL );
        mspRasterizer->initialize();

        mspMeshCache.reset( new MeshCache(mspRasterizer) );

        mspCamera = mspRasterizer->createCamera(60.0f, 0.01f, 5000.0f, glm::lookAt(glm::vec3(24, 48, 32), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1)));
        mspLightSet = mspRasterizer->createLightSet();

        spView->document()->iterateElements([&](ElementPtr spElem) -> bool { 
            _onElementAddRemove(spElem, true); 
            return false; 
        });
    }  

    virtual void render (void)	
    {
        RenderAlgorithm algorithm;
        algorithm.mClearColor = glgeom::color4f(0.1f, 0.1f, 0.0f, 1.0f);
        GlobalPass pass[4];
        pass[0].tbFlatShading = true;
        pass[0].spCamera = mspCamera;
        pass[0].spLightSet = mspLightSet;
        algorithm.mPasses.push_back(pass[0]);

        RenderList items;
        for (auto it = mRenderables.begin(); it != mRenderables.end(); ++it)
            (*it)->generate(items);

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
        using namespace glgeom;

        const std::string tag = spElem->tagName();

        if (tag == "VoxelCell")
        {
            mRenderables.push_back(RenderablePtr(new RdVoxelCell(mspRasterizer.get(), mspMeshCache)));
        }
        else if (tag == "Camera")
        {
            mspCamera = mspRasterizer->createCamera(60.0f, 0.1f, 2000.0f, glm::mat4());
            auto pComp = new CameraComp(mspCamera);
            pComp->resetViewDirection(spElem);
            spElem->attachComponent("rasterizer", pComp);
        }
        else 
        { 
        }
    }

    std::shared_ptr<RasterizerGL>       mspRasterizer;
    std::auto_ptr<MeshCache>            mspMeshCache;

    lx0::CameraPtr                      mspCamera;
    lx0::LightSetPtr                    mspLightSet;
    std::vector<RenderablePtr>          mRenderables;
    
    std::map<std::string,MaterialPtr>   mMaterials;
};

//===========================================================================//

class UIBindingImp : public lx0::UIBinding
{
public:

    UIBindingImp()
    {
        // _sendDocEventOnKeyIsDown(KC_W, "move_forward", lxvar(.1f));
        // _sendDocEventOnKeyDown(KC_R, "redraw", lxvar());
    }

    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard)
    {
        const float step = 0.4f;

        if (keyboard.bDown[KC_ESCAPE])
            Engine::acquire()->sendMessage("quit");
        
        if (keyboard.bDown[KC_R])
            spView->sendEvent("redraw", lxvar());

        if (keyboard.bDown[KC_W])
            spView->document()->sendEvent("move_forward", lxvar(step));
        if (keyboard.bDown[KC_S])
            spView->document()->sendEvent("move_forward", lxvar(-step));
        if (keyboard.bDown[KC_A])
            spView->document()->sendEvent("move_right", lxvar(-step));
        if (keyboard.bDown[KC_D])
            spView->document()->sendEvent("move_right", lxvar(step));
        if (keyboard.bDown[KC_Q])
            spView->document()->sendEvent("move_up", lxvar(-step));
        if (keyboard.bDown[KC_E])
            spView->document()->sendEvent("move_up", lxvar(step));
    }

    void onLDrag (ViewPtr spView, const MouseState& ms, const ButtonState& bs, KeyModifiers km)
    {
        const float horz = ms.deltaX() * -3.14f / 1000.0f;
        const float vert = ms.deltaY() * -3.1415f / 1000.0f;

        if (fabs(horz) > 1e-3f)
            spView->document()->sendEvent("rotate_horizontal", horz);
        if (fabs(vert) > 1e-3f)
            spView->document()->sendEvent("rotate_vertical", vert);

        spView->sendEvent("redraw");
    }
};

//===========================================================================//

class ControllerImp : public lx0::Controller
{
public:
    ControllerImp (lx0::Document* pDoc) : mpDocument(pDoc) {}

    virtual void handleEvent (std::string evt, lx0::lxvar params)
    {
        if (evt == "move_forward")
        {
            const float step = params.convert();
            auto spElem = mpDocument->getElementsByTagName("Camera")[0];
 
            lxvar val = spElem->value();
            glgeom::point3f pos = val["position"].convert();
            glgeom::point3f target = val["look_at"].convert();

            auto dir = glgeom::normalize(target - pos);
            pos += dir * step;
            target += dir * step;

            val.insert("position", lx0::lxvar_from(pos));
            val.insert("look_at", lx0::lxvar_from(target));

            spElem->value(val);

            mpDocument->view(0)->sendEvent("redraw");
        }
        else if (evt == "move_right")
        {
            const float step = params.convert();
            auto spElem = mpDocument->getElementsByTagName("Camera")[0];
 
            lxvar val = spElem->value();
            glgeom::point3f pos = val["position"].convert();
            glgeom::point3f target = val["look_at"].convert();

            auto dir = glgeom::normalize(target - pos);
            dir = glgeom::cross(dir, glgeom::vector3f(0, 0, 1));
            pos += dir * step;
            target += dir * step;

            val.insert("position", lx0::lxvar_from(pos));
            val.insert("look_at", lx0::lxvar_from(target));

            spElem->value(val);

            mpDocument->view(0)->sendEvent("redraw");
        }
        else if (evt == "move_up")
        {
            const float step = params.convert();
            auto spElem = mpDocument->getElementsByTagName("Camera")[0];
 
            lxvar val = spElem->value();
            glgeom::point3f pos = val["position"].convert();
            glgeom::point3f target = val["look_at"].convert();

            pos.z += step;
            target.z += step;

            val.insert("position", lx0::lxvar_from(pos));
            val.insert("look_at", lx0::lxvar_from(target));

            spElem->value(val);

            mpDocument->view(0)->sendEvent("redraw");
        }
        else if (evt == "rotate_horizontal")
        {
            const float step = params.convert();
            auto spElem = mpDocument->getElementsByTagName("Camera")[0];
            lxvar val = spElem->value();
            glgeom::point3f position = val["position"];
            glgeom::point3f target = val["look_at"];

            const auto view = target - position;
            const glgeom::vector3f rotated = rotate(view, glgeom::vector3f(0, 0, 1), float(params));
            target = position + rotated;

            val.insert("position", lx0::lxvar_from(position));
            val.insert("look_at", lx0::lxvar_from(target));

            spElem->value(val);
        }
        else if (evt == "rotate_vertical")
        {
            const float step = params.convert();
            auto spElem = mpDocument->getElementsByTagName("Camera")[0];
            lxvar val = spElem->value();
            glgeom::point3f position = val["position"];
            glgeom::point3f target = val["look_at"];

            const auto view = target - position;
            const auto right = cross(view, glgeom::vector3f(0, 0, 1));
            const glgeom::vector3f rotated = rotate(view, right, float(params));
            target = position + rotated;

            val.insert("position", lx0::lxvar_from(position));
            val.insert("look_at", lx0::lxvar_from(target));

            spElem->value(val);
        }

    }

    lx0::Document* mpDocument;
};

lx0::View::Component*   create_renderer()       { return new Renderer; }
lx0::UIBinding*         create_uibinding()      { return new UIBindingImp; }
lx0::Controller*        create_controller(DocumentPtr spDoc)    { return new ControllerImp(spDoc.get()); }

