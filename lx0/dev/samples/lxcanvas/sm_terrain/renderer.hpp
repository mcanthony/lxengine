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

#include <lx0/prototype/misc.hpp>
#include "rasterizer_ext.hpp"


extern lx0::prototype::Camera             gCamera;

using namespace lx0::subsystem::rasterizer;

class Renderable : public lx0::Element::Component
{
public:
    virtual void update(lx0::ElementPtr spElement) {}

    virtual void generate(lx0::ElementPtr spElement,
                  RasterizerGL& rasterizer,
                  lx0::prototype::Camera& cam1,
                  CameraPtr spCamera, 
                  LightSetPtr spLightSet, 
                  RenderList& list) = 0;
};


class Renderer : public lx0::Document::Component
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
    _generateItems (RenderList& items)
    {
        std::vector<lx0::ElementPtr> elems;
        elems.swap( mspDocument->getElements() );

        size_t count = 0;
        for (auto it = elems.begin(); it != elems.end(); ++it)
        {
            auto spRenderable = (*it)->getComponent<Renderable>("renderable");
            if (spRenderable)
            {
                spRenderable->generate(*it, mRasterizer, gCamera, spCamera, spLightSet, items);
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
    _generateRenderAlgorithm (lx0::RenderAlgorithm& algorithm)
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
    _generateSelectAlgorithm (lx0::RenderAlgorithm& algorithm)
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
    _renderImp (lx0::RenderAlgorithm& algorithm, RenderList& items)
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

    lx0::ItemPtr
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

    lx0::DocumentPtr                 mspDocument;

protected:
    lx0::CameraPtr       spCamera;       // Camera shared by all items
    lx0::LightSetPtr     spLightSet;
    lx0::RasterizerGL    mRasterizer;
    int             mViewMode;
};
