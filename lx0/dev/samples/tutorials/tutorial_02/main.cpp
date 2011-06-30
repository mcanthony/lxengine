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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

#include <iostream>
#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/rasterizer.hpp>

//===========================================================================//
//   U I - B I N D I N G
//===========================================================================//

class UIBindingImp : public lx0::UIBinding
{
public:
    virtual void updateFrame (lx0::ViewPtr spView, const lx0::KeyboardState& keyboard)
    {
        if (keyboard.bDown[lx0::KC_ESCAPE])
            lx0::Engine::acquire()->sendEvent("quit");
        if (keyboard.bDown[lx0::KC_R])
            spView->sendEvent("redraw");
    }
};

//===========================================================================//
//   R E N D E R E R
//===========================================================================//

class Renderer : public lx0::View::Component
{
public:
    virtual void initialize(lx0::ViewPtr spView)
    {
        //
        // Initialize the rasterizer subsystem as soon as the OpenGL context is
        // available.
        //
        mspRasterizer.reset( new lx0::RasterizerGL );
        mspRasterizer->initialize();

        //
        // Create a camera
        // 
        glm::mat4 viewMatrix = glm::lookAt(glm::vec3(1, -2, 1.5f), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
        mspCamera = mspRasterizer->createCamera(60.0f, 0.01f, 1000.0f, viewMatrix);

        //
        // Build the cube geometry
        //
        std::vector<glgeom::point3f> positions(8);
        positions[0] = glgeom::point3f(-.5f,-.5f,-.5f);
        positions[1] = glgeom::point3f( .5f,-.5f,-.5f);
        positions[2] = glgeom::point3f(-.5f, .5f,-.5f);
        positions[3] = glgeom::point3f( .5f, .5f,-.5f);
        positions[4] = glgeom::point3f(-.5f,-.5f, .5f);
        positions[5] = glgeom::point3f( .5f,-.5f, .5f);
        positions[6] = glgeom::point3f(-.5f, .5f, .5f);
        positions[7] = glgeom::point3f( .5f, .5f, .5f);
        
        std::vector<lx0::uint16> indices;
        indices.reserve(4 * 6);
        auto push_face = [&indices](lx0::uint8 i0, lx0::uint8 i1, lx0::uint8 i2, lx0::uint8 i3) {
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        };
        push_face(0, 2, 3, 1);      // -Z face
        push_face(4, 5, 7, 6);      // +Z face
        push_face(0, 4, 6, 2);      // -X face
        push_face(1, 3, 7, 5);      // +X face
        push_face(0, 1, 5, 4);      // -Y face
        push_face(2, 6, 7, 3);      // +Y face

        //
        // Create indexed geometry
        // Channels such as normals, per-vertex color, uv coordinates, etc. are optional
        //
        lx0::GeometryPtr spCube = mspRasterizer->createQuadList(indices, positions);

        //
        // Create the material
        //
        std::string filename = lx0::Engine::acquire()->globals().find("shader_filename").as<std::string>();
        lx0::MaterialPtr spMaterial = mspRasterizer->createMaterial(filename.c_str());

        //
        // Build the cube renderable
        //
        mspItem.reset(new lx0::Item);
        mspItem->spTransform = mspRasterizer->createTransform(mRotation);
        mspItem->spMaterial = spMaterial;
        mspItem->spGeometry = spCube;
    }

    virtual void render (void)	
    {
        lx0::RenderAlgorithm algorithm;
        algorithm.mClearColor = glgeom::color4f(0.1f, 0.3f, 0.8f, 1.0f);
        
        lx0::GlobalPass pass;
        pass.spCamera = mspCamera;
        algorithm.mPasses.push_back(pass);

        lx0::RenderList items;
        items.push_back(0, mspItem);

        mspRasterizer->beginFrame(algorithm);
        for (auto it = items.begin(); it != items.end(); ++it)
        {
            mspRasterizer->rasterizeList(algorithm, it->second.list);
        }
        mspRasterizer->endFrame();
    }

    virtual void update (lx0::ViewPtr spView) 
    {
        mRotation = glm::rotate(mRotation, 1.0f, glm::vec3(0, 0, 1));
        mspItem->spTransform = mspRasterizer->createTransform(mRotation);

        spView->sendEvent("redraw");
    }

protected:
    lx0::RasterizerGLPtr mspRasterizer;
    lx0::CameraPtr       mspCamera;
    lx0::ItemPtr         mspItem;
    glm::mat4            mRotation;
};

//===========================================================================//

//
// See http://www.boost.org/doc/libs/1_44_0/doc/html/program_options/tutorial.html
//
static bool 
parse_options (lx0::EnginePtr spEngine, int argc, char** argv)
{
    //
    // Set up the command-line options data structure
    //
    using namespace boost::program_options;
    

    std::string caption = boost::str( boost::format("Syntax: %1% [options] <file>.\nOptions") % argv[0] );
    options_description desc (caption);
    desc.add_options()
        ("help", "Print usage information and exit.")
        ("shader_filename", value<std::string>(), "Fragment shader to use for the cube.")
        ;

    //
    // Parse the options
    //
    variables_map vars;
    store(command_line_parser(argc, argv).options(desc).run(), vars);

    //
    // Now check the options for anything that might prevent execution 
    //
    if (vars.count("help"))
    {
        std::cout << desc << std::endl;
        return false;
    }
    
    if (vars.count("shader_filename") == 1)
        spEngine->globals()["shader_filename"] = vars["shader_filename"].as<std::string>();

    return true;
}

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        lx0::EnginePtr spEngine = lx0::Engine::acquire();
        spEngine->globals().add("shader_filename", 0, lx0::validate_string(), "media2/shaders/glsl/fragment/normal.frag");

        if (parse_options(spEngine, argc, argv))
        {
            lx0::DocumentPtr spDocument = spEngine->createDocument();

            lx0::ViewPtr spView = spDocument->createView("Canvas", "view", new Renderer );
            spView->addUIBinding( new UIBindingImp );

            lx0::lxvar options;
            options.insert("title", "Tutorial 02");
            options.insert("width", 640);
            options.insert("height", 480);
            spView->show(options);

            exitCode = spEngine->run();
        }
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled std::exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
