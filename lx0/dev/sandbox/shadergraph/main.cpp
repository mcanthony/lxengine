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
#include <lx0/subsystem/shaderbuilder.hpp>

using namespace lx0;

//===========================================================================//

class MaterialProcessor : public Document::Component
{
public:
    virtual void onAttached (DocumentPtr spDocument) 
    {
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
        mBuilder.buildShaderGLSL(material, desc);

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
            spDocument->attachComponent(new MaterialProcessor);
        }
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
