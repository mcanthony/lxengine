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

#include <lx0/extensions/rasterizer.hpp>
#include <lx0/subsystem/javascript.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>
#include <lx0/util/blendload.hpp>
#include <lx0/prototype/misc.hpp>
#include <glgeom/prototype/camera.hpp>
#include <lx0/subsystem/shaderbuilder.hpp>
#include "renderer.hpp"

using namespace lx0;

//===========================================================================//

class CameraComp : public Element::Component
{
public:
    virtual     const char* name() const { return "rasterizer"; }
    virtual     lx0::uint32 flags() const { return eSkipUpdate; }

    CameraComp (lx0::CameraPtr spCam) : mspCamera (spCam) {}

    virtual void    onValueChange       (ElementPtr spElem)
    {
        glgeom::vector3f position = spElem->value().find("position").convert();
        glgeom::point3f target = spElem->value().find("look_at").convert();

        mspCamera->viewMatrix = glm::lookAt(position.vec, target.vec, glm::vec3(0, 0, 1));
    }

    lx0::CameraPtr       mspCamera; 
};

//===========================================================================//

class Renderer : public View::Component
{
public:
    virtual void initialize(ViewPtr spView)
    {
        mspRasterizer.reset( new RasterizerGL );
        mspRasterizer->initialize();

        mspLightSet = mspRasterizer->createLightSet();

        lx0::processIncludeDocument(spView->document());

        spView->document()->iterateElements([&](ElementPtr spElem) -> bool { 
            _onElementAddRemove(spElem, true); 
            return false; 
        });

        spView->sendEvent("redraw");
    }  

    virtual void render (void)	
    {
        RenderAlgorithm algorithm;
        algorithm.mClearColor = glgeom::color4f(0.0f, 0.3f, 0.32f, 1.0f);
        GlobalPass pass[4];
        pass[0].spCamera = mspCamera;
        pass[0].spLightSet = mspLightSet;
        algorithm.mPasses.push_back(pass[0]);

        RenderList instances;
        for (auto it = mGeometry.begin(); it != mGeometry.end(); ++it)
            instances.push_back(0, *it);

        mspRasterizer->beginFrame(algorithm);

        for (auto it = instances.begin(); it != instances.end(); ++it)
        {
            mspRasterizer->rasterizeList(algorithm, it->second.list);
        }

        mspRasterizer->endFrame();
    }

    virtual void onElementAdded (DocumentPtr spDocument, ElementPtr spElem) 
    {
        lx_debug("Element '%s' added", spElem->tagName().c_str());
        _onElementAddRemove(spElem, true); 
    }

