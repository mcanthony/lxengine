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

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/rasterizer.hpp>
#include <lx0/subsystem/shaderbuilder.hpp>
#include <lx0/util/blendload.hpp>

//===========================================================================//
//   U I - B I N D I N G
//===========================================================================//

//
// The UIBinding is not intended to do much processing itself.  It should
// map device-events into high-level application events.
//
class UIBindingImp : public lx0::UIBinding
{
public:
    virtual void onKeyDown (lx0::ViewPtr spView, int keyCode) 
    { 
        if (keyCode == lx0::KC_G)
            spView->sendEvent("change_geometry", "next");
        if (keyCode == lx0::KC_F)
            spView->sendEvent("change_geometry", "prev");
        if (keyCode == lx0::KC_M)
            spView->sendEvent("next_material");
        if (keyCode == lx0::KC_N)
            spView->sendEvent("prev_material");
    }

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
    Renderer()
        : mCurrentMaterial    (0)
        , mCurrentGeometry    (0)
    {
    }

    virtual void initialize(lx0::ViewPtr spView)
    {
        //
        // Initialize the rasterizer subsystem as soon as the OpenGL context is
        // available.
        //
        mspRasterizer.reset( new lx0::RasterizerGL );
        mspRasterizer->initialize();

        //
        // Process the data in the document being viewed
        // 
        _processConfiguration();
        _processDocument( spView->document() );
        
        lx_check_error( !mMaterials.empty() );
        lx_check_error( !mGeometry.empty() );

        //
        // Create a set of lights
        // 
        lx0::LightPtr spLight0 = mspRasterizer->createLight();
        spLight0->position = glgeom::point3f(10, -10, 10);
        spLight0->color    = glgeom::color3f(1, 1, 1);
        
        lx0::LightPtr spLight1 = mspRasterizer->createLight();
        spLight0->position = glgeom::point3f(10, 10, 10);
        spLight0->color    = glgeom::color3f(.6f, .6f, 1);
        
        mspLightSet = mspRasterizer->createLightSet();
        mspLightSet->mLights.push_back(spLight0);
        mspLightSet->mLights.push_back(spLight1);

        //
        // Build the renderable
        //
        mspInstance.reset(new lx0::Instance);
        mspInstance->spTransform = mspRasterizer->createTransform(mRotation);
        mspInstance->spMaterial = mMaterials[mCurrentMaterial];
        mspInstance->spGeometry = mGeometry[mCurrentGeometry];

        //
        // Create the camera last since it is dependent on the bounds of the geometry
        // being viewed.  Therefore, it needs to be created after the geometry is 
        // loaded.
        // 
        mspCamera = _createCamera(mGeometry[mCurrentGeometry]->mBBox);
    }

    virtual void render (void)	
    {
        lx0::RenderAlgorithm algorithm;
        algorithm.mClearColor = glgeom::color4f(0.1f, 0.3f, 0.8f, 1.0f);
        
        lx0::GlobalPass pass;
        pass.spCamera   = mspCamera;
        pass.spLightSet = mspLightSet;
        algorithm.mPasses.push_back(pass);

        lx0::RenderList instances;
        instances.push_back(0, mspInstance);

        mspRasterizer->beginFrame(algorithm);
        for (auto it = instances.begin(); it != instances.end(); ++it)
        {
            mspRasterizer->rasterizeList(algorithm, it->second.list);
        }
        mspRasterizer->endFrame();
    }

    virtual void update (lx0::ViewPtr spView) 
    {
        //
        // Always rotate the model on every update
        //
        mRotation = glm::rotate(mRotation, 1.0f, glm::vec3(0, 0, 1));
        mspInstance->spTransform = mspRasterizer->createTransform(mRotation);

        spView->sendEvent("redraw");
    }

    virtual void handleEvent (std::string evt, lx0::lxvar params) 
    {
        if (evt == "change_geometry")
        {
            mCurrentGeometry = (params == "next") 
                ? (mCurrentGeometry + 1)
                : (mCurrentGeometry + mGeometry.size() - 1);
            mCurrentGeometry %= mGeometry.size();
            
            mspInstance->spGeometry = mGeometry[mCurrentGeometry];
            
            //
            // Recreate the camera after geometry changes since the camera position
            // is based on the bounds of the geometry being viewed.
            //
            mspCamera           = _createCamera(mspInstance->spGeometry->mBBox);
        }
        else if (evt == "next_material" || evt == "prev_material")
        {
            mCurrentMaterial = (evt == "next_material")
                ? (mCurrentMaterial + 1)
                : (mCurrentMaterial + mMaterials.size() - 1);
            mCurrentMaterial %= mMaterials.size();

            mspInstance->spMaterial = mMaterials[mCurrentMaterial];
        }
    }

protected:
    void _processConfiguration (void)
    {
        //
        // Check the global configuration
        //
        lx0::lxvar config = lx0::Engine::acquire()->globals();

        if (config.find("model_filename").is_defined())
        {
            _addGeometry(config["model_filename"]);
        }

        if (config.find("shader_filename").is_defined())
        {
            std::string source     = lx0::string_from_file(config["shader_filename"]);
            lx0::lxvar  parameters = config.find("params_filename").is_defined() 
                ? lx0::lxvar_from_file( config["params_filename"] )
                : lx0::lxvar::undefined();

            _addMaterial("", source, parameters);
        }
    }

