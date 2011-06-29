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
#include <boost/format.hpp>

// Lx0 headers
#include <lx0/lxengine.hpp>

using namespace lx0;


class Fragment
{
public:
    struct Input
    {
        std::string     mName;
        std::string     mType;
        lxvar           mDefault;
        std::string     mSemantic;
    };

    std::string                         mName;
    std::vector<Input>                  mInputs;
    std::string                         mOutput;
    std::map<std::string, std::string>  mSource;
};

class ShaderGraphRuntime
{
public:
    void        registerFragment    (lxvar frag);
    std::string compile             (lxvar desc);

    std::map<std::string, std::unique_ptr<Fragment>> mFragments;
};

void
ShaderGraphRuntime::registerFragment (lxvar frag)
{
    Fragment* pFrag = new Fragment;

    pFrag->mName = frag.find("name").as<std::string>();
    pFrag->mOutput = frag.find("output").as<std::string>();

    auto inputs = frag.find("input");
    for (auto it = inputs.begin(); it != inputs.end(); ++it)
    {
        Fragment::Input in;
        in.mName = (*it).at(0).as<std::string>();
        in.mType = (*it).at(1).as<std::string>();
        in.mDefault = (*it).at(2);
        in.mSemantic = ((*it).size() > 3) ? (*it).at(3).as<std::string>() : "";
        
        pFrag->mInputs.push_back(in);
    }

    mFragments.insert( std::make_pair(pFrag->mName, std::unique_ptr<Fragment>(pFrag)) );
}

std::string
ShaderGraphRuntime::compile (lxvar desc)
{
    lx_check_error( desc.is_array() );
    Fragment* pFrag = mFragments.find(desc.at(0).as<std::string>())->second.get();

    std::ostringstream func;
    func << pFrag->mOutput << " " << pFrag->mName << "(";
    for (auto it = pFrag->mInputs.begin(); it != pFrag->mInputs.end(); ++it)
    {
        func << it->mType << " " << it->mName;
        func << ((it + 1 == pFrag->mInputs.end()) ? ")\n" : ", ");
    }
    func << "{" << std::endl;
    func << "}" << std::endl;

    std::ostringstream call;
    call << pFrag->mName << "(";
    for (auto it = pFrag->mInputs.begin(); it != pFrag->mInputs.end(); ++it)
    {
        lxvar def = it->mDefault;

        if (def.is_array() && def.size() == 3)
        {
            call << "vec3("
                 << def.at(0).as<float>() << "," 
                 << def.at(1).as<float>() << "," 
                 << def.at(2).as<float>() << ")";
        }
        else if (def.is_float())
            call << def.as<float>();
        else if (def.is_int())
            call << def.as<int>();
        
        if (it + 1 != pFrag->mInputs.end())
            call << ", ";
        else
            call << ")";
    }

    std::string source;
    source += "#version 150\n";
    source += "\n";
    source += "// Standard shader constants\n";
    source += "const float PI = 3.14159265358979323846264;\n";
    source += "\n";
    source += "// Standard fragment shader variables\n";
    source += "in vec3         fragVertexOc;\n";
    source += "in vec3         fragVertexEc;\n";
    source += "in vec3         fragNormalOc;\n";
    source += "in vec3         fragNormalEc;\n";
    source += "in vec3         fragColor;\n";
    source += "\n";
    source += "out vec4        out_color;\n";
    source += "\n";
    source += "///// Generated Uniforms /////\n";
    source += "\n";
    /*for (std::vector<std::string>::iterator it = build.uniforms.begin(); it != build.uniforms.end(); ++it)
    {
        source += *it;
    }*/
    source += "\n";

    /*for (std::vector<std::string>::iterator it = build.functions.begin(); it != build.functions.end(); ++it)
    {
        source += *it;
        source += "\n";
    }*/
    source += func.str() + "\n";
    source += "\n";
    source += "void main()\n";
    source += "{\n";
    source += "    out_color = " + call.str() + ";\n";
    source += "}\n";

    return source;
}

//===========================================================================//

class MaterialProcessor : public Document::Component
{
public:
    virtual void onAttached (DocumentPtr spDocument) 
    {
        _loadNode("solid");
        _loadNode("checker");

        auto vMats = spDocument->getElementsByTagName("Material");
        for (auto it = vMats.begin(); it != vMats.end(); ++it)
            _processMaterial(*it);
    }

protected:
    void _loadNode (const char* name)
    {
        std::string path = "media2/appdata/sb_shadergraph/";
        std::string filename = path + name + ".node";

        std::cout << boost::format("* Adding node '%s'\n") % filename;

        lxvar value = lx_file_to_json(filename.c_str());
        mNodes.insert(std::make_pair(name, value));
    }

    void _processMaterial (ElementPtr spElem)
    {
        std::cout << "-----------------------------------------------------------------------\n";
        std::cout << boost::format("Processing '%1%'\n") % spElem->attr("id").as<std::string>();

        lxvar value = spElem->value();
        lx_check_error( value.is_map() && value.size() == 1 && value.find("graph").is_defined() );

        lxvar graph = value.find("graph");
        std::cout << format_tabbed(graph);

        std::string type = graph.find("_type").as<std::string>();

        std::cout << "-----------------------------------------------------------------------\n";
        std::stringstream ss;
        ss  << "#version 150\n"
            << "\n"
            << "void main()\n"
            << "{\n"
            << "    out_color = " << "()" << ";\n"
            << "}\n";
        std::cout << ss.str();
        std::cout << "\n";
    }

    std::map<std::string, lxvar> mNodes;
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

            // For each material, process the graph and produce an output by it's name

            ShaderGraphRuntime runtime;

            lxvar frag = lxvar::parse( lx0::lx_file_to_string("data/sandbox_shadergraph/phong.lxfrag").c_str() );
            runtime.registerFragment(frag);
            std::string src = runtime.compile(lxvar::parse("phong { }"));
            //std::cout << src << std::endl;
        }
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
