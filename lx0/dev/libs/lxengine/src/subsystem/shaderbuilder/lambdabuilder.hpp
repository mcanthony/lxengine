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

namespace lx0 
{
    namespace subsystem
    {
        namespace shaderbuilder_ns
        {
            namespace detail
            {
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

                    LambdaBuilder (NodeMap& nodeMap) : mNodes (nodeMap) {}

                    ShadeFunction   buildShader (lx0::lxvar graph);

                protected:
                    NodeMap& mNodes;
                };
            }
        }
    }
}
