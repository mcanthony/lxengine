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

#include <lx0/libs/rasterizer.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>
#include <lx0/util/blendload.hpp>
#include <lx0/prototype/misc.hpp>
#include <glgeom/prototype/camera.hpp>

using namespace lx0;
using namespace glgeom;

//===========================================================================//

class RdCamera : public Element::Component
{
public:
    RdCamera (RasterizerGL* pRasterizer)
        : mPosition (1, 1, 1)
        , mTarget   (0, 0, 0)
    {
        glgeom::radians fov(glgeom::degrees(60.0f));
        mspCamera = pRasterizer->createCamera(fov, 0.01f, 8000.0f, glm::lookAt(mPosition.vec, mTarget.vec, glm::vec3(0, 0, 1)));
    }
    
    virtual lx0::uint32 flags               (void) const { return eSkipUpdate; }

    virtual void onValueChange (ElementPtr spElem)
    {
        resetViewDirection(spElem);
    }

    void resetViewDirection (ElementPtr spElem)
    {
        mPosition = spElem->value().find("position").convert();
        mTarget = spElem->value().find("look_at").convert();

        auto view = glm::lookAt(mPosition.vec, mTarget.vec, glm::vec3(0, 0, 1));
        mspCamera->viewMatrix = view;
    }

    glgeom::point3f     mPosition;
    glgeom::point3f     mTarget;
    lx0::CameraPtr      mspCamera; 
};

typedef std::shared_ptr<RdCamera> RdCameraPtr;

static float computeHeight (const point3f& p)
{
    auto sparse0 = 4 * lx0::noise3d_perlin(p.x / 20.0f, p.y / 16.0f, .5f);
    auto sparse1 = lx0::noise3d_perlin(p.x / 60.0f, p.y / 68.0f, .25f);
    auto h = powf(sparse0, 2.0f);
    return h;
}

//===========================================================================//

class Renderable : public Element::Component
{
public:
    virtual void    generate (RasterizerGL* pRasterizer, MeshCachePtr spMeshCache, RdCameraPtr spCamera, RenderList& list) = 0;
};

typedef std::shared_ptr<Renderable> RenderablePtr;

class RdVoxelCell : public Renderable
{
public:
    RdVoxelCell (RasterizerGL* pRasterizer, MeshCachePtr spMeshCache, const vector3f& offset)
    {
        for (int z = 0; z < 4; ++z)
        {
            for (int y = 0; y < 16; ++y)
            {
                for (int x = 0; x < 16; ++x)
                {
                    const auto pt = offset + point3f(x + .5f, y + .5f, z + .5f);
                    auto h = computeHeight(pt);

                    if (pt.z > h)
                        mExists[_computeOffset(x, y, z)] = false;
                    else
                        mExists[_computeOffset(x, y, z)] = true;
                }
            }
        }
        mspInstance = _createItem(pRasterizer, offset);
    }

    virtual void onValueChange (ElementPtr spElem)
    {
    }

    virtual void generate (RasterizerGL* pRasterizer, MeshCachePtr spMeshCache, RdCameraPtr spCamera, RenderList& list)
    {
        if (mspInstance)
            list.push_back(0, mspInstance);
    }

protected:
    InstancePtr _createItem (RasterizerGL* pRasterizer, const vector3f& offset)
    {
        std::vector<point3f> position;
        std::vector<color3f> colors;

        for (int z = 0; z < 4; ++z)
        {
            for (int y = 0; y < 16; ++y)
            {
                for (int x = 0; x < 16; ++x)
                {
                    if (mExists[_computeOffset(x, y, z)])
                    {
                        glgeom::color3f color(.5f + x / 31.0f, .5f + y / 31.0f, .5f + z / 7.0f);
                        if ((x + y + z) % 2 == 1)
                            color = glgeom::color3f(.2f, .2f, .8f);
 
                        if (!_hasLocalBlock(x + 1, y, z))
                        {
                            position.push_back( point3f(x + .5f, y - .5f, z - .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x + .5f, y - .5f, z + .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x + .5f, y + .5f, z + .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x + .5f, y + .5f, z - .5f) );
                            colors.push_back(color);
                        }
                        if (!_hasLocalBlock(x - 1, y, z))
                        {
                            position.push_back( point3f(x - .5f, y - .5f, z - .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x - .5f, y + .5f, z - .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x - .5f, y + .5f, z + .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x - .5f, y - .5f, z + .5f) );
                            colors.push_back(color);
                        }

                        if (!_hasLocalBlock(x, y + 1, z))
                        {
                            position.push_back( point3f(x - .5f, y + .5f, z - .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x - .5f, y + .5f, z + .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x + .5f, y + .5f, z + .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x + .5f, y + .5f, z - .5f) );
                            colors.push_back(color);
                        }
                        if (!_hasLocalBlock(x, y - 1, z))
                        {
                            position.push_back( point3f(x - .5f, y - .5f, z - .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x + .5f, y - .5f, z - .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x + .5f, y - .5f, z + .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x - .5f, y - .5f, z + .5f) );
                            colors.push_back(color);
                        }

                        if (!_hasLocalBlock(x, y, z + 1))
                        {
                            position.push_back( point3f(x - .5f, y - .5f, z + .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x - .5f, y + .5f, z + .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x + .5f, y + .5f, z + .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x + .5f, y - .5f, z + .5f) );
                            colors.push_back(color);
                        }
                        if (!_hasLocalBlock(x, y, z - 1))
                        {
                            position.push_back( point3f(x - .5f, y - .5f, z - .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x + .5f, y - .5f, z - .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x + .5f, y + .5f, z - .5f) );
                            colors.push_back(color);
                            position.push_back( point3f(x - .5f, y + .5f, z - .5f) );
                            colors.push_back(color);
                        }
                    }
                }
            }
        }

        if (!position.empty())
        {
            InstancePtr spInstance(new Instance);
            spInstance->spMaterial = pRasterizer->createVertexColorMaterial();
            spInstance->spTransform = pRasterizer->createTransform(glgeom::vector3f(1, 1, 1), offset + glgeom::point3f(.5f, .5f, .5f));

            glgeom::primitive_buffer primitive;
            primitive.vertex.positions = position;
            primitive.vertex.colors = colors;
            spInstance->spGeometry = pRasterizer->createGeometry(primitive);
            return spInstance;
        }
        else 
            return InstancePtr();
    }


