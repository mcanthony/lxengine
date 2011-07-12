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

#include <iostream>
#include <lx0/subsystem/rasterizer.hpp>
#include <glgeom/ext/primitive_buffer.hpp>
#include "lxextensions/lxvar_wrap.hpp"

using namespace lx0;
using namespace glgeom;

//===========================================================================//

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
        // Process the data in the document being viewed
        // 
        spView->document()->iterateElements2([&](lx0::ElementPtr spElement) {
            onElementAdded(spView->document(), spElement);
        });

        //
        // Create a set of lights
        // 
        lx0::LightPtr spLight0 = mspRasterizer->createLight();
        spLight0->position = glgeom::point3f(10, -10, 10);
        spLight0->color    = glgeom::color3f(1, 1, 1);
        
        lx0::LightPtr spLight1 = mspRasterizer->createLight();
        spLight0->position = glgeom::point3f(10, 10, 10);
        spLight0->color    = glgeom::color3f(.6f, .6f, 1);
        
        mspLightSet = mspRasterizer->createLightSet();
        mspLightSet->mLights.push_back(spLight0);
        mspLightSet->mLights.push_back(spLight1);
    }

    virtual void shutdown   (View* pView)
    {
        mspRasterizer->shutdown();
    }

    virtual void render (void)	
    {
        lx0::RenderAlgorithm algorithm;
        algorithm.mClearColor = glgeom::color4f(0.1f, 0.3f, 0.8f, 1.0f);
        
        lx0::GlobalPass pass;
        pass.spCamera   = mspCamera;
        pass.spLightSet = mspLightSet;
        algorithm.mPasses.push_back(pass);

        lx0::RenderList instances;
        for (auto it = mInstances.begin(); it != mInstances.end(); ++it)
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
        if (spElem->tagName() == "Instance")
        {
            auto& primitive = lxvar_unwrap<glgeom::primitive_buffer>(spElem->value()["primitive"]);
            auto& transform = lxvar_unwrap<glgeom::mat4f>(spElem->value()["transform"]);

            auto pInstance = new lx0::Instance;
            pInstance->spTransform = mspRasterizer->createTransform(transform);
            pInstance->spMaterial = mspRasterizer->createMaterial("media2/shaders/glsl/fragment/diffuse_gray.frag");
            pInstance->spGeometry = mspRasterizer->createGeometry(primitive);
            mInstances.push_back(InstancePtr(pInstance));

            mBounds.merge(primitive.bbox);
        }
        else if (spElem->tagName() == "Player")
        {
            addValueChangeListener(spElem, [this](ElementPtr spElem) {
                auto position = lxvar_unwrap<glgeom::point3f>(spElem->value()["position"]);
                auto target   = lxvar_unwrap<glgeom::point3f>(spElem->value()["target"]);
                auto viewMatrix = glm::lookAt(position.vec, target.vec, glm::vec3(0, 0, 1));
                mspCamera = mspRasterizer->createCamera(glgeom::radians(glgeom::degrees(60)), 0.1f, 10000.0f, viewMatrix);
                spElem->document()->view(0)->sendEvent("redraw");
            });
        }
    }

protected:
    std::shared_ptr<RasterizerGL>   mspRasterizer;
    lx0::CameraPtr                  mspCamera;
    lx0::LightSetPtr                mspLightSet;
    std::vector<lx0::InstancePtr>   mInstances;
    glgeom::abbox3f                 mBounds;
};

lx0::View::Component*   create_renderer()       { return new Renderer; }
