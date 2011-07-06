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

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <lx0/core/init/version.hpp>
#include <lx0/core/log/log.hpp>
#include <lx0/subsystem/shaderbuilder.hpp>
#include <lx0/util/misc.hpp>

#include "glslbuilder.hpp"
#include "lambdabuilder.hpp"

using namespace lx0::subsystem::shaderbuilder_ns;
using namespace lx0;

//===========================================================================//

ShaderBuilder::ShaderBuilder()
{
    mpGLSLBuilder = new detail::GLSLBuilder(mNodes);
    mpLambdaBuilder = new detail::LambdaBuilder(mNodes);

    _loadBuiltinNodes();    
}

ShaderBuilder::~ShaderBuilder()
{
    // Not using a std::unique_ptr<> so that lambdabuilder.hpp doesn't have to be included
    // outside this file.
    delete mpGLSLBuilder;
    delete mpLambdaBuilder;
}

void
ShaderBuilder::_loadBuiltinNodes ()
{
    // This method should only be called once
    lx_assert(mNodeDirectories.empty());

    mNodeDirectories.push_back("media2/shaders/shaderbuilder/shading");
    mNodeDirectories.push_back("media2/shaders/shaderbuilder/patterns");
    mNodeDirectories.push_back("media2/shaders/shaderbuilder/mappers");

    _refreshNodes();
}

void 
ShaderBuilder::_refreshNodes (void)
{
    try
    {
        std::vector<std::string> nodeFiles = mNodeFilenames;

        for (auto it = mNodeDirectories.begin(); it != mNodeDirectories.end(); ++it)
            lx0::find_files_in_directory(nodeFiles, it->c_str(),  "node");

        lx0::find_files_in_directory(nodeFiles, "media2/shaders/shaderbuilder/shading",  "node");
        lx0::find_files_in_directory(nodeFiles, "media2/shaders/shaderbuilder/patterns", "node");
        lx0::find_files_in_directory(nodeFiles, "media2/shaders/shaderbuilder/mappers",  "node");

        for (auto it = nodeFiles.begin(); it != nodeFiles.end(); ++it)
            _loadNodeImp(*it);
    }
    catch (boost::system::system_error&)
    {
        lx_error("Error reading ShaderBuilder node files");
    }
}


void ShaderBuilder::loadNode (std::string filename)
{
    mNodeFilenames.push_back(filename);
    _loadNodeImp(filename);
}

void ShaderBuilder::_loadNodeImp (std::string filename)
{
    namespace bfs = boost::filesystem;

    lx_log("Loading shader builder node '%s'", filename.c_str());

    //
    // Create the node identifier (i.e. name) from the filename without
    // the extension.
    //
    bfs::path path(filename);
    std::string id = path.filename();
    id = id.substr(0, id.length() - path.extension().length());

    lxvar value = lxvar_from_file(filename);

    if (value["output"].is_undefined())
        lx_error("'output' not defined for node '%s'", filename.c_str());
    else if (value["input"].is_undefined())
        lx_error("'input' not defined for node '%s'", filename.c_str());
    else
        mNodes.insert(std::make_pair(id, value));
}

void ShaderBuilder::buildShaderGLSL (Material& material, lxvar graph)
{
    return mpGLSLBuilder->buildShader(material, graph);
}

/*!
    Return a dynamically created lambda function for the shader graph.
    
    Warning: lamdba functions have a noticable amount of overhead.  This approach
    is flexible, but not very efficient given the inner loop usage of shaders.
 */
ShaderBuilder::ShadeFunction 
ShaderBuilder::buildShaderLambda (lxvar graph)
{
    return mpLambdaBuilder->buildShader(graph);
}

void 
ShaderBuilder::x_buildShaderNative (lxvar graph)
{
    /*
        The idea for this method is as follows:

        - Construct C/C++ source using a means similar to the GLSL shader generation
        - Call an external compiler to generate a DLL
        - Hook into generated native code and call that

        For performance, *all* shaders should end up in the same DLL.  Meaning:
        - Don't compile the DLL until a shader is used (then compile all known shaders)
        - On shader calls, quickly check if the DLL needs to be recompiled due a shader(s) being added
     */
    lx_error("Not yet implemented!");
}

