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

#pragma once

#include <boost/logic/tribool.hpp>
#include <lx0/engine/document.hpp>

#include <glgeom/glgeom.hpp>
#include <glgeom/prototype/material_phong.hpp>

namespace lx0 { 

    class IRasterizer : public lx0::Document::Component
    {
    public:
    };

    IRasterizer* createIRasterizer();
}


#include <list>
#include <ctime>

#include <gl/glew.h>
#include <windows.h>        // Unfortunately must be included on Windows for GL.h to work
#include <gl/GL.h>

#include <lx0/lxengine.hpp>
#include <glgeom/glgeom.hpp>

using namespace lx0::core;

namespace lx0 
{
    namespace subsystem
    {
        /*!
            \defgroup lx0_subsystem_rasterizer lx0_subsystem_rasterizer
            \ingroup Subsystem
         */
        namespace rasterizer
        {
            class GlobalPass;
            class RasterizerGL;
            class Camera;
            typedef std::shared_ptr<Camera> CameraPtr;
            class LightSet;
            typedef std::shared_ptr<LightSet> LightSetPtr;

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            struct Geometry
            {
                virtual void activate(RasterizerGL*, GlobalPass& pass) = 0;
            };
            typedef std::shared_ptr<Geometry> GeometryPtr;

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class GeomImp : public Geometry
            {
            public:
                GeomImp() 
                    : mType(0)
                    , mVboIndices (0)
                    , mVao(0)
                    , mVboPosition(0)
                    , mVboNormal(0)
                    , mCount(0)
                    , mVboColors(0)
                    , mTexFlags (0) 
                    , mFaceCount (0)
                {}

                virtual void activate(RasterizerGL*, GlobalPass& pass);

                GLenum  mType;
                GLuint  mVao;
                GLsizei mCount;
                
                GLuint  mVboPosition;
                GLuint  mVboNormal;
                GLuint  mVboColors;

                GLuint  mVboIndices;
                GLuint  mTexFlags;
                GLuint  mFaceCount;
            };

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class Resource
            {
            public:
                virtual ~Resource() {}

                virtual void load () = 0;
                virtual void unload () = 0;
            };
            typedef std::shared_ptr<Resource> ResourcePtr;

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
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

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class Material
            {
            public:
                             Material   (GLuint id);

                virtual void activate   (RasterizerGL*, GlobalPass& pass);

                std::string mShaderFilename;

                bool        mBlend;
                bool        mZTest;
                bool        mZWrite;
                bool        mWireframe;
                int         mFilter;
                TexturePtr  mTextures[8];
    
            protected:
                // Note: the program should be separate from the parameters passed to
                // it.
                GLuint      mId;
            };
            typedef std::shared_ptr<Material> MaterialPtr;
            typedef std::weak_ptr<Material> MaterialWPtr;

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class SolidColorMaterial : public Material
            {
            public:
                        SolidColorMaterial(GLuint id);

                virtual void activate   (RasterizerGL*, GlobalPass& pass);

                glgeom::color3f    mColor;
            };

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class PhongMaterial : public Material
            {
            public:
                        PhongMaterial(GLuint id);

                virtual void activate   (RasterizerGL*, GlobalPass& pass);

                glgeom::material_phong_f    mPhong;
            };

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class GlobalPass
            {
            public:
                     GlobalPass()
                         : bOverrideMaterial    (false)
                         , tbWireframe          (boost::indeterminate)
                         , tbFlatShading        (boost::indeterminate)
                     { }

                CameraPtr       spCamera;
                LightSetPtr     spLightSet;

                bool            bOverrideMaterial;
                MaterialPtr     spMaterial;

                boost::tribool  tbWireframe;
                boost::tribool  tbFlatShading;
            };

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class RenderAlgorithm
            {
            public:
                glgeom::color4f         mClearColor;
                std::vector<GlobalPass> mPasses;
            };


            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class Camera
            {
            public:
                virtual void activate();
                float   fov;
                float   nearDist;
                float   farDist;
                glm::mat4 viewMatrix;
            };
            
            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            struct Transform
            {
                virtual void activate(CameraPtr);
                glm::mat4 mat;
            };
            typedef std::shared_ptr<Transform> TransformPtr;

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer

