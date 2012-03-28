//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2012 athile@athile.net (http://www.athile.net)

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

#include <boost/algorithm/string.hpp>

#include <glgeom/ext/primitive_buffer.hpp>

#include <lx0/lxengine.hpp>
#include <lx0/extensions/rasterizer.hpp>
#include <lx0/subsystem/shaderbuilder.hpp>
#include <lx0/subsystem/javascript.hpp>
#include <lx0/util/blendload.hpp>
#include <lx0/util/misc.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>

//===========================================================================//
//   P R O F I L I N G   D A T A
//===========================================================================//
/*
    The LxEngine profiling system is intended for coarse-grain profiling,
    as it is simple and straightforward but introduces a non-neglible amount
    of overhead.  

    The counters are created and identified by integer ids.  These counters
    are thread-safe and will report time spent in a per thread basis.  The
    counters alternately can be identified by their string name: this is
    useful for identifying counters registered in different modules (where
    access to the integer id may not be available).

    The counters need to be registered before they are used in order to 
    operate correctly.
 */
namespace 
{
    struct Profile
    {
        Profile() { ::memset(this, 0, sizeof(*this)); }
                    
        int     render;
        int     update;
        
        void initialize()
        {
            auto pEngine = lx0::Engine::acquire().get();

            //
            // Register the counters.  This must be done before the lx0::ProfileSection
            // class can be used to record times for a section of code with that 
            // counter.
            //
            pEngine->registerProfileCounter("Renderer render",        &render);            
            pEngine->registerProfileCounter("Renderer update",        &update);            

            //
            // Add correlations to the profile.  The list of built-in counter names
            // can be determined by looking at the contents of lxprofile.log after
            // running an application.
            //
            pEngine->addProfileRelationship("Engine runLoop", "Renderer render");
            pEngine->addProfileRelationship("Engine runLoop", "Renderer update");
        }
    } profile;
}

//===========================================================================//
//   G E O M E T R Y D A T A
//===========================================================================//
/*
    A standard LxEngine Geometry object coupled with additional
    data that can be specified in the XML file.
 */
struct GeometryData
{
    lx0::GeometryPtr    spGeometry;
    float               zoom;
};

//===========================================================================//
//   R E N D E R E R
//===========================================================================//

/*
    The main plug-in used by the tutorial.  
    
    The Renderer is a View::Component which responds to redraw requests, 
    frame updates, events, as well as other standard Component notifications.
    This is where we define how to redraw the screen, how to react to events
    (e.g. key presses), and the majority of the application logic.
 */
class Renderer : public lx0::View::Component
{
public:
    Renderer()
        : mCurrentMaterial    (0)
        , mCurrentGeometry    (0)
        , mbRotate            (true)
        , mCurrentZoom        (1.0f)
        , miRenderAlgorithm   (0)
    {
        //
        // The profile counters need to be initialized prior to use.  This
        // is a good place to do so as the constructor will always be called
        // before any of the counters within the profile object are used.
        //
        profile.initialize();
    }

    virtual void initialize (lx0::ViewPtr spView)
    {
        //
        // Initialize the rasterizer subsystem as soon as the OpenGL context is
        // available.
        //
        mspRasterizer.reset( new lx0::RasterizerGL );
        mspRasterizer->initialize();

        //
        // Create a couple offscreen frame buffers to use with the multipass
        // rendering algorithms.
        // 
        int width = spView->width();
        int height = spView->height();
        mspFBOffscreen0 = mspRasterizer->createFrameBuffer(width, height);
        mspFBOffscreen1 = mspRasterizer->createFrameBuffer(width, height);

        //
        // Add an empty light set; the Document will populate it with
        // individual lights.
        // 
        mspLightSet = mspRasterizer->createLightSet();

        //
        // Process the data in the document being viewed
        // 
        _processDocument( spView->document() );
        
        lx_check_error( !mMaterials.empty() );
        lx_check_error( !mGeometry.empty() );

        //
        // Build the instance.
        //
        // This is bundle of geometry, material, and transform to be rendered.  
        // The material and geometry pointers will be updated as the user cycles
        // through the array of settings loaded by the document.
        //
        mspInstance.reset(new lx0::Instance);
        mspInstance->spTransform = mspRasterizer->createTransform(mRotation);
        mspInstance->spMaterial = mMaterials[mCurrentMaterial];
        mspInstance->spGeometry = mGeometry[mCurrentGeometry].spGeometry;

        //
        // Create the camera last since it is dependent on the bounds of the geometry
        // being viewed.  Therefore, it needs to be created after the geometry is 
        // loaded.
        // 
        auto& geomData = mGeometry[mCurrentGeometry];
        mCurrentZoom = geomData.zoom;
        mspCamera = _createCamera(geomData.spGeometry->mBBox, mCurrentZoom);
    }

