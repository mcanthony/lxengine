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

#include <lx0/subsystem/shaderbuilder.hpp>
#include <glgeom/prototype/image.hpp>

namespace lx0 
{
    namespace subsystem
    {
        namespace shaderbuilder_ns
        {
            namespace detail
            {
                class TextureCache
                {
                public:
                    typedef std::shared_ptr<glgeom::image3f>    Image3fPtr;

                    void        add     (const std::string& id, Image3fPtr spImage);
                    Image3fPtr  acquire (const std::string& filename);

                protected:
                    std::map<std::string, Image3fPtr> mCache;
                };

                class Cache
                {
                public:
                    typedef std::shared_ptr<glgeom::cubemap3f>    Image3fPtr;

                    void        add     (const std::string& id, Image3fPtr spImage);
                    Image3fPtr  acquire (const std::string& id);

                protected:
                    std::map<std::string, Image3fPtr> mCache;
                };

                class LambdaBuilder
                {
                public:
                    typedef std::map<std::string, lxvar>      NodeMap;
                    typedef lx0::ShaderBuilder::ShaderContext Context;
                    typedef lx0::ShaderBuilder::ShadeFunction ShadeFunction;

                    typedef std::function<float     (const Context&)> FunctionFloat;
                    typedef std::function<glm::vec2 (const Context&)> FunctionVec2;
                    typedef std::function<glm::vec3 (const Context&)> FunctionVec3;
                    typedef std::function<glm::vec4 (const Context&)> FunctionVec4;

                    LambdaBuilder (NodeMap& nodeMap);

                    ShadeFunction   buildShader (lx0::lxvar graph);

                    void            addTexture          (std::string id, std::shared_ptr<glgeom::image3f> spImage);
                    void            addTexture          (std::string id, std::shared_ptr<glgeom::cubemap3f> spImage);

                protected:
                    void                                _init            (void);

                    FunctionFloat                       _buildFloat      (lxvar param);
                    FunctionVec2                        _buildVec2       (lxvar param);
                    FunctionVec3                        _buildVec3       (lxvar param);
                    std::shared_ptr<glgeom::image3f>    _buildSampler2d  (lxvar param);
                    std::shared_ptr<glgeom::cubemap3f>  _buildSamplerCube (lxvar param);
                 
                    NodeMap&        mNodes;
                    TextureCache    mTextureCache;
                    Cache           mCubemapCache;

                    std::map<std::string, std::function<FunctionVec4 (lxvar,lxvar)>>  mFuncs4f;
                    std::map<std::string, std::function<FunctionVec3 (lxvar,lxvar)>>  mFuncs3f;
                    std::map<std::string, std::function<FunctionVec2 (lxvar,lxvar)>>  mFuncs2f;
                    std::map<std::string, std::function<FunctionFloat(lxvar,lxvar)>>  mFuncs1f;
                };
            }
        }
    }
}