    virtual void handleEvent (std::string evt, lxvar params)
    {
        if (evt == "screenshot")
        {
            std::string filename = params.is_defined() ? params.as<std::string>() : "screenshot.png";

            glgeom::image3f img;
            render();
            mspRasterizer->readBackBuffer(img);

            lx0::save_png(img, filename.c_str());
        }
    }

protected:
    MaterialPtr _findMaterial(ElementPtr spElem)
    {
        auto it = mMaterials.find(query(spElem->attr("material"), ""));
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
            std::string name = query(spElem->attr("id"), "");
            lx0::lxvar  graph = spElem->value().find("graph");
            
            try
            {
                lx0::ShaderBuilder::Material material;
                mShaderBuilder.buildShaderGLSL(material, graph);
            
                auto spMat = mspRasterizer->createMaterial(material.uniqueName, material.source, material.parameters);
                spMat->trimParameterTypes();
                mMaterials.insert(std::make_pair(name, spMat));
            }
            catch (lx0::error_exception& e)
            {
                // Consume the exception
                lx_warn("Could not create Material with id='%s'\n\nDetails:%s\n", name.c_str(), e.what());
            }
        }
        else if (tag == "Sphere")
        {
            glgeom::point3f center = spElem->value().find("center").convert();
            float radius = spElem->value().find("radius").convert(.5f);
            glgeom::vector3f scale(radius / .5f, radius / .5f, radius / .5f);

            auto pInstance = new Instance;
            pInstance->spMaterial = _findMaterial(spElem);
            pInstance->spTransform = mspRasterizer->createTransform(scale, center);
            pInstance->spGeometry = _loadMesh("media2/models/unit_sphere-000.blend");

            mGeometry.push_back(InstancePtr(pInstance));
        }
        else if (tag == "Cube")
        {
            glgeom::point3f center = spElem->value().find("center").convert();
            glgeom::vector3f scale = spElem->value().find("scale").convert();

            auto pInstance = new Instance;
            pInstance->spMaterial = _findMaterial(spElem);
            pInstance->spTransform = mspRasterizer->createTransform(scale, center);
            pInstance->spGeometry = _loadMesh("media2/models/unit_cube-000.blend");
            
            mGeometry.push_back(InstancePtr(pInstance));
        }
        else if (tag == "Cylinder")
        {
            glgeom::point3f center = spElem->value().find("base").convert();
            center.z += .5f;
            float radius = spElem->value().find("radius").convert();
            glgeom::vector3f scale(radius / .5f, radius / .5f, radius / .5f);

            auto pInstance = new Instance;
            pInstance->spMaterial = _findMaterial(spElem);
            pInstance->spTransform = mspRasterizer->createTransform(scale, center);
            pInstance->spGeometry = _loadMesh("media2/models/unit_geometry/unit_cylinder-001.blend");
            
            mGeometry.push_back(InstancePtr(pInstance));
        }
        else if (tag == "Cone")
        {
            glgeom::point3f center = spElem->value().find("base").convert();
            center.z += .5f;
            float radius = spElem->value().find("radius").convert();
            glgeom::vector3f scale(radius / .5f, radius / .5f, radius / .5f);

            auto pInstance = new Instance;
            pInstance->spMaterial = _findMaterial(spElem);
            pInstance->spTransform = mspRasterizer->createTransform(scale, center);
            pInstance->spGeometry = _loadMesh("media2/models/unit_geometry/unit_cone-000.blend");
            
            mGeometry.push_back(InstancePtr(pInstance));
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

                vector3f xaxis, yaxis;
                glgeom::perpendicular_axes_smooth(normal, xaxis, yaxis);

                lx_assert( is_finite(normal) );
                lx_assert( is_finite(xaxis) );
                lx_assert( is_finite(yaxis) );
                lx_assert( is_orthonormal(normal, xaxis, yaxis) );

                // Create the new basis.  This creates a right-handed coordinate system where
                // -z is into the screen / increasing depth.
                //
                // A basis transformation from one coordinate system to another via a matrix
                // is done by setting each column of the matrix to the vectors representing
                // the new coordinate system (in terms of the old coordinate system).
                //
                auto R = glm::mat4(glm::mat3(xaxis.vec, yaxis.vec, normal.vec));
                auto T = glm::translate(mat, -(normal * d).vec);
                mat = T * R;
            }


            auto pInstance = new Instance;
            pInstance->spMaterial = _findMaterial(spElem);
            pInstance->spTransform = mspRasterizer->createTransform(glm::mat4(mat));
            pInstance->spGeometry = _loadMesh("media2/models/plane_1k-001.blend");
            
            mGeometry.push_back(InstancePtr(pInstance));
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
            mspCamera = mspRasterizer->createCamera(glgeom::pi() / 3.0f, 0.1f, 2000.0f, view);