    /*
        The View::Component render virtual method is called when the view 
        needs to be redrawn.
     */
    virtual void render (void)	
    {
        lx0::ProfileSection section(profile.render);

        //
        // The RenderAlgorithm describes number of global scene passes and
        // as well as setup data such as the clear color.  Depending on the
        // activate requested algorithm, populate this data structure with
        // the appropriate parameters.
        //
        lx0::RenderAlgorithm algorithm;
        algorithm.mClearColor = glgeom::color4f(0.1f, 0.3f, 0.8f, 1.0f);
        
        lx0::GlobalPass pass;
        switch (miRenderAlgorithm % 5)
        {
        default:
        case 0:
            //
            // Set up a "default" rendering algorithm.  A camera and light set
            // are all that are provided: the algorithm will default to rendering
            // to the screen and the shader/material data will all come from 
            // the individual entities.
            //
            pass.spCamera   = mspCamera;
            pass.spLightSet = mspLightSet;
            algorithm.mPasses.push_back(pass);
            break;

        case 1:
            //
            // Set up a Gaussian blur via a three-pass render.  The first
            // pass will draw the scene normally but to an offscreen frame
            // buffer (FBO 0). The second pass will blit the result of the first
            // pass (FBO 0) to a second offscreen frame buffer (FBO 1) and apply 
            /// the first of the Gaussian blur shaders (a vertical blur).  The 
            // final, third pass will blit the result of the second pass (FBO 1)
            // to the display and apply the second Gaussian blur shader (a
            // horizontal blur).
            //
            
            // Draw Scene -> FBO 0
            pass.spFrameBuffer = mspFBOffscreen0;
            pass.spCamera   = mspCamera;
            pass.spLightSet = mspLightSet;
            algorithm.mPasses.push_back(pass);

            // Blit FBO 0 -> FBO 1
            pass.spFrameBuffer = mspFBOffscreen1;
            pass.spSourceFBO = mspFBOffscreen0;
            pass.spMaterial = mspRasterizer->acquireMaterial("BlitFBOGaussianPass1");
            algorithm.mPasses.push_back(pass);

            // Blit FBO 1 -> Display
            pass.spFrameBuffer.reset();             
            pass.spSourceFBO = mspFBOffscreen1;
            pass.spMaterial = mspRasterizer->acquireMaterial("BlitFBOGaussianPass2");
            algorithm.mPasses.push_back(pass);
            break;

        case 2:
            //
            // Set up a two pass grayscale render algorithm: draw the scene normally
            // to an offscreen FBO, then blit that FBO to the display using a 
            // shader that will transform all colors to their grayscale equivalent.
            //
            pass.spFrameBuffer = mspFBOffscreen0;
            pass.spCamera   = mspCamera;
            pass.spLightSet = mspLightSet;
            pass.optClearColor = std::make_pair(true, glgeom::color4f(0, 0, 0, 0));
            algorithm.mPasses.push_back(pass);

            pass.spFrameBuffer.reset();
            pass.spSourceFBO = mspFBOffscreen0;
            pass.spMaterial = mspRasterizer->acquireMaterial("BlitFBOGrayscale");
            algorithm.mPasses.push_back(pass);
            break;

        case 3:
            //
            // A similar two pass render algorithm: draw the scene normally to an
            // offscreen FBO, then blit the FBO to the display using a pixel shader
            // which will invert the RGB value.
            //
            pass.spFrameBuffer = mspFBOffscreen0;
            pass.spCamera   = mspCamera;
            pass.spLightSet = mspLightSet;
            pass.optClearColor = std::make_pair(true, glgeom::color4f(0, 0, 0, 0));
            algorithm.mPasses.push_back(pass);

            pass.spFrameBuffer.reset();
            pass.spSourceFBO = mspFBOffscreen0;
            pass.spMaterial = mspRasterizer->acquireMaterial("BlitFBOInvert");
            algorithm.mPasses.push_back(pass);
            break;

        case 4:
            //
            // Similar to the the grayscale and invert algorithms, this two pass
            // algorithm differs only by the pixel shader used: this one will apply
            // a color inversion and use a box blur filter (that varies in size based 
            // on distance from the viewport center) on the image.
            //
            pass.spFrameBuffer = mspFBOffscreen0;
            pass.spCamera   = mspCamera;
            pass.spLightSet = mspLightSet;
            pass.optClearColor = std::make_pair(true, glgeom::color4f(0, 0, 0, 0));
            algorithm.mPasses.push_back(pass);

            pass.spFrameBuffer.reset();
            pass.spSourceFBO = mspFBOffscreen0;
            pass.spMaterial = mspRasterizer->acquireMaterial("BlitFBOInvertBlur");
            algorithm.mPasses.push_back(pass);
            break;
        }

        //
        // The scene is described via a RenderList, which (for our purposes) is
        // a simple list of Instance objects.  The Instance objects describe the
        // individual entities as a set of geometry, material, and any custom 
        // settings or overrides for that entity.
        //
        lx0::RenderList instances;
        instances.push_back(0, mspInstance);

        //
        // This is a standard rendering loop: begin the frame, then rasterize
        // each layer of the RenderList according via rasterizeList.
        //
        mspRasterizer->beginFrame(algorithm);
        for (auto it = instances.begin(); it != instances.end(); ++it)
        {
            mspRasterizer->rasterizeList(algorithm, it->second.list);
        }
        mspRasterizer->endFrame();
    }

