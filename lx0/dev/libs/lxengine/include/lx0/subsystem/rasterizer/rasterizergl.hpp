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
                         : tbWireframe          (boost::indeterminate)
                         , tbFlatShading        (boost::indeterminate)
                         , optClearColor        (false, glgeom::color4f(0, 0, 0, 1))
                     { }

                FrameBufferPtr  spFrameBuffer;          // optional: screen is the default

                FrameBufferPtr  spSourceFBO;            // if set, will blit this as a full screen quad

                CameraPtr       spCamera;
                LightSetPtr     spLightSet;
                MaterialPtr     spMaterial;

                boost::tribool                      tbWireframe;
                boost::tribool                      tbFlatShading;
                std::pair<bool, glgeom::color4f>    optClearColor;
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
            //! 
            /*!
                \ingroup lx0_subsystem_rasterizer

                Reference: OpenGL 4.0 Shading Language Cookbook, Chapter 4.
             */
            class FrameBuffer 
                : public std::enable_shared_from_this<FrameBuffer>
            {
            public:
                enum Type
                {
                    eDefaultFrameBuffer = -1,
                    eCreateFrameBuffer, 
                };

                FrameBuffer     (Type type, int width=0, int height=0);
                ~FrameBuffer    (void);

                int             width       (void) const { return mWidth; }
                int             height      (void) const { return mHeight; }
                GLuint          textureId   (void) const { return mTextureHandle; }
                
                void            activate();

            protected:
                GLuint  mHandle;                
                int     mWidth;
                int     mHeight;

                GLuint  mTextureHandle;
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
                Data about a single frame.
             */
            class FrameData
            {
            public:
                FrameData()
                    : shaderProgramActivations (0)
                {
                }

                int     shaderProgramActivations;
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
                friend class Material;

                                RasterizerGL    ();
                                ~RasterizerGL   ();

                void            initialize      (void);
                void            shutdown        (void);

                CameraPtr       createCamera    (glgeom::radians fov, float nearDist, float farDist, glm::mat4& viewMatrix);
                
                FrameBufferPtr  createFrameBuffer           (int width, int height);

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
                TransformPtr    createTransform             (glm::mat4& projMat, glm::mat4& viewMat, glm::mat4& modelMat);
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
                GeometryPtr     createQuadList  (const std::vector<lx0::uint16>& indices,
                                                 const std::vector<lx0::uint8>& faceFlags,
                                                 const std::vector<glgeom::point3f>& positions, 
                                                 const std::vector<glgeom::vector3f>& normals,
                                                 const std::vector<glgeom::color3f>& colors);

                void            refreshTextures (void);

                const FrameData& frameData      (void) const { return mFrameData; }
                void            beginFrame      (RenderAlgorithm& algorithm);
                void            endFrame        (void);

                void            rasterizeList   (RenderAlgorithm& algorithm, std::vector<std::shared_ptr<Instance>>& list);
                void            rasterizeItem   (GlobalPass& pass, std::shared_ptr<Instance> spInstance);
                
                unsigned int    readPixel       (int x, int y);
                void            readFrontBuffer (glgeom::image3f& img);
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

                    // Global pass info
                    GlobalPass*     pGlobalPass;
                    FrameBufferPtr  spFBO;

                    // Instance Info
                    InstancePtr     spInstance;
                    unsigned int    itemId;
                    LightSetPtr     spLightSet;
                    CameraPtr       spCamera;
                    MaterialPtr     spMaterial;
                    TransformPtr    spTransform;

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

                    // Semantic variables
                    GLuint      sourceFBOTexture;

                } mContext;

            protected:
                GLuint      _createProgramFromFile  (std::string filename);
                GLuint      _createProgram          (std::string uniqueId, GLenum geometryType, std::string& source);
                GLuint      _createProgram2         (std::string source);
                GLuint      _createShader           (const char* filename, GLuint type);
                GLuint      _createShader2          (std::string& source, GLuint type);
                void        _linkProgram            (GLuint prog, const char* pszSource = nullptr);
                
                MaterialPtr _acquireDefaultPointMaterial    (void);
                MaterialPtr _acquireDefaultLineMaterial     (void);
                MaterialPtr _acquireDefaultSurfaceMaterial  (void);

                GeometryPtr _acquireFullScreenQuad          (int width, int height);

                void        _readBuffer             (GLenum buffer, glgeom::image3f& img);

                std::unique_ptr<lx0::OpenGlApi3_2>  gl3_2;

                std::map<std::string, GLuint>       mCachePrograms;
                std::list<ResourcePtr>              mResources;
                std::vector<TexturePtr>             mTextures;
            public:
                std::map<std::string,TexturePtr>    mTextureCache;      //!< @todo Replace with a more sophisicated, formal cache
                lx0::uint32                         mFrameNum;

            protected:
                bool                            mInited;
                bool                            mShutdown;
                FrameData                       mFrameData;

                FrameBufferPtr                  mspFBOScreen;

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