    void _processDocument (lx0::DocumentPtr spDocument)
    {
        // Find all the <Material> elements in the document and translate
        // them into runtime materials.
        //
        auto vMaterials = spDocument->getElementsByTagName("Material");
        for (auto it = vMaterials.begin(); it != vMaterials.end(); ++it)
            _processMaterial(*it);

        // Do the same for <Geometry> elements
        //
        auto vGeometry = spDocument->getElementsByTagName("Geometry");
        for (auto it = vGeometry.begin(); it != vGeometry.end(); ++it)
            _processGeometry(*it);
    }

    void _processMaterial (lx0::ElementPtr spElem)
    {
        lx0::lxvar  graph = spElem->value().find("graph");

        //
        // Use the Shader Builder subsystem to construct a material
        // (i.e. unique id, shader source code, and set of parameters)
        //
        lx0::ShaderBuilder::Material material;
        mShaderBuilder.buildShaderGLSL(material, graph);

        //
        // Pass on the generated material data which the rasterizer
        // will use to compile a shader plus a parameter set.
        //
        _addMaterial(material.uniqueName, material.source, material.parameters);
    }

    void _processGeometry (lx0::ElementPtr spElem)
    {
        //
        // Extract the data from the DOM
        //
        std::string sourceFilename = spElem->attr("src").as<std::string>();
        _addGeometry(sourceFilename);
    }


    //
    // Creates a camera with fixed view direction and a view distance determined by
    // the visibility bounds.
    //
    lx0::CameraPtr _createCamera (const glgeom::abbox3f& bbox)
    {
        const glgeom::vector3f viewDirection(-1, 2, -1.5f);
        const float            viewDistance (bbox.diagonal() * .9f);

        glgeom::point3f viewPoint  = glgeom::point3f(0, 0, 0) - glgeom::normalize(viewDirection) * viewDistance; 
        glm::mat4       viewMatrix = glm::lookAt(viewPoint.vec, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
            
        return mspRasterizer->createCamera(glgeom::pi() / 3.0f, 0.01f, 1000.0f, viewMatrix);
    }

    void _addMaterial (std::string uniqueName, std::string source, lx0::lxvar parameters)
    {
        lx0::MaterialPtr spMaterial = mspRasterizer->createMaterial(uniqueName, source, parameters);
        mMaterials.push_back(spMaterial);
    }

    void _addGeometry (const std::string& modelFilename)
    {
        std::cout << "Loading '" << modelFilename << "'" << std::endl;            
        lx0::GeometryPtr spModel = lx0::geometry_from_blendfile(mspRasterizer, modelFilename.c_str());
        mGeometry.push_back(spModel);
    }

    lx0::ShaderBuilder            mShaderBuilder;
    lx0::RasterizerGLPtr          mspRasterizer;
    lx0::CameraPtr                mspCamera;
    lx0::LightSetPtr              mspLightSet;
    lx0::InstancePtr                  mspInstance;

    glm::mat4                     mRotation;

    size_t                        mCurrentMaterial;
    std::vector<lx0::MaterialPtr> mMaterials;

    size_t                        mCurrentGeometry;
    std::vector<lx0::GeometryPtr> mGeometry;
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

        //
        // Add several global configuration variables.  These will be exported as 
        // command-line options.
        //
        spEngine->globals().add("shader_filename",  lx0::eAcceptsString, lx0::validate_filename(),           lx0::lxvar::undefined());
        spEngine->globals().add("params_filename",  lx0::eAcceptsString, lx0::validate_filename(),           lx0::lxvar::undefined());
        spEngine->globals().add("model_filename",   lx0::eAcceptsString, lx0::validate_filename(),           lx0::lxvar::undefined());
        spEngine->globals().add("view_width",       lx0::eAcceptsInt,    lx0::validate_int_range(32, 4096),  512);
        spEngine->globals().add("view_height",      lx0::eAcceptsInt,    lx0::validate_int_range(32, 4096),  512);

        //
        // Have the Engine parse the command-line, checking if any configuration options 
        // have been set.
        //
        if (spEngine->parseCommandLine(argc, argv))
        {
            lx0::DocumentPtr spDocument = spEngine->loadDocument("media2/appdata/tutorial_03/document.xml");

            lx0::ViewPtr spView = spDocument->createView("Canvas", "view", new Renderer );
            spView->addUIBinding( new UIBindingImp );

            lx0::lxvar options;
            options.insert("title", "Tutorial 03");
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
