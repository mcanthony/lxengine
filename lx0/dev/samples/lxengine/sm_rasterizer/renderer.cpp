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
        pass[0].tbFlatShading = true;
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
    MaterialPtr _findMaterial(ElementPtr spElem)
    {
        auto it = mMaterials.find(spElem->attr("material").query(""));
        if (it != mMaterials.end())
            return it->second;
        else
            return mspRasterizer->createPhongMaterial(glgeom::material_phong_f());
    }

    void _onElementAddRemove (ElementPtr spElem, bool bAdd)
    {
        using namespace glgeom;

        const std::string tag = spElem->tagName();

        if (tag == "Material")
        {
            glgeom::material_phong_f phong;
            phong.emmissive = spElem->value().find("emmissive").convert(color3f(0, 0, 0));
            phong.diffuse = spElem->value().find("diffuse").convert(color3f(1, 1, 1));
            phong.specular = spElem->value().find("specular").convert(color3f(0, 0, 0));
            phong.specular_n = spElem->value().find("specular_n").convert(8.0f);

            std::string name = spElem->attr("id").query("");
            auto spMat = mspRasterizer->createPhongMaterial(phong);

            mMaterials.insert(std::make_pair(name, spMat));
        }
        else if (tag == "Sphere")
        {
            glgeom::point3f center = spElem->value().find("center").convert();
            float radius = spElem->value().find("radius").convert(.5f);
            glgeom::vector3f scale(radius / .5f, radius / .5f, radius / .5f);

            auto pItem = new Item;
            pItem->spCamera   = mspCamera;
            pItem->spLightSet = mspLightSet;
            pItem->spMaterial = _findMaterial(spElem);
            pItem->spTransform = mspRasterizer->createTransform(scale, center);
            pItem->spGeometry = _loadMesh("media2/models/unit_sphere-000.blend");

            mGeometry.push_back(ItemPtr(pItem));
        }
        else if (tag == "Cube")
        {
            glgeom::point3f center = spElem->value().find("center").convert();
            glgeom::vector3f scale = spElem->value().find("scale").convert();

            auto pItem = new Item;
            pItem->spCamera   = mspCamera;
            pItem->spLightSet = mspLightSet;
            pItem->spMaterial = _findMaterial(spElem);
            pItem->spTransform = mspRasterizer->createTransform(scale, center);
            pItem->spGeometry = _loadMesh("media2/models/unit_cube-000.blend");
            
            mGeometry.push_back(ItemPtr(pItem));
        }
        else if (tag == "Cylinder")
        {
            glgeom::point3f center = spElem->value().find("base").convert();
            center.z += .5f;
            float radius = spElem->value().find("radius").convert();
            glgeom::vector3f scale(radius / .5f, radius / .5f, radius / .5f);

            auto pItem = new Item;
            pItem->spCamera   = mspCamera;
            pItem->spLightSet = mspLightSet;
            pItem->spMaterial = _findMaterial(spElem);
            pItem->spTransform = mspRasterizer->createTransform(scale, center);
            pItem->spGeometry = _loadMesh("media2/models/unit_geometry/unit_cylinder-001.blend");
            
            mGeometry.push_back(ItemPtr(pItem));
        }
        else if (tag == "Cone")
        {
            glgeom::point3f center = spElem->value().find("base").convert();
            center.z += .5f;
            float radius = spElem->value().find("radius").convert();
            glgeom::vector3f scale(radius / .5f, radius / .5f, radius / .5f);

            auto pItem = new Item;
            pItem->spCamera   = mspCamera;
            pItem->spLightSet = mspLightSet;
            pItem->spMaterial = _findMaterial(spElem);
            pItem->spTransform = mspRasterizer->createTransform(scale, center);
            pItem->spGeometry = _loadMesh("media2/models/unit_geometry/unit_cone-000.blend");
            
            mGeometry.push_back(ItemPtr(pItem));
        }
        else if (tag == "Plane")
        {
            glgeom::vector3f normal = spElem->value().find("normal").convert();
            normal = normalize(normal);
            float d = spElem->value().find("d").convert();

            // The unit geometry has the normal pointing toward +Z.  Create a new basis
            // where +Z points in the direction of the specified normal.
            glm::mat4 mat;
            {
                using namespace glgeom;

                vector3f right;
                if (abs((abs(normal.x) - 1.0f)) < 0.1f)
                    right = vector3f(0, 1, 0);
                else
                    right = vector3f(1, 0, 0);

                vector3f yaxis = normalize( cross(normal, right) );
                vector3f xaxis = normalize( cross(yaxis, normal) );

                // Create the new basis.  This creates a right-handed coordinate system where
                // -z is into the screen / increasing depth.
                //
                // A basis transformation from one coordinate system to another via a matrix
                // is done by setting each column of the matrix to the vectors representing
                // the new coordinate system (in terms of the old coordinate system).
                //
                // The final step is simply to convert that basis transformation matrix into
                // quaternion form and return the value.
                //
                mat = glm::mat4(glm::mat3(xaxis.vec, yaxis.vec, normal.vec));

                // Now add in the translation
                mat = glm::translate(mat, (normal * d).vec);
            }


            auto pItem = new Item;
            pItem->spCamera   = mspCamera;
            pItem->spLightSet = mspLightSet;
            pItem->spMaterial = _findMaterial(spElem);
            pItem->spTransform = mspRasterizer->createTransform(glm::mat4(mat));
            pItem->spGeometry = _loadMesh("media2/models/plane_1k-000.blend");
            
            mGeometry.push_back(ItemPtr(pItem));
        }
        else if (tag == "Light")
        {
            LightPtr spLight = mspRasterizer->createLight();
            spLight->position = spElem->value().find("position").convert();
            spLight->color    = spElem->value().find("color").convert();

            mspLightSet->mLights.push_back(spLight);
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

    GeometryPtr _loadMesh (const char* filename)
    {
        auto it = mMeshes.find(filename);
        if (it != mMeshes.end())
        {
            lx_debug("Reusing cache for '%s'", filename);
            return it->second;
        }
        else
        {
            auto spGeom = lx0::quadlist_from_blendfile(*mspRasterizer.get(), filename);
            mMeshes.insert(std::make_pair(filename, spGeom));
            return spGeom;
        }
    }


    std::shared_ptr<RasterizerGL> mspRasterizer;

    lx0::CameraPtr       mspCamera;       // Camera shared by all items
    lx0::LightSetPtr     mspLightSet;

    std::map<std::string,GeometryPtr>   mMeshes;
    std::vector<ItemPtr>                mGeometry;
    std::map<std::string,MaterialPtr>   mMaterials;
};

//===========================================================================//

class UIBindingImp : public lx0::UIBinding
{
public:

    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard)
    {
        if (keyboard.bDown[KC_ESCAPE])
            Engine::acquire()->sendMessage("quit");
        if (keyboard.bDown[KC_R])
            spView->sendEvent("redraw", lxvar());
        if (keyboard.bDown[KC_W])
            spView->sendEvent("move_forward", lxvar(.1f));
    }
};

lx0::View::Component*   create_renderer()       { return new Renderer; }
lx0::UIBinding*      create_uibinding()     { return new UIBindingImp; }

