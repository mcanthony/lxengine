//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

#pragma once

#include <lx0/lxengine.hpp>
#include <lx0/prototype/misc.hpp>
#include "rasterizer_ext.hpp"


extern lx0::Camera2             gCamera;

using namespace lx0::subsystem::rasterizer_ns;

class Renderable : public lx0::Element::Component
{
public:
    virtual void update(lx0::ElementPtr spElement) {}

    virtual void generate(lx0::ElementPtr spElement,
                  RasterizerGL& rasterizer,
                  lx0::Camera2& cam1,
                  CameraPtr spCamera, 
                  LightSetPtr spLightSet, 
                  RenderList& list) = 0;
};


/*!
 */
class Renderer 
    : public lx0::View::Component
{
public:
    Renderer (lx0::DocumentPtr spDoc)
        : mViewMode (0)
        , mspDocument (spDoc)
    {
    }

    virtual void initialize (lx0::ViewPtr spView);

    void 
    resize (int width, int height)
    {

    }

    void
    update (void)
    {
        std::vector<lx0::ElementPtr> elems;
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
    _generateItems (RenderList& instances)
    {
        std::vector<lx0::ElementPtr> elems;
        elems.swap( mspDocument->getElements() );

        size_t count = 0;
        for (auto it = elems.begin(); it != elems.end(); ++it)
        {
            auto spRenderable = (*it)->getComponent<Renderable>("renderable");
            if (spRenderable)
            {
                spRenderable->generate(*it, mRasterizer, gCamera, spCamera, spLightSet, instances);
            }
            count ++;
        }
    }

    void
    cycleViewMode (void)
    {
        mViewMode = (mViewMode + 1) % 3;
    }

    void 
    _generateRenderAlgorithm (lx0::RenderAlgorithm& algorithm);

    void 
    _generateSelectAlgorithm (lx0::RenderAlgorithm& algorithm)
    {
        algorithm.mClearColor = glgeom::color4f(0.0f, 0.0f, 0.0f, 0.0f);

        // The solid.frag is not sufficient since the alpha mask needs to be read from RGBA textures
        // to ensure only the right pixels are actually written to the pixel buffer.
        //
        GlobalPass pass[4];
        pass[0].spMaterial = mRasterizer.createMaterial(
            "SelectMaterial",
            lx0::string_from_file("common/shaders/glsl/fragment/solid.frag"),
            lx0::lxvar::map());
        algorithm.mPasses.push_back(pass[0]); 
    }

    void
    _renderImp (lx0::RenderAlgorithm& algorithm, RenderList& instances)
    {
        spCamera->viewMatrix = view_matrix(gCamera);

        mRasterizer.beginFrame(algorithm);

        _generateItems(instances);
        mRasterizer.rasterizeList(algorithm, instances);

        mRasterizer.endFrame();
    }

    void 
    render (void)	
    {
        RenderAlgorithm algorithm;
        RenderList instances;

        _generateRenderAlgorithm(algorithm);
        _renderImp(algorithm, instances);

        mRasterizer.refreshTextures();
    }

    lx0::InstancePtr
    select (int x, int y)
    {
        RenderAlgorithm algorithm;
        _generateSelectAlgorithm(algorithm);

        // Draw to the backbuffer
        RenderList instances;
        _renderImp(algorithm, instances);
        unsigned int id = mRasterizer.readPixel(x, y);
        return instances.getInstance(id);
    }

    virtual void handleEvent (std::string evt, lx0::lxvar params);

    lx0::DocumentPtr                 mspDocument;

protected:
    lx0::CameraPtr      spCamera;       // Camera shared by all instances
    lx0::LightSetPtr    spLightSet;
    lx0::RasterizerGL   mRasterizer;
    int                 mViewMode;
};
