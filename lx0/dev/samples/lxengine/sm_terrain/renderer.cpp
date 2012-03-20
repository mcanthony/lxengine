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

#include <lx0/lxengine.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>
#include "renderer.hpp"
#include <lx0/subsystem/blendreader.hpp>
#include <lx0/util/blendload.hpp>
#include "main.hpp"
#include "physics.hpp"

using namespace lx0;

lx0::Camera2             gCamera;

//===========================================================================//

class SkyMap : public Renderable
{
public:
    virtual void update(ElementPtr spElement)
    {
        mRotation += glgeom::two_pi() / 1000.0f;
    }

    virtual void generate(ElementPtr spElement,
                      RasterizerGL& rasterizer,
                      lx0::Camera2& cam1,
                      CameraPtr spCamera, 
                      LightSetPtr spLightSet, 
                      RenderList& list)
    {
        if (!mspInstance)
        {               
            mPosition = glgeom::point3f ( 0.0f, 0.0f, 0.0f);

            auto spGeom = lx0::quadlist_from_blendfile(rasterizer, "media2/models/unit_hemisphere-000.blend", 200.0f);

            auto spMat = rasterizer.createMaterial("SkyMap", lx0::string_from_file("media2/shaders/glsl/fragment/skymap.frag"), lxvar::map());
            spMat->mBlend = false;
            spMat->mWireframe = false;
            spMat->mZTest = false;
            spMat->mZWrite = false;
            spMat->mTextures[0] = rasterizer.createTexture("media2/textures/skymaps/polar/bluesky_grayclouds.png");

            auto pInstance = new Instance;
            pInstance->spCamera   = spCamera;
            pInstance->spLightSet = spLightSet;
            pInstance->spMaterial = spMat;
            pInstance->spTransform = rasterizer.createTransformEye(mPosition.x, mPosition.y, mPosition.z, glgeom::radians(0.0f));
            pInstance->spGeometry = spGeom;
            
            mspInstance.reset(pInstance);
        }

        mspInstance->spTransform = rasterizer.createTransformEye(mPosition.x, mPosition.y, mPosition.z, mRotation);
        list.push_back(0, mspInstance);
    }

protected:
    glgeom::point3f   mPosition;
    glgeom::radians   mRotation;
    InstancePtr           mspInstance;
};

//===========================================================================//

class SpriteShared
{
public:
    MaterialPtr _ensureMaterial (RasterizerGL& rasterizer, std::string image)
    {
        if (!mspMaterial)
        {
            auto spMat = rasterizer.createMaterial("SpriteShared", lx0::string_from_file("media2/shaders/glsl/fragment/texture1_fog.frag"), lxvar::map());
            spMat->mBlend = true;
            spMat->mFilter = GL_NEAREST;
            spMat->mTextures[0] = rasterizer.createTexture(image.c_str());
            mspMaterial = spMat;
        }
        return mspMaterial;
    }

    GeometryPtr  _ensureGeom (RasterizerGL& rasterizer)
    {
        if (!mspGeom)
        {
            const float kSize = 1.0f;

            std::vector<glgeom::point3f> positions;
            positions.push_back( glgeom::point3f(0, 0, 0) );
            positions.push_back( glgeom::point3f(kSize, 0, 0) );
            positions.push_back( glgeom::point3f(kSize, 0, kSize) );
            positions.push_back( glgeom::point3f(0, 0, kSize) );

            std::vector<glgeom::vector3f> normals;
            normals.push_back( glgeom::vector3f(0, 1, 0) );
            normals.push_back( glgeom::vector3f(0, 1, 0) );
            normals.push_back( glgeom::vector3f(0, 1, 0) );
            normals.push_back( glgeom::vector3f(0, 1, 0) );

            std::vector<glgeom::color3f> colors;
            colors.push_back( glgeom::color3f(0, 1, 0) );
            colors.push_back( glgeom::color3f(1, 1, 0) );
            colors.push_back( glgeom::color3f(1, 0, 0) );
            colors.push_back( glgeom::color3f(0, 0, 0) );

            std::vector<unsigned short> indicies;
            indicies.push_back(0);
            indicies.push_back(1);
            indicies.push_back(2);
            indicies.push_back(3);

            glgeom::primitive_buffer primitive;
            primitive.type = "quadlist";
            primitive.vertex.positions.swap(positions);
            primitive.vertex.normals.swap(normals);
            primitive.vertex.colors.swap(colors);
            primitive.indices.swap(indicies);

            mspGeom = rasterizer.createGeometry(primitive);
        }
        return mspGeom;
    }

