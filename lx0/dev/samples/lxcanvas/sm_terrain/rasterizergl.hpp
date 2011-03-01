//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010 athile@athile.net (http://www.athile.net)

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

#include <gl/glew.h>
#include <windows.h>        // Unfortunately must be included on Windows for GL.h to work
#include <gl/GL.h>

#include <lx0/core/core.hpp>

using namespace lx0::core;

namespace Rasterizer
{
    struct Geometry
    {
        virtual void activate() = 0;
    };
    typedef std::shared_ptr<Geometry> GeometryPtr;

    class GeomImp : public Geometry
    {
    public:
        GeomImp() : mType(0), mVboIndices (0),  mVao(0), mVboPosition(0), mVboNormal(0), mCount(0), mVboColors(0) {}

        virtual void activate();

        GLenum  mType;
        GLuint  mVao;
        GLsizei mCount;
        GLuint  mVboPosition;
        GLuint  mVboNormal;
        GLuint  mVboColors;
        GLuint  mVboIndices;
    };

    class Resource
    {
    public:
        virtual ~Resource() {}

        virtual void load () = 0;
        virtual void unload () = 0;
    };
    typedef std::shared_ptr<Resource> ResourcePtr;

    class Texture : public Resource
    {
    public:

        std::string mFilename;

        std::time_t mFileTimestamp;

        
        GLuint      mId;

        Texture();

        virtual void load ();
        virtual void unload ();
    };
    typedef std::shared_ptr<Texture> TexturePtr;
}

class RasterizerGL
{
public:
    typedef Rasterizer::Geometry    Geometry;
    typedef Rasterizer::GeometryPtr GeometryPtr;
    typedef Rasterizer::ResourcePtr ResourcePtr;
    typedef Rasterizer::TexturePtr  TexturePtr;

    struct Camera
    {
        virtual void activate();
        float   fov;
        float   nearDist;
        float   farDist;
        matrix4 viewMatrix;
    };
    typedef std::shared_ptr<Camera> CameraPtr;

    struct Material
    {
                     Material   (GLuint id);

        virtual void activate   (RasterizerGL*);

        bool        mBlend;
        int         mFilter;
        TexturePtr  mTextures[8];
    
    protected:
        // Note: the program should be separate from the parameters passed to
        // it.
        GLuint      mId;
    };
    typedef std::shared_ptr<Material> MaterialPtr;
    typedef std::weak_ptr<Material> MaterialWPtr;

    struct QuadList : public Geometry
    {
        virtual void activate();

        size_t size;
        GLuint vbo[1];
        GLuint vao[1];
    };

    struct LightSet
    {
        virtual void activate() {}
    };
    typedef std::shared_ptr<LightSet> LightSetPtr;

    struct Transform
    {
        virtual void activate(CameraPtr);
        matrix4 mat;
    };
    typedef std::shared_ptr<Transform> TransformPtr;

    struct BillboardTransform : public Transform
    {
        virtual void activate(CameraPtr);
    };

    class Item
    {
    public:
        virtual void rasterize(RasterizerGL*);

        //weak_ptr<Target> wpTarget;
        CameraPtr    spCamera;
        TransformPtr spTransform;
        MaterialPtr  spMaterial;
        GeometryPtr  spGeometry;
        LightSetPtr  spLightSet;
    };
    typedef std::shared_ptr<Item> ItemPtr;

    void            initialize      (void);
    void            shutdown        (void);

    CameraPtr       createCamera    (float fov, float nearDist, float farDist, matrix4& viewMatrix);
    LightSetPtr     createLightSet  (void);
    MaterialPtr     createMaterial  (std::string fragShader);
    TexturePtr      createTexture   (const char* filename);

    TransformPtr    createTransform (matrix4& mat);
    TransformPtr    createTransform (float tx, float ty, float tz);
    TransformPtr    createTransformBillboard(float tx, float ty, float tz);

    GeometryPtr     createQuadList  (std::vector<point3>& quads);
    GeometryPtr     createQuadList  (std::vector<unsigned short>& indices, 
                                     std::vector<point3>& positions, 
                                     std::vector<vector3>& normals,
                                     std::vector<vector3>& colors);

    void            refreshTextures (void);

    void            beginScene      (void);
    void            endScene        (void);

    void            rasterizeList   (std::vector<std::shared_ptr<Item>>& list);
    void            rasterize       (std::shared_ptr<Item> spItem);

protected:
    GLuint      _createProgram   (std::string fragShader);
    GLuint      _createProgram2  (std::string fragShader);
    GLuint      _createShader    (const char* filename, GLuint type);
    void        _linkProgram     (GLuint prog);

    std::map<std::string, GLuint> mCachePrograms;

    std::list<ResourcePtr>      mResources;
    std::vector<TexturePtr>     mTextures;
    
};