            class Light
            {
            public:
                glgeom::point3f position;
                glgeom::color3f color;
            };
            typedef std::shared_ptr<Light> LightPtr;

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class LightSet
            {
            public:
                virtual void activate() {}

                std::vector<LightPtr> mLights;
            };
            typedef std::shared_ptr<LightSet> LightSetPtr;

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class Item
            {
            public:
                class Data
                {
                public:
                    virtual ~Data() {}
                };

                template <typename T>
                class DataT : public Data
                {
                public:
                    DataT(const T& t) : data(t) {}
                    T data;
                };


                Item() {}
        
                //virtual void rasterize(RasterizerGL*);

                //weak_ptr<Target> wpTarget;  - probably should be a member of the RenderList layer or RenderAlgorithm?
                CameraPtr    spCamera;
                TransformPtr spTransform;
                MaterialPtr  spMaterial;
                GeometryPtr  spGeometry;
                LightSetPtr  spLightSet;

                template <typename T>
                void setData (const T& data)
                {
                    mpData.reset( new DataT<T>(data) );
                }

                template <typename T>
                T getData () 
                {  
                    DataT<T>* pData = dynamic_cast<DataT<T>*>(mpData.get());
                    if (pData)
                        return pData->data;
                    else
                        return T();
                }

            protected:
                std::auto_ptr<Data> mpData;
            };
            typedef std::shared_ptr<Item> ItemPtr;

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class QuadList : public Geometry
            {
            public:
                virtual void activate(RasterizerGL*, GlobalPass& pass);

                size_t size;
                GLuint vbo[1];
                GLuint vao[1];
            };
            typedef std::shared_ptr<QuadList> QuadListPtr;

            //===========================================================================//
            /*!
                \ingroup lx0_subsystem_rasterizer
                
                Represents all the items to render for a particular frame.  The items are organized
                into a set of layers, each with an ordered list of items.  Each layer has its own
                set of settings which may control the optimization, re-ordering, etc. of the list
                for that layer.
             */
            class RenderList
            {
            public:
                typedef lx0::subsystem::rasterizer::ItemPtr ItemPtr;
                typedef std::vector<ItemPtr>                ItemList;

                struct Layer
                {
                    void*       pSettings;
                    ItemList    list;
                };

                typedef std::map<int,Layer> LayerMap;



                void                    push_back   (int layer, ItemPtr spItem);

                LayerMap::iterator      begin       (void)      { return mLayers.begin(); }
                LayerMap::iterator      end         (void)      { return mLayers.end(); }

                ItemPtr                 getItem     (unsigned int id);

            protected:
                LayerMap    mLayers;
            };


            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class RasterizerGL
            {
            public:

                void            initialize      (void);
                void            shutdown        (void);

                CameraPtr       createCamera    (float fov, float nearDist, float farDist, glm::mat4& viewMatrix);
                
                LightPtr        createLight     (void);
                LightSetPtr     createLightSet  (void);

                MaterialPtr     createMaterial              (std::string fragShader);
                MaterialPtr     createSolidColorMaterial    (const glgeom::color3f& rgb);
                MaterialPtr     createPhongMaterial         (const glgeom::material_phong_f& mat);

                TexturePtr      createTexture               (const char* filename);

                TransformPtr    createTransform             (glm::mat4& mat);
                TransformPtr    createTransform             (float tx, float ty, float tz);
                TransformPtr    createTransform             (const glgeom::vector3f& scale, const glgeom::point3f& center);
                TransformPtr    createTransformBillboardXY  (float tx, float ty, float tz);
                TransformPtr    createTransformBillboardXYS (float tx, float ty, float tz, float sx, float sy, float sz);
                TransformPtr    createTransformEye          (float tx, float ty, float tz, glgeom::radians z_angle);

                GeometryPtr     createQuadList  (std::vector<glgeom::point3f>& quads);
                GeometryPtr     createQuadList  (std::vector<unsigned short>& indices, 
                                                    std::vector<glgeom::point3f>& positions, 
                                                    std::vector<glgeom::vector3f>& normals,
                                                    std::vector<glgeom::color3f>& colors);
                GeometryPtr     createQuadList  (std::vector<unsigned short>& indices,
                                                 std::vector<lx0::uint8>& faceFlags,
                                                    std::vector<glgeom::point3f>& positions, 
                                                    std::vector<glgeom::vector3f>& normals,
                                                    std::vector<glgeom::color3f>& colors);

                void            refreshTextures (void);

                void            beginScene      (RenderAlgorithm& algorithm);
                void            endScene        (void);

                void            rasterizeList   (RenderAlgorithm& algorithm, std::vector<std::shared_ptr<Item>>& list);
                void            rasterize       (GlobalPass& pass, std::shared_ptr<Item> spItem);
                unsigned int    readPixel       (int x, int y);

                struct 
                {
                    GlobalPass*     pGlobalPass;
                    ItemPtr         spItem;
                    unsigned int    itemId;

                    LightSetPtr     spLightSet;
                    CameraPtr       spCamera;
                    MaterialPtr     spMaterial;

                    unsigned int    textureUnit;
                } mContext;

            protected:
                GLuint      _createProgram   (std::string fragShader);
                GLuint      _createProgram2  (std::string fragShader);
                GLuint      _createShader    (const char* filename, GLuint type);
                void        _linkProgram     (GLuint prog);

                std::map<std::string, GLuint> mCachePrograms;

                std::list<ResourcePtr>      mResources;
                std::vector<TexturePtr>     mTextures;
            };
        }
    }

    using namespace lx0::subsystem::rasterizer;
}