    int  _computeOffset (int x, int y, int z)
    {
        return z * 16 * 16 + y * 16 + x;
    }

    bool _hasLocalBlock (int x, int y, int z)
    {
        int offset = _computeOffset(x, y, z);
        if (offset < 0 || offset >= 16 * 16 * 4)
            return false;
        else if (x < 0 || x >= 16)
            return false;
        else if (y < 0 || y >= 16)
            return false;
        else if (z < 0 || z >= 4)
            return false;
        else
            return mExists[offset];
    }

    bool    mExists[16 * 16 * 4];
    InstancePtr mspInstance;
};

typedef std::shared_ptr<RdVoxelCell> RdVoxelCellPtr;

class RdSparseGrid : public Renderable
{
public:
    RdSparseGrid ()
    {
    }

    virtual void generate (RasterizerGL* pRasterizer, MeshCachePtr spMeshCache, RdCameraPtr spCamera, RenderList& list)
    {
        const int radius = 5;
        const int x0 = int(spCamera->mPosition.x/16) - radius;
        const int x1 = int(spCamera->mPosition.x/16) + radius;
        const int y0 = int(spCamera->mPosition.y/16) - radius;
        const int y1 = int(spCamera->mPosition.y/16) + radius;
        const int z0 = int(spCamera->mPosition.z/4) - radius * 4;
        const int z1 = int(spCamera->mPosition.z/4) + radius * 4;

        for (int z = z0; z <= z1; ++z)
        {
            for (int y = y0; y <= y1; ++y)
            {
                for (int x = x0; x <= x1; ++x)
                {
                    Index idx(x, y, z);

                    RenderablePtr spRenderable;

                    auto it = mGrid.find(idx);
                    if (it == mGrid.end())
                    {
                        vector3f offset(x * 16, y * 16, z * 4);
                        auto pCell = new RdVoxelCell(pRasterizer, spMeshCache, offset);

                        spRenderable = RenderablePtr(pCell);
                        mGrid.insert(std::make_pair(idx, spRenderable));
                    }
                    else
                        spRenderable = it->second;

                    spRenderable->generate(pRasterizer, spMeshCache, spCamera, list);
                }
            }
        }
    }

protected:
    struct Index
    {
        Index(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
        bool operator < (const Index& that) const
        {
            if (x < that.x) 
                return true;
            else if (x > that.x)
                return false;
            else if (y < that.y) 
                return true;
            else if (y > that.y)
                return false;
            else if (z < that.z) 
                return true;
            else
                return false;
        }
        int x, y, z;
    };

    std::map<Index, RenderablePtr> mGrid;
};


//===========================================================================//

class Renderer : public View::Component
{
public:
    ~Renderer()
    {
        lx_log("Renderer dtor");
    }

    virtual void initialize(ViewPtr spView)
    {
        mspRasterizer.reset( new RasterizerGL );
        mspRasterizer->initialize();

        mspMeshCache.reset( new MeshCache(mspRasterizer) );

        mspCamera.reset(new RdCamera(mspRasterizer.get()));
        mspLightSet = mspRasterizer->createLightSet();

        spView->document()->iterateElements([&](ElementPtr spElem) -> bool { 
            _onElementAddRemove(spElem, true); 
            return false; 
        });
    }

    virtual void shutdown   (View* pView)
    {
        mspRasterizer->shutdown();
    }

    virtual void render (void)	
    {
        RenderAlgorithm algorithm;
        algorithm.mClearColor = glgeom::color4f(0.1f, 0.1f, 0.0f, 1.0f);
        GlobalPass pass[4];
        pass[0].tbFlatShading = true;
        pass[0].spCamera = mspCamera->mspCamera;
        pass[0].spLightSet = mspLightSet;
        algorithm.mPasses.push_back(pass[0]);

        RenderList instances;
        for (auto it = mRenderables.begin(); it != mRenderables.end(); ++it)
            (*it)->generate(mspRasterizer.get(), mspMeshCache, mspCamera, instances);

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



protected:
    void _onElementAddRemove (ElementPtr spElem, bool bAdd)
    {
        using namespace glgeom;

        const std::string tag = spElem->tagName();

        /*if (tag == "VoxelCell")
        {
            mRenderables.push_back(RenderablePtr(new RdVoxelCell(mspRasterizer.get(), mspMeshCache)));
        }*/
        if (tag == "SparseGrid")
        {
            mRenderables.push_back(RenderablePtr(new RdSparseGrid));
        }
        else if (tag == "Camera")
        {
            mspCamera->resetViewDirection(spElem);
            spElem->attachComponent(mspCamera);
        }
        else 
        { 
        }
    }

    std::shared_ptr<RasterizerGL>       mspRasterizer;
    MeshCachePtr                        mspMeshCache;

    RdCameraPtr                         mspCamera;
    lx0::LightSetPtr                    mspLightSet;
    std::vector<RenderablePtr>          mRenderables;  
};



lx0::View::Component*   create_renderer()       { return new Renderer; }
