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

    mNodeDirectories.push_back("common/shaders/shaderbuilder/shading");
    mNodeDirectories.push_back("common/shaders/shaderbuilder/patterns");
    mNodeDirectories.push_back("common/shaders/shaderbuilder/mappers");

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

        for (auto it = nodeFiles.begin(); it != nodeFiles.end(); ++it)
        {
            try
            {
                _loadNodeImp(*it);
            }
            catch (lx0::error_exception& e)
            {
                lx_warn("Error with node file '%s'.  Ignoring. Error details: %s\n", it->c_str(), e.what());
            }
        }
    }
    catch (boost::system::system_error&)
    {
        throw lx_error_exception("Error reading ShaderBuilder node files");
    }
}


void ShaderBuilder::loadNode (std::string filename)
{
    mNodeFilenames.push_back(filename);
    _loadNodeImp(filename);
}

/*
    The source can currently be specified in one of three ways:

    * A string source fragment
    * An array of strings
    * A filename containing the source

    This function will normalize things such that the given node["source"]
    field always is a single source string.
 */
static
void 
_normalizeFragmentSource (lxvar& value, boost::filesystem::path& path)
{
    lx_check_error(value.find("source").is_array() || value.find("source").is_string());

    //
    // Check if the source field is a reference to a file - in which case
    // load it and swap it into the node definition.
    //
    if (value["source"].is_string())
    {
        std::string filename = value["source"].as<std::string>();
        
        if (!boost::ends_with(filename, ".nfrag"))
        {
            // Presumably it's already in the right format: nothing needs to be
            // done.
        }
        else
        {
            //
            // The source appears to be coming from a file: load that file into
            // a string.
            //
            boost::filesystem::path fragPath( path.parent_path().string() + "/" + filename );
            std::string source = lx0::string_from_file(fragPath.string());

            value["source"] = source;
        }
    }
    else if (value["source"].is_array())
    {
        std::string source;
        for (auto it = value["source"].begin(); it != value["source"].end(); ++it)
            source += (*it).as<std::string>() + "\n";

        value["source"] = source;
    }
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
    std::string id = path.filename().string();
    id = id.substr(0, id.length() - path.extension().string().length());

    try
    {
        lxvar value = lxvar_from_file(filename);

        lx_check_error(value["output"].is_defined(),    "'output' not defined for node '%s'", filename);
        lx_check_error(value["input"].is_defined(),     "'input' not defined for node '%s'", filename);
        lx_check_error(value["source"].is_defined(),    "'source' not defined for node '%s'", filename);

        _normalizeFragmentSource(value, path);

        bool bExists = !mNodes.insert(std::make_pair(id, value)).second;

        lx_check_warn(!bExists, "ShaderBuilder node by name '%s' already exists in cache.  Ignoring new defintion", id);
    }
    catch (lx0::error_exception& e)
    {
        e.location(__FILE__, __LINE__);
        e.detail("Error loading node from file '%s'", filename);
        e.detail("Ignoring node.");
        throw;
    }
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
    lx_check_error(graph.is_defined());

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
    throw lx_error_exception("Not yet implemented!");
}

void            
ShaderBuilder::addTexture (std::string id, std::shared_ptr<glgeom::cubemap3f> spImage)
{
    mpLambdaBuilder->addTexture(id, spImage);
}

void            
ShaderBuilder::addTexture (std::string id, std::shared_ptr<glgeom::image3f> spImage)
{
    mpLambdaBuilder->addTexture(id, spImage);
}

