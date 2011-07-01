//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010 athile@athile.net (http://www.athile.net)

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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

// Standard headers
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

// Lx0 headers
#include <lx0/lxengine.hpp>

using namespace lx0;

//===========================================================================//

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


class ShaderBuilder
{
public:
    struct Material
    {
        std::string     uniqueName;
        std::string     source;
        lxvar           parameters;
    };

    void loadNode (const char* name)
    {
        std::string path = "media2/shaders/glsl/nodes/";
        std::string filename = path + name + ".node";

        lxvar value = lxvar_from_file(filename.c_str());
        mNodes.insert(std::make_pair(name, value));
    }

    void buildShader (Material& material, lxvar graph)
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
        _processNode(shader, context, parameters, graph);

        //
        // Copy  the results (and discard all the intermediate data)
        //
        material.uniqueName = shader.mName;
        material.source     = _formatSource(shader);
        material.parameters["shaderUniqueName"] = shader.mName;
        material.parameters["parameters"] = parameters;
    }

protected:
    struct Shader
    {
        std::string                 mName;
        std::string                 mVersion;
        std::vector<std::string>    mConstants;
        std::vector<std::string>    mShaderInputs;
        std::vector<std::string>    mShaderOutputs;
        std::vector<std::string>    mUniforms;
        std::vector<std::string>    mFunctions;
        std::vector<std::string>    mSource;
    };

    struct Context
    {
        Context() : mNodeCount (0) {}

        int                         mNodeCount;
        std::set<std::string>       mFunctionsBuilt;
        std::vector<std::string>    mArgumentStack;
    };

    int _processNode (Shader& shader, Context& context, lxvar& parameters, lxvar desc)
    {
        lx_check_error(desc.find("_type").is_defined());

        const int         id       = context.mNodeCount++;
        const std::string nodeType = desc["_type"].as<std::string>();

        auto it = mNodes.find(nodeType);
        if (it == mNodes.end())
            lx_error("Node of type '%s' not loaded.", desc["_type"].as<std::string>().c_str());

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

                if (desc.find(argName).is_map())
                {
                    context.mArgumentStack.push_back(argName);
                    auto childId = _processNode(shader, context, parameters, desc[argName]);
                    context.mArgumentStack.pop_back();

                    ss << "n" << childId << "_ret";
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

                    lxvar userValue = desc.find(argName);
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
            ss << node["output"].as<std::string>() << " n" << id << "_ret = " << funcName << "(";
            int i = 0;
            for (auto it = node["input"].begin(); it != node["input"].end(); ++it)
            {
                const auto  argName = it.key();

                ss << "n" << id << "_" << argName;
                if (i + 1 < node["input"].size())
                    ss << ", ";
                ++i;
            }
            ss << ");\n\n";
            shader.mSource.push_back(ss.str());
        }

        return id;
    }

    std::string _valueToStr (lxvar type, lxvar value)
    {
        boost::format fmt;
        if (type == "vec2")
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

    std::string _formatSource (Shader& shader)
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

    std::map<std::string, lxvar> mNodes;
};

//===========================================================================//

class MaterialProcessor : public Document::Component
{
public:
    virtual void onAttached (DocumentPtr spDocument) 
    {
        mBuilder.loadNode("solid");
        mBuilder.loadNode("checker");
        mBuilder.loadNode("weave");
        mBuilder.loadNode("spherical");
        mBuilder.loadNode("cube");

        auto vMats = spDocument->getElementsByTagName("Material");
        for (auto it = vMats.begin(); it != vMats.end(); ++it)
            _processMaterial(*it);
    }

protected:
    void _writeFile (std::string filename, std::string content)
    {
        std::ofstream file;
        file.open(filename);
        file << content << std::endl;
        file.close();
    }

    void _processMaterial (ElementPtr spElem)
    {
        std::string name = spElem->attr("id").as<std::string>();
        lxvar desc = spElem->value().find("graph");

        ShaderBuilder::Material material;
        mBuilder.buildShader(material, desc);

        std::string shaderFile = boost::str(boost::format("lxshader_%1%.frag") % material.uniqueName);
        std::string paramFile = boost::str(boost::format("lxparams_%1%.params") % name);
        std::cout << boost::format("Writing shader/params\n\t%1%\n\t%2%\n") % shaderFile % paramFile;
    
        _writeFile(shaderFile, material.source);
        _writeFile(paramFile,  lx0::format_json(material.parameters));
    }


    ShaderBuilder mBuilder;
};


//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        EnginePtr spEngine = Engine::acquire();
        {
            auto spDocument = spEngine->loadDocument("media2/appdata/sb_shadergraph/sample.xml");
            spDocument->attachComponent("material_processor", new MaterialProcessor);
        }
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