    /*
        Called every frame to give the renderer an opportunity to do 
        any data update such as simulations, animations, or other 
        incremental data updates.
     */
    virtual void 
    update (lx0::ViewPtr spView) 
    {
        lx0::ProfileSection section(profile.update);

        //
        // Rotate the model on every update, if that option is enabled.
        //
        if (mbRotate)
        {
            mRotation = glm::rotate(mRotation, 1.0f, glm::vec3(0, 0, 1));
            mspInstance->spTransform = mspRasterizer->createTransform(mRotation);
        }

        //
        // Check if new geometry has loaded and update the active instance.  
        // Recreate the camera as well since the bounds will have changed.
        //
        auto& geomData = mGeometry[mCurrentGeometry];
        if (mGeometry[mCurrentGeometry].spGeometry.get() != mspInstance->spGeometry.get())
        {
            mspInstance->spGeometry = mGeometry[mCurrentGeometry].spGeometry;
            mspCamera = _createCamera(geomData.spGeometry->mBBox, mCurrentZoom);
        }

        //
        // If the zoom value on the geometry has changed, adjust the
        // camera view accordingly.
        //        
        if (mCurrentZoom != geomData.zoom)
        {
            mCurrentZoom = geomData.zoom;
            mspCamera = _createCamera(geomData.spGeometry->mBBox, mCurrentZoom);
        }

        //
        // Views are not necessarily redrawn every frame; the application must indicate
        // whether a redraw is actually needed.  This tutorial forces a redraw every
        // frame by always sending the redraw message to the view with every update.
        //
        spView->sendEvent("redraw");
    }

