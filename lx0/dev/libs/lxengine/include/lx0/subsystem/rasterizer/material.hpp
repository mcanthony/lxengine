//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011-2012 athile@athile.net (http://www.athile.net)

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

            class MaterialType : public std::enable_shared_from_this<MaterialType>
            {
            public:
                struct _Parameter
                {
                    std::string name;
                    GLenum      type;
                    GLint       size;
                    GLint       location;
                };
                struct Uniform : public _Parameter { };
                struct Attribute : public _Parameter { };

                            MaterialType    (GLuint id);
                            ~MaterialType   ();

                virtual void    activate    (RasterizerGL* pRasterizer, GlobalPass& pass);

                MaterialInstancePtr createInstance (lx0::lxvar& parameters);

                void        iterateUniforms     (std::function<void(const Uniform& uniform)> f); 
                void        iterateAttributes   (std::function<void(const Attribute& attribute)> f); 

                GLenum      mGeometryType;

                GLuint      mProgram;
                GLuint      mVertShader;
                GLuint      mGeomShader;
                GLuint      mFragShader;

                std::string mName;
                lx0::lxvar  mDefaults;
            };

            class MaterialInstance : public std::enable_shared_from_this<MaterialInstance>
            {
            public:
                typedef MaterialType::Attribute Attribute;
                typedef MaterialType::Uniform   Uniform;

                                MaterialInstance    (MaterialTypePtr spMaterialType, lx0::lxvar& parameters);

                virtual void    activate   (RasterizerGL* pRasterizer, GlobalPass& pass);
                void            _compile   (RasterizerGL* pRasterizer);

                std::function<void()>   _generateBaseInstruction    (RasterizerGL*);
                std::function<void()>   _generateInstruction        (RasterizerGL*, const Attribute&, lx0::lxvar& value);
                std::function<void()>   _generateInstruction        (RasterizerGL*, const Uniform&, lx0::lxvar& value);

                std::string                         mName;
                MaterialTypePtr                     mspMaterialType;
                lx0::lxvar                          mParameters;

                bool                                mBlend;
                bool                                mZTest;
                bool                                mZWrite;
                bool                                mWireframe;
                int                                 mFilter;
                TexturePtr                          mTextures[8];

                bool                                mbDirty;
                std::vector<std::function<void()>>  mInstructions;

            protected:

            };

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class Material
            {
            public:
                             Material   (GLuint id);

                virtual void activate   (RasterizerGL*, GlobalPass& pass);

                GLenum      mGeometryType;
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
            class VertexColorMaterial : public Material
            {
            public:
                        VertexColorMaterial(GLuint id);

                virtual void activate   (RasterizerGL*, GlobalPass& pass);
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
            class GenericMaterial : public Material
            {
            public:
                                GenericMaterial (GLuint id);

                virtual void    activate        (RasterizerGL*, GlobalPass& pass);

            //protected:
                void            _compile        (RasterizerGL*);

                lxvar                                   mParameters;
                bool                                    mbNeedsCompile;
                std::vector<std::function<void (void)>> mInstructions;
            };

        }
    }
}
