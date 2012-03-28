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

            class MaterialClass : public std::enable_shared_from_this<MaterialClass>
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

                            MaterialClass    (GLuint id);
                            ~MaterialClass   ();

                virtual void    activate    (RasterizerGL* pRasterizer, GlobalPass& pass);

                MaterialPtr createInstance (lx0::lxvar& parameters);

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

            class Material : public std::enable_shared_from_this<Material>
            {
            public:
                typedef MaterialClass::Attribute Attribute;
                typedef MaterialClass::Uniform   Uniform;

                                Material    (MaterialClassPtr spMaterialClass, lx0::lxvar& parameters);

                virtual void    activate   (RasterizerGL* pRasterizer, GlobalPass& pass);

                void            trimParameterTypes  (void);

                void            _compile   (RasterizerGL* pRasterizer);

                std::function<void()>   _generateBaseInstruction    (RasterizerGL*);
                std::function<void()>   _generateInstruction        (RasterizerGL*, const Attribute&, lx0::lxvar& value);
                std::function<void()>   _generateInstruction        (RasterizerGL*, const Uniform&, lx0::lxvar& value);

                std::string                         mName;
                MaterialClassPtr                     mspMaterialClass;
                lx0::lxvar                          mParameters;

                bool                                mBlend;
                bool                                mZTest;
                bool                                mZWrite;
                bool                                mWireframe;
                int                                 mFilter;
                TexturePtr                          mTextures[8];

            protected:
                bool                                mbDirty;
                std::vector<std::function<void()>>  mInstructions;
            };
        }
    }
}