    /*
        LxEngine abstracts direct user input and commands from the logic to handle
        those interactions.  The keyboard logic, for example, translates key presses
        into high-level "events."  This way the application logic responds to 
        events such as "next_material" rather than a specific key press such as 
        "N" meaning switch to the next material.  This abstraction allows for easy
        customization of keyboard shortcuts as well as allows for thing such as 
        scripted interaction (i.e. simulating a keypress) without any changes to
        the internal handling logic.
     */
    virtual void 
    handleEvent (std::string evt, lx0::lxvar params) 
    {
        if (evt == "change_geometry")
        {
            mCurrentGeometry = (params == "next") 
                ? (mCurrentGeometry + 1)
                : (mCurrentGeometry + mGeometry.size() - 1);
            mCurrentGeometry %= mGeometry.size();
            
            auto& geomData = mGeometry[mCurrentGeometry];
            mspInstance->spGeometry = geomData.spGeometry;
            
            //
            // Recreate the camera after geometry changes since the camera position
            // is based on the bounds of the geometry being viewed.
            //
            mCurrentZoom = geomData.zoom;
            mspCamera = _createCamera(geomData.spGeometry->mBBox, mCurrentZoom);
        }
        else if (evt == "next_material" || evt == "prev_material")
        {
            mCurrentMaterial = (evt == "next_material")
                ? (mCurrentMaterial + 1)
                : (mCurrentMaterial + mMaterials.size() - 1);
            mCurrentMaterial %= mMaterials.size();

            mspInstance->spMaterial = mMaterials[mCurrentMaterial];
        }
        else if (evt == "toggle_rotation")
        {
            mbRotate = !mbRotate;
        }
        else if (evt == "toggle_wireframe")
        {
            //@todo This should be handled as a global rendering algorithm override instead
            for (auto it = mMaterials.begin(); it != mMaterials.end(); ++it)
                (*it)->mWireframe = !(*it)->mWireframe;
        }
        else if (evt == "cycle_renderalgorithm")
        {
            miRenderAlgorithm++;
        }
        else if (evt == "zoom_in")
        {
            _handleEventZoom(true);
        }
        else if (evt == "zoom_out")
        {
            _handleEventZoom(false);
        }
        else if (evt == "cancel_event")
        {
            mEventHandle.reset();
        }
    }

protected:

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
            _processGeometry(spDocument, *it);