    static std::shared_ptr<SpriteShared> acquire()
    {
        static std::weak_ptr<SpriteShared> s_wpSingleton;
        auto spSingleton = s_wpSingleton.lock();
        if (!spSingleton)
        {
            spSingleton.reset(new SpriteShared);
            s_wpSingleton = spSingleton;
        }
        return spSingleton;
    }

protected:
    MaterialPtr       mspMaterial;
    GeometryPtr       mspGeom;
};

class Sprite : public Renderable
{
public:
    virtual void generate(ElementPtr spElement,
                      RasterizerGL& rasterizer,
                      lx0::Camera2& cam1,
                      CameraPtr spCamera, 
                      LightSetPtr spLightSet, 
                      RenderList& list)
    {
        if (!mspInstance)
        {
            std::string image = spElement->attr("image").as<std::string>();

            auto pInstance = new Instance;
            pInstance->setData<ElementPtr>(spElement);
            pInstance->spCamera   = spCamera;
            pInstance->spLightSet = spLightSet;
            pInstance->spMaterial = SpriteShared::acquire()->_ensureMaterial(rasterizer, image);
            pInstance->spGeometry = SpriteShared::acquire()->_ensureGeom(rasterizer);
            
            mspInstance.reset(pInstance);
        }

        lxvar px = spElement->attr("position");
        glgeom::point3f pos(px.at(0).as<float>(), px.at(1).as<float>(), px.size() == 3 ? px.at(2).as<float>() : 0.0f); 
        float scale = query( spElement->attr("scale"), 1.0f);
        mspInstance->spTransform = rasterizer.createTransformBillboardXYS(pos.x, pos.y, pos.z, scale, scale, scale);

        list.push_back(1, mspInstance);
    }

protected:
    InstancePtr           mspInstance;
};

//===========================================================================//

void 
Renderer::initialize (lx0::ViewPtr spView)
{
    gCamera.mPosition = glgeom::point3f(20, 20, 2);
    gCamera.mTarget = glgeom::point3f(0, 0, 0);
    gCamera.mWorldUp = glgeom::vector3f(0, 0, 1);
    gCamera.mFov = 60.0f;
    gCamera.mNear = 0.01f;  // 1 cm
    gCamera.mFar = 2000.0f; // 2 km

    gCamera.mPosition.z = 0.0f;
    //gCamera.mTarget.z = gCamera.mPosition.z;

    mRasterizer.initialize();

    glgeom::radians fov( glgeom::degrees(gCamera.mFov) );
    spCamera = mRasterizer.createCamera(fov, gCamera.mNear, gCamera.mFar, view_matrix(gCamera));
    spLightSet = mRasterizer.createLightSet();
} 

void 
Renderer::handleEvent (std::string evt, lx0::lxvar params)
{
    if (evt == "select_object")
    {
        auto& spInstance = select( params.at(0).as<int>(), params.at(1).as<int>() );
        auto spElement = spInstance->getData<ElementPtr>();
        std::string name = spElement
            ? query(spElement->attr("image"), "unknown").c_str()
            : "no associated element";
        //printf("Select: %s (%s)\n", spInstance->spMaterial->mShaderFilename.c_str(), name.c_str());
    }
    else if (evt == "cycle_viewmode")
        cycleViewMode();
}

void 
Renderer::_generateRenderAlgorithm (lx0::RenderAlgorithm& algorithm)
{
    algorithm.mClearColor = glgeom::color4f(0.09f, 0.09f, 0.11f, 1.0f);

    GlobalPass pass[4];
    switch (mViewMode)
    {
    default:
        throw lx_error_exception("Invalid view mode %d", mViewMode);
    case 0:
        // Use a single pass with all the default settings
        algorithm.mPasses.push_back(pass[0]);
        break;
    case 1:
        pass[0].tbWireframe = true;
        algorithm.mPasses.push_back(pass[0]);
        break;
    case 2:
        pass[0].spMaterial = mRasterizer.createMaterial(
            "RenderMaterial",
            lx0::string_from_file("media2/shaders/glsl/fragment/solid.frag"),
            lx0::lxvar::map());
        algorithm.mPasses.push_back(pass[0]);
        break;
    }  
}


Element::Component* new_Sprite() { return new Sprite; }
Element::Component* new_SkyMap() { return new SkyMap; }

