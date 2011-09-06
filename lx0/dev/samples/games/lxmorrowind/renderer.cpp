//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

#include <iostream>
#include <glgeom/ext/primitive_buffer.hpp>
#include <lx0/subsystem/rasterizer.hpp>
#include <lx0/subsystem/shaderbuilder.hpp>
#include <lx0/util/misc.hpp>

#include "lxextensions/material_handle.hpp"

using namespace lx0;
using namespace glgeom;

//===========================================================================//

/*
    Experimental class designed to allow a std::function<> callback to be
    associated with a particular Element objects value being changed.

    This is "experimental" since, while useful, the core API is intended to
    be as lean as possible - and it's not clear if this wrapper should be
    part of the API or if there are ways to accomplish the same while not
    forcing the user to learn another concept.
 */
class ValueChangeListener : public Element::Component
{
public:
    ValueChangeListener (std::function<void (ElementPtr)> func) : mFunc (func) {}
    virtual void onValueChange (ElementPtr spElem) 
    {
        mFunc(spElem);
    }
protected:
    std::function<void (ElementPtr)> mFunc;
};

void addValueChangeListener (ElementPtr spElem, std::function<void (ElementPtr)> func)
{
    spElem->attachComponent(new ValueChangeListener(func));
    func(spElem);
}

//===========================================================================//

/*
    Receives the view update notification and renders the view based on what's
    current in the Document.
 */
class Renderer : public View::Component
{
public:
    ~Renderer()
    {
    }

    virtual void initialize(ViewPtr spView)
    {
        mspRasterizer.reset( new RasterizerGL );
        mspRasterizer->initialize();

        //
        // Create an empty light set that will be populated by the Document
        //
        mspLightSet = mspRasterizer->createLightSet();

        //
        // Process the data in the document being viewed
        // 
        spView->document()->iterateElements2([&](lx0::ElementPtr spElement) {
            onElementAdded(spView->document(), spElement);
        });
    }

    virtual void shutdown   (View* pView)
    {
        mspRasterizer->shutdown();
    }

    virtual void update (ViewPtr spView) 
    {
        //
        // Force a redraw at least once every 16 ms (~60 fps)
        //
        lx0::uint32 now = lx0::lx_milliseconds();
        if (now - mLastUpdate >= 16)
        {
            render();
            spView->swapBuffers();
            mLastUpdate = now;
        }
    }
    lx0::uint32 mLastUpdate;

    virtual void render (void)	
    {
        lx0::RenderAlgorithm algorithm;
        algorithm.mClearColor = glgeom::color4f(0.05f, 0.15f, 0.4f, 1.0f);
        
        lx0::GlobalPass pass;
        pass.spCamera   = mspCamera;
        pass.spLightSet = mspLightSet;
        algorithm.mPasses.push_back(pass);

        //
        // Hack at culling since LxEngine does not yet have a built-in culling
        // mechanism.
        //
        auto viewPoint = glm::inverse(mspCamera->viewMatrix) * glgeom::point3f(0, 0, 0);
        lx0::RenderList instances;
        for (auto it = mInstances.begin(); it != mInstances.end(); ++it)
        {
            bool cull = false;

            const auto& bsphere = (*it)->bsphere;
            if (false && is_finite(bsphere))
            {
                const auto& mat = (*it)->spTransform->mat;
                if (glgeom::distance(mat * bsphere.center, viewPoint) > 500.0f + 1.25f * bsphere.radius)
                    cull = true;
            }

            if (!cull)
            {
                int layer = ((*it)->spMaterial->mBlend == true) ? 1 : 0;
                instances.push_back(layer, *it);
            }
        }

        mspRasterizer->beginFrame(algorithm);
        for (auto it = instances.begin(); it != instances.end(); ++it)
        {
            mspRasterizer->rasterizeList(algorithm, it->second.list);
        }
        mspRasterizer->endFrame();
    }

    virtual void onElementAdded (DocumentPtr spDocument, ElementPtr spElem) 
    {
        if (spElem->tagName() == "Instance")
        {
            auto& primitive = spElem->value()["primitive"].unwrap<glgeom::primitive_buffer>();
            auto& transform = spElem->value()["transform"].unwrap<glgeom::mat4f>();
            auto& material = spElem->value()["material"];

            auto pInstance = new lx0::Instance;
            pInstance->spTransform = mspRasterizer->createTransform(transform);

            if (material.is_defined())
                pInstance->spMaterial = _buildMaterial(material);
            else
                pInstance->spMaterial = mMaterials.find("phong_checker")->second;

            pInstance->spGeometry = mspRasterizer->createGeometry(primitive);
            pInstance->bsphere    = glgeom::bsphere3f(primitive.bbox.center(), primitive.bbox.diagonal());
            mInstances.push_back(InstancePtr(pInstance));

            mBounds.merge(primitive.bbox);
        }
        else if (spElem->tagName() == "Player")
        {
            addValueChangeListener(spElem, [this](ElementPtr spElem) {
                auto position = spElem->value()["position"].unwrap2<glgeom::point3f>();
                auto target   = spElem->value()["target"].unwrap2<glgeom::point3f>();
                auto viewMatrix = glm::lookAt(position.vec, target.vec, glm::vec3(0, 0, 1));

                float farDistance  = std::max( glgeom::distance(position, target) * 1.5f, 1000.0f );
                float nearDistance = glm::clamp( farDistance / 1e6f, 0.05f, 10.0f );
                mspCamera = mspRasterizer->createCamera(glgeom::radians(glgeom::degrees(60)), nearDistance, farDistance, viewMatrix);
            });
        }
        else if (spElem->tagName() == "Material")
        {
            auto material = mShaderBuilder.buildShaderGLSL( spElem->value().find("graph") );
            auto spMaterial = mspRasterizer->createMaterial(material.uniqueName, material.source, material.parameters);

            std::string name = query(spElem->attr("id"), "");
            mMaterials.insert(std::make_pair(name, spMaterial));
        }
        else if (spElem->tagName() == "Light")
        {
            auto& light = spElem->value().unwrap<glgeom::point_light_f>();
            mspLightSet->mLights.push_back( mspRasterizer->createLight(light) );
        }
        else if (spElem->tagName() == "Texture")
        {
            auto& texture = spElem->value().unwrap<texture_handle>();
            auto spTexture = mspRasterizer->createTextureDDS( *texture.callback() );
            mspRasterizer->cacheTexture(texture.name, spTexture);
        }
    }

    lx0::MaterialPtr _buildMaterial (lx0::lxvar& material)
    {
        auto desc = mShaderBuilder.buildShaderGLSL(material["graph"]);
        auto spMaterial = mspRasterizer->createMaterial(desc.uniqueName, desc.source, desc.parameters);
        spMaterial->mBlend = query_path(material, "options/blend", false);
        return spMaterial;
    }

protected:
    std::shared_ptr<RasterizerGL>           mspRasterizer;
    lx0::ShaderBuilder                      mShaderBuilder;

    lx0::CameraPtr                          mspCamera;
    lx0::LightSetPtr                        mspLightSet;
    std::map<std::string, lx0::MaterialPtr> mMaterials;
    std::map<std::string, lx0::TexturePtr>  mTextures;
    std::vector<lx0::InstancePtr>           mInstances;
    glgeom::abbox3f                         mBounds;
};

lx0::View::Component*   create_renderer()       { return new Renderer; }
