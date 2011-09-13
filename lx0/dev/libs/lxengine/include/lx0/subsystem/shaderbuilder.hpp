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

#include <string>
#include <set>
#include <functional>

#include <glgeom/core/primitives.hpp>
#include <glgeom/core/color.hpp>
#include <glgeom/prototype/image.hpp>
#include <lx0/core/lxvar/lxvar.hpp>

namespace lx0 
{
    namespace subsystem
    {
        /*!
            \defgroup lx0_subsystem_shaderbuilder lx0_subsystem_shaderbuilder
            \ingroup Subsystem
         */
        namespace shaderbuilder_ns
        {
            namespace detail
            {
                class GLSLBuilder;
                class LambdaBuilder;
                class NativeBuilder;
            }
            
            //===========================================================================//
            //!
            /*!
             */
            class ShaderBuilder
            {
            public:
                struct Material
                {
                    std::string     uniqueName;
                    std::string     source;
                    lx0::lxvar      parameters;
                };

                struct ShaderContext
                {
                    typedef     std::function<glgeom::color3f (const glgeom::ray3f&)>   TraceFunction;

                    glm::mat4               unifViewMatrix;

                    int                     unifLightCount;
                    std::vector<glm::vec3>  unifLightPosition;
                    std::vector<glm::vec3>  unifLightColor;

                    glm::vec3               unifEyeWc;

                    glm::vec3               fragVertexWc;
                    glm::vec3               fragVertexOc;
                    glm::vec3               fragVertexEc;

                    glm::vec3               fragNormalWc;
                    glm::vec3               fragNormalOc;
                    glm::vec3               fragNormalEc;

                    TraceFunction           traceFunc;
                };
                typedef std::function<glm::vec3 (const ShaderContext&)> ShadeFunction;


                                ShaderBuilder();
                                ~ShaderBuilder();

                void            loadNode            (std::string path);

                Material        buildShaderGLSL     (lxvar graph)   { Material m; buildShaderGLSL(m, graph); return m; }
                void            buildShaderGLSL     (Material& material, lxvar graph);
                ShadeFunction   buildShaderLambda   (lxvar graph);
                void            x_buildShaderNative (lxvar graph);

                void            addTexture          (std::string id, std::shared_ptr<glgeom::cubemap3f> spImage);
                void            addTexture          (std::string id, std::shared_ptr<glgeom::image3f> spImage);


            protected:
                typedef std::map<std::string, lxvar> NodeMap;

                void            _loadBuiltinNodes   (void);
                void            _refreshNodes       (void);

                void            _loadNodeImp        (std::string filename);

                detail::GLSLBuilder*        mpGLSLBuilder;
                detail::LambdaBuilder*      mpLambdaBuilder;
                detail::NativeBuilder*      mpNativeBuilder;

                std::vector<std::string>    mNodeDirectories;
                std::vector<std::string>    mNodeFilenames;
                NodeMap                     mNodes;
            };

        }
    }

    using namespace lx0::subsystem::shaderbuilder_ns;
}
