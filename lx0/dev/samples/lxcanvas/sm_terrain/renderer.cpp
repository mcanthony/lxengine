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
        mRotation += glgeom::two_pi() / 30.0f;
    }

    virtual void generate(ElementPtr spElement,
                      RasterizerGL& rasterizer,
                      lx0::Camera2& cam1,
                      CameraPtr spCamera, 
                      LightSetPtr spLightSet, 
                      RenderList& list)
    {
        if (!mspItem)
        {
            lx0::BlendReader reader;
            reader.open("media2/models/unit_hemisphere-000.blend");
            
            auto meshBlocks = reader.getBlocksByType("Mesh");
            for (auto it = meshBlocks.begin(); it != meshBlocks.end(); ++it)
            {
                auto spBlock = *it;
                auto spMesh = reader.readObject( spBlock->address );
                const auto totalVertices = spMesh->field<int>("totvert");
                const auto totalFaces = spMesh->field<int>("totface");

                std::vector<glgeom::point3f>  positions;
                std::vector<glgeom::vector3f> normals;
                std::vector<glgeom::color3f>  colors;
                std::vector<unsigned short> indicies;

                positions.reserve(totalVertices);
                normals.reserve(totalVertices);
                colors.reserve(totalVertices);
                indicies.reserve(totalFaces * 4);

                auto spVerts = reader.readObject( spMesh->field<unsigned __int64>("mvert") );
                for (int i = 0; i < totalVertices; ++i)
                {
                    glgeom::point3f p;
                    p.x = spVerts->field<float>("co", 0);
                    p.y = spVerts->field<float>("co", 1);
                    p.z = spVerts->field<float>("co", 2);
                    p.vec *= 200;
                    positions.push_back(p);

                    glgeom::vector3f n;
                    n.x = spVerts->field<short>("no", 0) / float(std::numeric_limits<short>::max());
                    n.y = spVerts->field<short>("no", 1) / float(std::numeric_limits<short>::max());
                    n.z = spVerts->field<short>("no", 2) / float(std::numeric_limits<short>::max());
                    normals.push_back(n);

                    colors.push_back( glgeom::color3f(1, 1, 1) );

                    spVerts->next();
                }

                auto spFaces = reader.readObject( spMesh->field<unsigned __int64>("mface") );
                for (int i = 0; i < totalFaces; ++i)
                {
                    int vi[4];
                    vi[0] = spFaces->field<int>("v1");
                    vi[1] = spFaces->field<int>("v2");
                    vi[2] = spFaces->field<int>("v3");
                    vi[3] = spFaces->field<int>("v4");

                    // Convert tris into degenerate quads
                    if (vi[3] == 0)
                        vi[3] = vi[2];

                    for (int j = 0; j < 4; ++j)
                        indicies.push_back(vi[j]);

                    spFaces->next();
                }
                 
                glgeom::point3f pos( 0.0f, 0.0f, 0.0f);
                mPosition = pos;

                auto spGeom = rasterizer.createQuadList(indicies, positions, normals, colors);

                auto spMat = rasterizer.createMaterial("media2/shaders/glsl/fragment/skymap.frag");
                spMat->mBlend = false;
                spMat->mWireframe = false;
                spMat->mZTest = false;
                spMat->mZWrite = false;
                spMat->mTextures[0] = rasterizer.createTexture("media2/textures/skymaps/polar/bluesky_grayclouds.png");

                auto pItem = new Item;
                pItem->spCamera   = spCamera;
                pItem->spLightSet = spLightSet;
                pItem->spMaterial = spMat;
                pItem->spTransform = rasterizer.createTransformEye(pos.x, pos.y, pos.z, glgeom::radians(0.0f));
                pItem->spGeometry = spGeom;
            
                mspItem.reset(pItem);
            }
        }

        mspItem->spTransform = rasterizer.createTransformEye(mPosition.x, mPosition.y, mPosition.z, mRotation);
        list.push_back(0, mspItem);
    }

protected:
    glgeom::point3f   mPosition;
    glgeom::radians   mRotation;
    ItemPtr           mspItem;
};

//===========================================================================//

class SpriteShared
{
public:
    MaterialPtr _ensureMaterial (RasterizerGL& rasterizer, std::string image)
    {
        if (!mspMaterial)
        {
            auto spMat = rasterizer.createMaterial("media2/shaders/glsl/fragment/texture1_fog.frag");
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

            mspGeom = rasterizer.createQuadList(indicies, positions, normals, colors);
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
        if (!mspItem)
        {
            std::string image = spElement->attr("image").asString();

            auto pItem = new Item;
            pItem->setData<ElementPtr>(spElement);
            pItem->spCamera   = spCamera;
            pItem->spLightSet = spLightSet;
            pItem->spMaterial = SpriteShared::acquire()->_ensureMaterial(rasterizer, image);
            pItem->spGeometry = SpriteShared::acquire()->_ensureGeom(rasterizer);
            
            mspItem.reset(pItem);
        }

        lxvar px = spElement->attr("position");
        glgeom::point3f pos(px.at(0).asFloat(), px.at(1).asFloat(), px.size() == 3 ? px.at(2).asFloat() : 0.0f); 
        float scale = spElement->attr("scale").query(1.0f);
        mspItem->spTransform = rasterizer.createTransformBillboardXYS(pos.x, pos.y, pos.z, scale, scale, scale);

        list.push_back(1, mspItem);
    }

protected:
    ItemPtr           mspItem;
};


void 
Renderer::handleEvent (std::string evt, lx0::lxvar params)
{
    if (evt == "select_object")
    {
        auto& spItem = select( params.at(0).asInt(), params.at(1).asInt() );
        auto spElement = spItem->getData<ElementPtr>();
        std::string name = spElement
            ? spElement->attr("image").query("unknown").c_str()
            : "no associated element";
        printf("Select: %s (%s)\n", spItem->spMaterial->mShaderFilename.c_str(), name.c_str());
    }
    else if (evt == "cycle_viewmode")
        cycleViewMode();
}

Element::Component* new_Sprite() { return new Sprite; }
Element::Component* new_SkyMap() { return new SkyMap; }

