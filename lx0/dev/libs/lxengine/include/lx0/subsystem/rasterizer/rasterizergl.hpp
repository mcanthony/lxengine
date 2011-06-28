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

namespace lx0 
{
    namespace subsystem
    {
        namespace rasterizer_ns
        {

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
                typedef lx0::subsystem::rasterizer_ns::ItemPtr ItemPtr;
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
                                RasterizerGL    ();
                                ~RasterizerGL   ();

                void            initialize      (void);
                void            shutdown        (void);

                CameraPtr       createCamera    (float fov, float nearDist, float farDist, glm::mat4& viewMatrix);
                
                LightPtr        createLight     (void);
                LightSetPtr     createLightSet  (void);

                MaterialPtr     createMaterial              (std::string fragShader);
                MaterialPtr     createSolidColorMaterial    (const glgeom::color3f& rgb);
                MaterialPtr     createVertexColorMaterial   (void);
                MaterialPtr     createPhongMaterial         (const glgeom::material_phong_f& mat);

                TexturePtr      createTexture               (const char* filename);

                TransformPtr    createTransform             (glm::mat4& mat);
                TransformPtr    createTransform             (float tx, float ty, float tz);
                TransformPtr    createTransform             (const glgeom::vector3f& scale, const glgeom::point3f& center);
                TransformPtr    createTransformBillboardXY  (float tx, float ty, float tz);
                TransformPtr    createTransformBillboardXYS (float tx, float ty, float tz, float sx, float sy, float sz);
                TransformPtr    createTransformEye          (float tx, float ty, float tz, glgeom::radians z_angle);

                GeometryPtr     createQuadList  (std::vector<glgeom::point3f>& positions, 
                                                 std::vector<glgeom::color3f>& colors);
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
                void            rasterizeItem   (GlobalPass& pass, std::shared_ptr<Item> spItem);
                unsigned int    readPixel       (int x, int y);

                struct Context
                {
                    Context()
                        : tbFlatShading     (boost::indeterminate)
                    {}

                    GlobalPass*     pGlobalPass;
                    ItemPtr         spItem;
                    unsigned int    itemId;

                    LightSetPtr     spLightSet;
                    CameraPtr       spCamera;
                    MaterialPtr     spMaterial;

                    unsigned int    textureUnit;

                    boost::tribool  tbFlatShading;

                    class Uniforms
                    {
                    public:
                        void reset()
                        {
                            spProjMatrix.reset();
                            spViewMatrix.reset();
                        }
                        void activate();

                        std::shared_ptr<glm::mat4>  spProjMatrix;
                        std::shared_ptr<glm::mat4>  spViewMatrix;

                    } uniforms;

                } mContext;

            protected:
                GLuint      _createProgram   (std::string fragShader);
                GLuint      _createProgram2  (std::string fragShader);
                GLuint      _createShader    (const char* filename, GLuint type);
                void        _linkProgram     (GLuint prog);

                std::unique_ptr<GLInterface>      gl;

                std::map<std::string, GLuint>   mCachePrograms;
                std::list<ResourcePtr>          mResources;
                std::vector<TexturePtr>         mTextures;

                bool                            mInited;
                bool                            mShutdown;

                struct
                {
                    lx0::Timer  tmLifetime;
                    lx0::Timer  tmScene;
                    lx0::Timer  tmRasterizeList;
                    lx0::Timer  tmRasterizeItem;
                    lx0::Timer  tmMaterialActivate;
                    lx0::Timer  tmGeometryActivate;
                } mStats;
            };
        }
    }
}
