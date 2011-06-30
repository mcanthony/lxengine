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

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/rasterizer.hpp>
#include <lx0/util/blendload.hpp>

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
        lx0::EnginePtr spEngine = lx0::Engine::acquire();

        //
        // Initialize the rasterizer subsystem as soon as the OpenGL context is
        // available.
        //
        mspRasterizer.reset( new lx0::RasterizerGL );
        mspRasterizer->initialize();

        //
        // Create geometry
        //
        std::string modelFilename = spEngine->globals().find("model_filename").as<std::string>();
        glgeom::abbox3f bbox;
        lx0::GeometryPtr spModel = lx0::geometry_from_blendfile(mspRasterizer, modelFilename.c_str(), bbox);

        //
        // Create a camera
        // 
        glgeom::vector3f viewDirection(-1, 2, -1.5f);
        float viewDistance = bbox.diagonal() * 1.5f;
        glgeom::point3f viewPoint = glgeom::point3f(0, 0, 0) - glgeom::normalize(viewDirection) * viewDistance; 
        glm::mat4 viewMatrix = glm::lookAt(viewPoint.vec, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
        mspCamera = mspRasterizer->createCamera(60.0f, 0.01f, 1000.0f, viewMatrix);

        //
        // Create the material
        //
        std::string shaderFilename = spEngine->globals().find("shader_filename").as<std::string>();
        lx0::MaterialPtr spMaterial = mspRasterizer->createMaterial(shaderFilename.c_str());

        //
        // Build the cube renderable
        //
        mspItem.reset(new lx0::Item);
        mspItem->spTransform = mspRasterizer->createTransform(mRotation);
        mspItem->spMaterial = spMaterial;
        mspItem->spGeometry = spModel;
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
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        lx0::EnginePtr spEngine = lx0::Engine::acquire();
        spEngine->globals().add("shader_filename",  lx0::eAcceptsString, lx0::validate_string(),             "media2/shaders/glsl/fragment/normal.frag");
        spEngine->globals().add("model_filename",   lx0::eAcceptsString, lx0::validate_string(),             "media2/models/unit_cube-000.blend");
        spEngine->globals().add("view_width",       lx0::eAcceptsInt,    lx0::validate_int_range(32, 4096),  512);
        spEngine->globals().add("view_height",      lx0::eAcceptsInt,    lx0::validate_int_range(32, 4096),  512);

        if (spEngine->parseCommandLine(argc, argv))
        {
            lx0::DocumentPtr spDocument = spEngine->createDocument();

            lx0::ViewPtr spView = spDocument->createView("Canvas", "view", new Renderer );
            spView->addUIBinding( new UIBindingImp );

            lx0::lxvar options;
            options.insert("title", "Tutorial 02");
            options.insert("width", spEngine->globals()["view_width"]);
            options.insert("height", spEngine->globals()["view_height"]);
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