            spElem->attachComponent(new CameraComp(mspCamera));
        }
        else if (tag == "Texture")
        {
            std::string id = spElem->attr("id").as<std::string>();
            std::string type = spElem->value().find("type").as<std::string>();
            int width = spElem->value().find("width").as<int>();
            int height = spElem->value().find("height").as<int>();
            std::string funcName = spElem->value().find("function").as<std::string>();

#ifdef _DEBUG
            width = std::min(width, 32);
            height = std::min(height, 32);
#endif

            auto spIJavascriptDoc = spElem->document()->getComponent<IJavascriptDoc>();

            if (type == "cubemap")
            {
                glgeom::cubemap3f cubemap(width, height);
                            
                std::cout << "Generating texture (cubemap)...";
                spIJavascriptDoc->runInContext([&](void) 
                {
                    auto func = spIJavascriptDoc->acquireFunction<glm::vec3 (float, float, float)>( funcName.c_str() );
                    glgeom::generate_cube_map(cubemap, [func](const glm::vec3& p) -> glm::vec3 {
                        return func(p.x, p.y, p.z);
                    });
                });
                std::cout << "done." << std::endl;

                mspRasterizer->cacheTexture(id, mspRasterizer->createTextureCubeMap(cubemap));
            }
            else if (type == "2d")
            {
                glgeom::image3f image(width, height);

                std::cout << "Generating texture (2d)...";
                spIJavascriptDoc->runInContext([&](void) 
                {
                    auto func = spIJavascriptDoc->acquireFunction<glm::vec3 (float, float)>( funcName.c_str() );
                    glgeom::generate_image3f(image, [func](glm::vec2 p, glm::ivec2) -> glm::vec3 {
                        return func(p.x, p.y);
                    });
                });
                std::cout << "done." << std::endl;

                mspRasterizer->cacheTexture(id, mspRasterizer->createTexture3f(image));
            }
        }
        else 
        { 
        }
    }

    static lx0::GeometryPtr 
    _createGeometry (RasterizerGL* spRasterizer, const char* filename, float scale)
    {

        glgeom::primitive_buffer primitive;
        glm::mat4 scaleMat = glm::scale(glm::mat4(), glm::vec3(scale, scale, scale));
        lx0::primitive_buffer_from_blendfile(primitive, filename, scaleMat);
            
        auto spGeometry = spRasterizer->createQuadList(primitive.indices, primitive.face.flags, primitive.vertex.positions, primitive.vertex.normals, primitive.vertex.colors);
        spGeometry->mBBox = primitive.bbox;

        return spGeometry;
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
            auto spGeom = _createGeometry(mspRasterizer.get(), filename, 1.0f);
            mMeshes.insert(std::make_pair(filename, spGeom));
            return spGeom;
        }
    }


    std::shared_ptr<RasterizerGL> mspRasterizer;
    lx0::ShaderBuilder            mShaderBuilder;

    lx0::CameraPtr                      mspCamera;       // Camera shared by all instances
    lx0::LightSetPtr                    mspLightSet;

    std::map<std::string,GeometryPtr>   mMeshes;
    std::vector<InstancePtr>            mGeometry;
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

    virtual void onKeyDown (lx0::ViewPtr spView, int keyCode) 
    { 
        if (keyCode == lx0::KC_P)
            spView->sendEvent("screenshot");
    }

    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard)
    {
        if (keyboard.bDown[KC_ESCAPE])
            Engine::acquire()->sendEvent("quit");
        
        if (keyboard.bDown[KC_R])
            spView->sendEvent("redraw", lxvar());

        if (keyboard.bDown[KC_W])
            spView->document()->sendEvent("move_forward", lxvar(.04f));
        if (keyboard.bDown[KC_S])
            spView->document()->sendEvent("move_forward", lxvar(-.04f));
        if (keyboard.bDown[KC_A])
            spView->document()->sendEvent("move_right", lxvar(-.04f));
        if (keyboard.bDown[KC_D])
            spView->document()->sendEvent("move_right", lxvar(.04f));
        if (keyboard.bDown[KC_Q])
            spView->document()->sendEvent("move_up", lxvar(-.04f));
        if (keyboard.bDown[KC_E])
            spView->document()->sendEvent("move_up", lxvar(.04f));
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

