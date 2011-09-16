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

#include <iostream>
#include <glgeom/prototype/image.hpp>
#include <glgeom/ext/primitive_buffer.hpp>

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
                
                Represents all the instances to render for a particular frame.  The instances are organized
                into a set of layers, each with an ordered list of instances.  Each layer has its own
                set of settings which may control the optimization, re-ordering, etc. of the list
                for that layer.
             */
            class RenderList
            {
            public:
                typedef lx0::subsystem::rasterizer_ns::InstancePtr InstancePtr;
                typedef std::vector<InstancePtr>                ItemList;

                struct Layer
                {
                    void*       pSettings;
                    ItemList    list;
                };

                typedef std::map<int,Layer> LayerMap;



                void                    push_back   (int layer, InstancePtr spInstance);

                LayerMap::iterator      begin       (void)      { return mLayers.begin(); }
                LayerMap::iterator      end         (void)      { return mLayers.end(); }

                InstancePtr                 getInstance     (unsigned int id);

            protected:
                LayerMap    mLayers;
            };


            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            /*!
                Low-level rasterization system.

                @todo Allow 'immmediate-mode' rasterization of primitive_buffer.
                @todo Unify the caching system so that all resources are optionally named.
             */
            class RasterizerGL
            {
            public:
                                RasterizerGL    ();
                                ~RasterizerGL   ();

                void            initialize      (void);
                void            shutdown        (void);

                CameraPtr       createCamera    (glgeom::radians fov, float nearDist, float farDist, glm::mat4& viewMatrix);
                
                LightSetPtr     createLightSet              (void);
                LightPtr        createLight                 (void);
                LightPtr        createLight                 (const glgeom::point_light_f& light);

                MaterialPtr     createMaterial              (std::string fragShader);
                MaterialPtr     createMaterial              (std::string vertexShader, std::string geometryShader, std::string fragmentShader);
                MaterialPtr     createMaterial              (std::string name, std::string fragmentSource, lxvar parameters);
                MaterialPtr     createSolidColorMaterial    (const glgeom::color3f& rgb);
                MaterialPtr     createVertexColorMaterial   (void);
                MaterialPtr     createPhongMaterial         (const glgeom::material_phong_f& mat);

                TexturePtr      createTexture               (const char* filename);
                TexturePtr      createTextureCubeMap        (const char* xpos, const char* xneg, const char* ypos, const char* yneg, const char* zpos, const char* zneg); 
                TexturePtr      createTextureCubeMap        (const glgeom::cubemap3f& image); 
                TexturePtr      createTextureDDS            (std::istream& stream);
                TexturePtr      createTexture3f             (const glgeom::image3f& image);
                void            cacheTexture                (std::string name, TexturePtr spTexture);

                TransformPtr    createTransform             (glm::mat4& mat);
                TransformPtr    createTransform             (float tx, float ty, float tz);
                TransformPtr    createTransform             (const glgeom::vector3f& scale, const glgeom::point3f& center);
                TransformPtr    createTransformBillboardXY  (float tx, float ty, float tz);
                TransformPtr    createTransformBillboardXYS (float tx, float ty, float tz, float sx, float sy, float sz);
                TransformPtr    createTransformEye          (float tx, float ty, float tz, glgeom::radians z_angle);

                GeometryPtr     createGeometry  (glgeom::primitive_buffer& primitive);
                GeometryPtr     createQuadList  (std::vector<glgeom::point3f>& positions, 
                                                 std::vector<glgeom::color3f>& colors);
                GeometryPtr     createQuadList  (std::vector<lx0::uint16>& indices,
                                                 std::vector<glgeom::point3f>& positions);
                GeometryPtr     createQuadList  (std::vector<lx0::uint16>& indices, 
                                                 std::vector<glgeom::point3f>& positions, 
                                                 std::vector<glgeom::vector3f>& normals,
                                                 std::vector<glgeom::color3f>& colors);
                GeometryPtr     createQuadList  (std::vector<lx0::uint16>& indices,
                                                 std::vector<lx0::uint8>& faceFlags,
                                                 std::vector<glgeom::point3f>& positions, 
                                                 std::vector<glgeom::vector3f>& normals,
                                                 std::vector<glgeom::color3f>& colors);

                void            refreshTextures (void);

                void            beginFrame      (RenderAlgorithm& algorithm);
                void            endFrame        (void);

                void            rasterizeList   (RenderAlgorithm& algorithm, std::vector<std::shared_ptr<Instance>>& list);
                void            rasterizeItem   (GlobalPass& pass, std::shared_ptr<Instance> spInstance);
                
                unsigned int    readPixel       (int x, int y);
                void            readBackBuffer  (glgeom::image3f& img);

                struct ActiveLights
                {
                    std::vector<glgeom::point3f> positionsEc;
                    std::vector<glgeom::color3f> colors;
                    std::vector<glm::vec3>       attenuation;

                    std::vector<float>           glowRadius;
                    std::vector<float>           glowMultiplier;
                    std::vector<float>           glowExponent;
                };

                struct FrameContext
                {
                    std::map<LightSet*,ActiveLights> activeLights;
                };

                /*
                    The context of "what's currently happening" in the rasterization
                    process.  This includes the resolved data of what light set is
                    being used, how many texture units are being used, etc.
                 */
                struct Context
                {
                    Context()
                        : tbFlatShading     (boost::indeterminate)
                    {}

                    GlobalPass*     pGlobalPass;
                    InstancePtr     spInstance;
                    unsigned int    itemId;

                    LightSetPtr     spLightSet;
                    CameraPtr       spCamera;
                    MaterialPtr     spMaterial;

                    unsigned int    textureUnit;

                    boost::tribool  tbFlatShading;

                    FrameContext    frame;

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
                GLuint      _createProgramFromFile  (std::string filename);
                GLuint      _createProgram          (std::string uniqueId, std::string& source);
                GLuint      _createProgram2         (std::string source);
                GLuint      _createShader           (const char* filename, GLuint type);
                GLuint      _createShader2          (std::string& source, GLuint type);
                void        _linkProgram            (GLuint prog, const char* pszSource = nullptr);

                std::unique_ptr<GLInterface>      gl;

                std::map<std::string, GLuint>       mCachePrograms;
                std::list<ResourcePtr>              mResources;
                std::vector<TexturePtr>             mTextures;
            public:
                std::map<std::string,TexturePtr>    mTextureCache;      //!< @todo Replace with a more sophisicated, formal cache
                lx0::uint32                         mFrameNum;

            protected:
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
                    lx0::Timer  tmLightSetActivate;
                    lx0::Timer  tmTransformActivate;
                } mStats;
            };
        }
    }
}
