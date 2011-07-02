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

using namespace lx0::subsystem::shaderbuilder_ns;
using namespace lx0;


//===========================================================================//

namespace {

    std::string _indent (const std::string& s, std::string space)
    {
        std::string t;
        t.reserve(s.length());
        t += space;
        for (size_t i = 0; i < s.length(); ++i)
        {
            t += s[i];
            if (s[i] == '\n' && i + 1 < s.length())
                t += space;
        }
        return t;
    }

    std::string _refine (std::string s, std::string space)
    {
        boost::trim(s);
        return _indent(s, space);
    }
}

//===========================================================================//

ShaderBuilder::ShaderBuilder()
{
    _loadBuiltinNodes();
}

void
ShaderBuilder::_loadBuiltinNodes ()
{
    using namespace boost::filesystem;

    std::vector<std::string> nodes;
    for (directory_iterator dit("media2/shaders/glsl/nodes/"); dit != directory_iterator(); ++dit)
    {
        if (is_regular_file(dit->status()))
        {
            std::string filename = dit->path().filename();
            if (boost::ends_with(filename, std::string(".node")))
            {
                std::string path = dit->path().string();
                nodes.push_back(path);
            }
        }
    }

    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        loadNode(*it);
}

void ShaderBuilder::loadNode (std::string filename)
{
    namespace bfs = boost::filesystem;

    lx_debug("Loading shader builder node '%s'", filename.c_str());

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

void ShaderBuilder::buildShader (Material& material, lxvar graph)
{
    Shader          shader;
    lxvar           parameters;
    Context         context;

    //
    // Set up all the predefined variables
    //
    shader.mVersion = "#version 150";

    shader.mConstants.push_back("const float PI = 3.14159265358979323846264;");

    shader.mShaderInputs.push_back("in vec3         fragVertexOc;");
    shader.mShaderInputs.push_back("in vec3         fragVertexEc;");
    shader.mShaderInputs.push_back("in vec3         fragNormalOc;");
    shader.mShaderInputs.push_back("in vec3         fragNormalEc;");
    shader.mShaderInputs.push_back("in vec3         fragColor;");

    shader.mShaderOutputs.push_back("out vec4        out_color;");

    //
    // Start processing at the root and recurse through the nodes
    //
    _processNode(shader, context, parameters, graph, "vec4");

    //
    // Copy  the results (and discard all the intermediate data)
    //
    material.uniqueName = shader.mName;
    material.source     = _formatSource(shader);
    material.parameters = parameters;
}


int 
ShaderBuilder::_processNode (Shader& shader, Context& context, lxvar& parameters, lxvar graph, std::string requiredOutputType)
{
    lx_check_error(graph.find("_type").is_defined());

    const int         id       = context.mNodeCount++;
    const std::string nodeType = graph["_type"].as<std::string>();

    auto it = mNodes.find(nodeType);
    if (it == mNodes.end())
        lx_error("Node of type '%s' not loaded.", nodeType.c_str());

    // Build up a unique name for this shader.  Do this for caching purposes so 
    // that duplicate names indicate a shader can be reused.
    shader.mName += boost::str( boost::format("%s[") % nodeType );

    lxvar node = mNodes[nodeType];
    lx_check_error(node.find("output").is_defined());
    lx_check_error(node.find("input").is_defined());

    const std::string outputType = node.find("output").as<std::string>();
    const std::string funcName = boost::str(boost::format("fn_%1%") % nodeType);

    //
    // Write out the node's code if this is the first time it has been seen
    //
    if (context.mFunctionsBuilt.insert(nodeType).second)
    {
        std::stringstream ss;
        ss << boost::format("%2% %1% (") % funcName % outputType;
        int i = 0;
        for (auto it = node["input"].begin(); it != node["input"].end(); ++it)
        {
            lxvar& input = *it;
            ss << input[0].as<std::string>() << " " << it.key();
            if (i + 1 < node["input"].size())
                ss << ", ";
            ++i;
        }
        ss << ")\n"
            << "{\n"
            << _refine(node["source"].as<std::string>(), "    ") << "\n"
            << "}\n";

        shader.mFunctions.push_back(ss.str());
    }

    // Generate the required external uniforms
    _processUniforms(shader, context, node);

    // Generate the node inputs
    {
        std::stringstream ss;
        int i = 0;
        for (auto it = node["input"].begin(); it != node["input"].end(); ++it)
        {
            const auto  argName = it.key();
            const auto  type = (*it)[0].as<std::string>();
            const auto& defaultValue = (*it)[1];

            ss << type << " n" << id << "_" << argName << " = ";

            if (graph.find(argName).is_map())
            {
                context.mArgumentStack.push_back(argName);
                auto childId = _processNode(shader, context, parameters, graph[argName], type);
                context.mArgumentStack.pop_back();

                const auto returnValueName = boost::str( boost::format("n%1%_ret") % childId ); 
                ss << returnValueName;
            }
            else
            {
                shader.mName += "U";

                //
                // Either a default value or a user-specified value; in either case,
                // this becomes a uniform.
                //
                std::string argUniformName = "unif_";
                if (!context.mArgumentStack.empty())
                    argUniformName += boost::join(context.mArgumentStack, "_") + "_";
                argUniformName += argName;

                lxvar userValue = graph.find(argName);
                lxvar value = userValue.is_defined() ? userValue : defaultValue;

                // Store the value in the parameter table
                parameters[argUniformName][0] = type;
                parameters[argUniformName][1] = value;

                // 
                // Theoretically, the uniforms should *always* be initialized to the default value and
                // not the user-specified value (if it exists).  In practical, for testing, it's nice to
                // have the generated shader set to the values set by the graph first generated it so that
                // the viewer doesn't have to deal with any parameter mapping (i.e. it can just load the
                // fragment file and be done).
                //
                std::string uniform = boost::str( boost::format("uniform %-6s %-32s = %s;") % type % argUniformName % _valueToStr(type, value) );
                shader.mUniforms.push_back(uniform);

                ss << argUniformName;
            }
                
            ss << ";\n";
            ++i;
        }

        shader.mName += "]";
        shader.mSource.push_back(ss.str());
    }

    // Generate the node function call
    {
        std::stringstream ss;
        

        std::string convertPrefix;
        std::string convertPostfix;
        if (requiredOutputType != outputType)
        {
            if (requiredOutputType == "vec4" && outputType == "vec3")
            {
                convertPrefix = "vec4( ";
                convertPostfix = ", 1.0)";
            }
            else
                lx_warn("Implicit conversion between different types!");
        }

        ss << requiredOutputType << " n" << id << "_ret = ";
        ss << convertPrefix;
        ss << funcName << "(";
        int i = 0;
        for (auto it = node["input"].begin(); it != node["input"].end(); ++it)
        {
            const auto  argName = it.key();

            ss << "n" << id << "_" << argName;
            if (i + 1 < node["input"].size())
                ss << ", ";
            ++i;
        }
        ss << ")";
        ss << convertPostfix;
        ss << ";\n\n";
        shader.mSource.push_back(ss.str());
    }

    return id;
}

void 
ShaderBuilder::_processUniforms (Shader& shader, Context& context, lxvar& node)
{
    auto uniforms = node.find("uniforms");

    if (uniforms.is_defined())
    {
        for (auto it = uniforms.begin(); it != uniforms.end(); ++it)
        {
            Declaration declaration;
            declaration.name  = it.key();
            declaration.type  = (*it)[0].as<std::string>();
            declaration.count = "";

            // Split out the array count, if present
            size_t index = declaration.type.find('[');
            if (index != std::string::npos)
            {
                declaration.count = declaration.type.substr(index);
                declaration.type = declaration.type.substr(0, index);
            }

            if (context.mFunctionsBuilt.insert(declaration.name).second)
                shader.mNodeUniforms.push_back(declaration);
        }
    }
}

std::string 
ShaderBuilder::_valueToStr (lxvar type, lxvar value)
{
    boost::format fmt;
    if (type == "float")
        fmt = boost::format("%f") % value.as<float>();
    else if (type == "vec2")
        fmt = boost::format("vec2(%f, %f)") % value[0].as<float>() % value[1].as<float>();
    else if (type == "vec3")
        fmt = boost::format("vec3(%f, %f, %f)") % value[0].as<float>() % value[1].as<float>() % value[2].as<float>();
    else if (type == "vec4")
        fmt = boost::format("vec4(%f, %f, %f, %f)") % value[0].as<float>() % value[1].as<float>() % value[2].as<float>()  % value[3].as<float>();
    else
    {
        fmt = boost::format("SHADER_GENERATION_ERROR");
        lx_error("Unrecognized type '%s'", type.as<std::string>().c_str());
    }
    return boost::str(fmt);
}

std::string ShaderBuilder::_formatSource (Shader& shader)
{
    std::stringstream ss;
    ss << "// Fragment shader source generated by LxEngine ShaderBuilder\n"
        << "// Generated using LxEngine " << boost::format("v%u.%u.%u\n") % lx0::LXENGINE_VERSION_MAJOR % lx0::LXENGINE_VERSION_MINOR % lx0::LXENGINE_VERSION_REVISION
        << "//\n"
        << "// Unique name: " << shader.mName << "\n"
        << "//\n";
    ss << shader.mVersion << std::endl;
    ss << "\n";
    ss << "// Standard shader constants\n";
    for (auto it = shader.mConstants.begin(); it != shader.mConstants.end(); ++it)
        ss << *it << "\n";
    ss << "\n";
    ss << "// Standard fragment shader variables\n";
    for (auto it = shader.mShaderInputs.begin(); it != shader.mShaderInputs.end(); ++it)
        ss << *it << "\n";
    ss << "\n";
    for (auto it = shader.mShaderOutputs.begin(); it != shader.mShaderOutputs.end(); ++it)
        ss << *it << "\n";
    ss << "\n";
    ss  << "//\n"
        << "// Required External Uniforms\n"
        << "//\n";
    for (auto it = shader.mNodeUniforms.begin(); it != shader.mNodeUniforms.end(); ++it)
        ss << boost::format("uniform %-6s %s%s;\n") % it->type % it->name % it->count;
    ss  << "\n";
    ss  << "//\n"
        << "// Generated Uniforms\n"
        << "//\n";
    ss  << boost::join( shader.mUniforms, "\n" );
    ss << "\n";
    ss << "\n";
    ss << "//\n";
    ss << "// Functions\n";
    ss << "//\n";
    ss << "\n";
    for (auto it = shader.mFunctions.begin(); it != shader.mFunctions.end(); ++it)
        ss << *it << "\n";
    ss << "\n";
    ss << "//\n";
    ss << "// Main Entry-Point\n";
    ss << "//\n";
    ss  << "void main()\n"
        << "{\n";
    for (auto it = shader.mSource.begin(); it != shader.mSource.end(); ++it)
        ss << _indent(*it, "    ");
    ss  << "    out_color = n0_ret;\n"
        << "}\n";
    return ss.str();
}