        // And once more for <Light> elements
        //
        auto vLights = spDocument->getElementsByTagName("Light");
        for (auto it = vLights.begin(); it != vLights.end(); ++it)
            _processLight(*it);
    }

    void _processMaterial (lx0::ElementPtr spElem)
    {
        lx0::lxvar  render = spElem->value().is_defined() ? spElem->value().find("render") : lx0::lxvar::undefined();
        lx0::lxvar  graph = spElem->value().is_defined() ? spElem->value().find("graph") : lx0::lxvar::undefined();

        if (graph.is_defined())
        {
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
            _addMaterial(material.uniqueName, material.source, material.parameters, render);
        }
        else
        {
            std::string src = spElem->attr("src").query(std::string(""));
            std::string instance = spElem->attr("instance").query(std::string(""));

            auto spMaterial = mspRasterizer->acquireMaterial(src, instance);
            mMaterials.push_back(spMaterial);
        }
    }

    void _processGeometry (lx0::DocumentPtr spDocument, lx0::ElementPtr spElem)
    {
        //
        // Extract the data from the DOM
        //
        std::string sourceFilename = spElem->attr("src").as<std::string>();
        float       zoom           = spElem->attr("zoom").query<float>(1.0f);
        _addGeometry(spDocument, sourceFilename, zoom);
    }

    void _processLight (lx0::ElementPtr spElem)
    {
        //
        // Extract the data from the DOM and push it to the light set
        //
        lx0::LightPtr spLight = mspRasterizer->createLight();
        spLight->position  = spElem->value()["position"].convert();
        spLight->color     = spElem->value()["color"].convert();
        mspLightSet->mLights.push_back(spLight);
    }

    //
    // Creates a camera with fixed view direction and a view distance determined by
    // the visibility bounds.
    //
    lx0::CameraPtr _createCamera (const glgeom::abbox3f& bbox, float zoomFactor)
    {
        lx_check_error(!bbox.empty());

        const glgeom::vector3f viewDirection(-1, 2, -1.5f);
        const float            viewDistance (bbox.diagonal() * .9f / zoomFactor);

        glgeom::point3f viewPoint  = glgeom::point3f(0, 0, 0) - glgeom::normalize(viewDirection) * viewDistance; 
        glm::mat4       viewMatrix = glm::lookAt(viewPoint.vec, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
            
        return mspRasterizer->createCamera(glgeom::pi() / 3.0f, 0.01f, 1000.0f, viewMatrix);
    }

    void _addMaterial (std::string uniqueName, std::string source, lx0::lxvar parameters, lx0::lxvar render)
    {
        lx0::lxvar params2;
        for (auto it = parameters.begin(); it != parameters.end(); ++it)
        {
            params2[it.key()] = (*it)[1].clone();
        }

        auto spMaterial = mspRasterizer->createMaterial(uniqueName, source, params2);
        mMaterials.push_back(spMaterial);
    }

    void
    _addGeometryBlender (lx0::DocumentPtr spDocument, const std::string filename, int index)
    {
        //
        // This is executed as a task in the main thread; therefore no locks or threading 
        // protection is necessary on the rasterizer or geometry list.  If this were
        // executed outside the main thread, then the calls to createQuadList and the
        // modification to mGeometry would be problematic.
        //
        auto addGeometry = [this,index](glgeom::primitive_buffer* primitive) {
                auto spGeometry = mspRasterizer->createQuadList(primitive->indices, 
                                                                primitive->face.flags, 
                                                                primitive->vertex.positions, 
                                                                primitive->vertex.normals, 
                                                                primitive->vertex.colors);
                spGeometry->mBBox = primitive->bbox;
                mGeometry[index].spGeometry = spGeometry;
                delete primitive;
        };

        //
        // The geometry loading uses only local data and therefore can safely be executed
        // in a separate thread.
        //
        auto loadGeometry = [this,filename,addGeometry]() {

            lx0::Timer timer;
            if (!lx0::Engine::acquire()->isShuttingDown())
            {
                lx0::TimeSection section(timer);
                lx_message("Loading Blender model '%1%'", filename);
                glgeom::primitive_buffer* primitive = new glgeom::primitive_buffer;
                glm::mat4 scaleMat = glm::scale(glm::mat4(), glm::vec3(1, 1, 1));
                lx0::primitive_buffer_from_blendfile(*primitive, filename.c_str(), scaleMat);
                    
                auto f = addGeometry;
                if (!lx0::Engine::acquire()->isShuttingDown())
                    lx0::Engine::acquire()->sendTask([primitive,f](){ f(primitive); });
            }
            lx_message("Model loaded in %2%ms", filename, timer.totalMs());
        };

        lx0::Engine::acquire()->sendWorkerTask(loadGeometry);
    }

    void
    _addGeometryJavascript (lx0::DocumentPtr spDocument, const std::string filename, int index)
    {
        //
        // Add the script execution as task in the Engine queue.  This will be executed
        // once the Engine::run() loop begins.  There's not much advantage or necessity to 
        // queuing the script execution rather than running it directly; this is being
        // done mostly demonstrate using the sendTask() method.
        //
        lx0::Engine::acquire()->sendTask([this,spDocument,filename,index]() {

            lx_message("Loading geometry script '%1%'", filename);
            auto spJavascriptDoc = spDocument->getComponent<lx0::IJavascriptDoc>();
            
            //
            // The result of the script is converted to an lxvar as an intermediate step.  This is
            // a bit inefficient, but flexible and convenient as a common intermediary.
            //
            auto source = lx0::string_from_file(filename);     
            lx0::lxvar result = spJavascriptDoc->run(source);

            if (result.is_defined())
            {
                glgeom::primitive_buffer primitive = result.convert();
                glgeom::compute_bounds(primitive, primitive.bbox);               

                auto spGeometry = mspRasterizer->createGeometry(primitive);
                spGeometry->mBBox = primitive.bbox;
                mGeometry[index].spGeometry = spGeometry;
            }
            else
                throw lx_error_exception("Script failure!");
        });
    }

    void 
    _addGeometry (lx0::DocumentPtr spDocument, const std::string filename, float zoom)
    {
        //
        // Insert a stub item into the geometry index.  This will be replaced by
        // the real geometry as soon as the geometry is loaded.
        //
        auto index = mGeometry.size();
        mGeometry.push_back(GeometryData());
        mGeometry[index].zoom = zoom;            

        mGeometry[index].spGeometry = mspRasterizer->acquireGeometry("basic2d/Empty");
        glgeom::abbox3f bbox;
        bbox.min = glgeom::point3f(-1, -1, -1);
        bbox.max = glgeom::point3f( 1,  1,  1);
        mGeometry[index].spGeometry->mBBox = bbox;

        if (boost::iends_with(filename, ".blend"))
            _addGeometryBlender(spDocument, filename, index);

        else if (boost::iends_with(filename, ".js"))
            _addGeometryJavascript(spDocument, filename, index);
        
        else
            throw lx_error_exception("Unrecognized geometry source!");
    }

    void
    _handleEventZoom (bool bZoomIn)
    {
        //
        // Parameters for the zoom effect: how much are we changing the zoom
        // factor and how long should the view transition take?
        //
        float zoomFactor = bZoomIn ? 2.0f : 0.5f;
        float intervalMs = 750.0f;

        //
        // Create a lambda function that will incrementally zoom in or out
        // over a given time period.
        //            
        int         geometryIndex = mCurrentGeometry;
        float       startValue    = mGeometry[geometryIndex].zoom;
        float       endValue      = startValue * zoomFactor;
        lx0::uint32 startTime     = lx0::lx_milliseconds();

        //
        // Apply a sinusoidal smooth (ease-in/ease-out) to the alpha value
        // for a smoother animation.  If the set interval time has expired
        // then end the event reoccurence by setting the final value exactly 
        // and returning -1 (i.e. the time delay before rerunning the event).
        //
        auto func = [this, geometryIndex, startValue, endValue, startTime, intervalMs]() -> int {
            
            // Grab the refernece to the zoom variable every time since the 
            // mGeometry vector may need to reallocate as new geometry is added.
            float& zoom  = mGeometry[geometryIndex].zoom;

            float  alpha = float(lx0::lx_milliseconds() - startTime) / intervalMs;
            if (!(alpha >= 1.0))
            {
                zoom = glm::mix(startValue, endValue, glgeom::sinusodial_smooth(alpha));
                return 0;
            }
            else
            {
                zoom = endValue;
                return -1;
            }
        };
            
        //
        // Add the function as an event in the event queue and record an event
        // handle.  The mEventHandle is used to cancel pending events; namely
        // if the handle is reassigned or reset() is called on the handle before
        // the event is done, the event will not be executed.  In this case,
        // the same event handle is used for all view changes, thus cancelling
        // any in-progress view changes when a new one is requested (i.e. that's
        // the desired behavior we want.)
        //
        lx0::Engine::acquire()->sendEvent(func, mEventHandle);
    }

    // Rasterizer objects
    lx0::ShaderBuilder            mShaderBuilder;
    lx0::RasterizerGLPtr          mspRasterizer;
    lx0::FrameBufferPtr           mspFBOffscreen0;
    lx0::FrameBufferPtr           mspFBOffscreen1;
    lx0::CameraPtr                mspCamera;
    lx0::LightSetPtr              mspLightSet;
    lx0::InstancePtr              mspInstance;
    
    // Current settings
    bool                          mbRotate;
    glm::mat4                     mRotation;
    float                         mCurrentZoom;
    int                           miRenderAlgorithm;
    size_t                        mCurrentMaterial;
    size_t                        mCurrentGeometry;

    // Application data
    lx0::EventHandle              mEventHandle;
    std::vector<lx0::MaterialPtr> mMaterials;
    std::vector<GeometryData>     mGeometry;
};

//===========================================================================//
//   P L U G - I N   E N T R Y P O I N T
//===========================================================================//

extern "C" _declspec(dllexport) void initializePlugin()
{
    //
    // Assign the class a name (so that scripts and other code have a way to
    // refer to it) and an associated function for creating an instance of
    // the class.
    //
    auto spEngine = lx0::Engine::acquire();   
    spEngine->registerViewComponent("Renderer", []() { return new Renderer; });
}
