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
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>
#include <iomanip>

// Lx0 headers
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/blendreader.hpp>
#include <glgeom/glgeom.hpp>

using namespace lx0::core;
using namespace lx0::subsystem::blendreader_ns;

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

void displayStructure (BlendReader& reader, std::string name)
{
    auto spStruct = reader.getStructureByName(name);
    std::cout << name << std::endl;
    for (auto it = spStruct->fields.begin(); it != spStruct->fields.end(); ++it)
    {
        std::cout << "\t" << std::setw(16) << it->type << "\t" << it->name 
            << " (" << it->ref << ")"
            << " (" << it->offset << " : " << it->size << " bytes)" <<std::endl;
    }
    std::cout << std::endl;
}

int 
main (int argc, char** argv)
{
    // It is necessary to call lx_init() if the Engine class is not being used.
    lx0::lx_init();

    std::string filename = "media2/models/unit_cube-000.blend";
    if (argc == 2)
        filename = argv[1];

    BlendReader reader;
    if ( reader.open(filename) )
    {
        displayStructure(reader, "Mesh");
        displayStructure(reader, "MVert");
        displayStructure(reader, "MFace");
        displayStructure(reader, "MTFace");

        auto meshBlocks = reader.getBlocksByType("Mesh");
        std::cout << "Meshs = " << meshBlocks.size() << std::endl;

        for (auto it = meshBlocks.begin(); it != meshBlocks.end(); ++it)
        {
            auto spBlock = *it;
            std::cout << "Mesh address: 0x" << spBlock->address << std::endl;
            auto spMesh = reader.readObject( spBlock->address );

            auto totalVertices = spMesh->field<int>("totvert");
            auto totalFaces = spMesh->field<int>("totface");
            std::cout << "Total vertices: " << totalVertices << std::endl;

            auto spVerts = reader.readObject( spMesh->address("mvert") );
            for (int i = 0; i < totalVertices; ++i)
            {
                float x = spVerts->field<float>("co", 0);
                float y = spVerts->field<float>("co", 1);
                float z = spVerts->field<float>("co", 2);

                glgeom::vector3f n;
                n.x = spVerts->field<short>("no", 0) / float(std::numeric_limits<short>::max());
                n.y = spVerts->field<short>("no", 1) / float(std::numeric_limits<short>::max());
                n.z = spVerts->field<short>("no", 2) / float(std::numeric_limits<short>::max());

                std::cout << "V" << i << ":" << x << ", " << y << ", " << z << std::endl;
                std::cout << "N" << i << ":" << n.x << "," << n.y << ", " << n.z << std::endl;

                spVerts->next();
            }

            auto spFaces = reader.readObject( spMesh->address("mface") );
            for (int i = 0; i < totalFaces; ++i)
            {
                int vi[4];
                vi[0] = spFaces->field<int>("v1");
                vi[1] = spFaces->field<int>("v2");
                vi[2] = spFaces->field<int>("v3");
                vi[3] = spFaces->field<int>("v4");

                std::cout << "F" << i << ": " 
                    << vi[0] << ", " 
                    << vi[1] << ", "
                    << vi[2] << ", "
                    << vi[3] << std::endl;

                spFaces->next();
            }
        }
    }
    else
        lx_warn("Could not open file!");

    return 0;
}
