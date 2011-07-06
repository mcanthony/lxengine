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
                class LambdaBuilder;
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
                    glm::vec3   fragVertexOc;
                };
                typedef std::function<glm::vec3 (const ShaderContext&)> ShadeFunction;

                                ShaderBuilder();
                                ~ShaderBuilder();

                void            loadNode            (std::string path);
                void            buildShaderGLSL     (Material& material, lxvar graph);

                ShadeFunction   buildShaderLambda   (lxvar graph);

                void            x_buildShaderNative (lxvar graph);

            protected:
                struct Declaration
                {
                    std::string     type;
                    std::string     count;
                    std::string     name;
                };

                struct Shader
                {
                    std::string                 mName;
                    std::string                 mVersion;
                    std::vector<std::string>    mConstants;
                    std::vector<std::string>    mShaderInputs;
                    std::vector<std::string>    mShaderOutputs;
                    std::vector<std::string>    mUniforms;
                    std::vector<Declaration>    mNodeUniforms;
                    std::vector<std::string>    mFunctions;
                    std::vector<std::string>    mSource;
                };

                struct Context
                {
                    Context() : mNodeCount (0) {}

                    int                         mNodeCount;
                    std::set<std::string>       mFunctionsBuilt;
                    std::set<std::string>       mNodeUniforms;
                    std::vector<std::string>    mArgumentStack;
                };

                typedef std::map<std::string, lxvar> NodeMap;

                void            _loadBuiltinNodes   (void);
                void            _refreshNodes       (void);

                void            _loadNodeImp        (std::string filename);

                int             _processNode        (Shader& shader, Context& context, lxvar& parameters, lxvar desc, std::string requiredOutputType);
                void            _processUniforms    (Shader& shader, Context& context, lxvar& graph);


                std::string     _valueToStr         (lxvar type, lxvar value);
                std::string     _formatSource       (Shader& shader);

                std::vector<std::string>    mNodeDirectories;
                std::vector<std::string>    mNodeFilenames;
                NodeMap                     mNodes;

                detail::LambdaBuilder*      mpLambdaBuilder;
            };

        }
    }

    using namespace lx0::subsystem::shaderbuilder_ns;
}
