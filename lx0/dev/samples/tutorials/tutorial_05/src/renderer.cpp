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
#include <boost/interprocess/detail/atomic.hpp>

#include <glgeom/extension/primitive_buffer.hpp>
#include <glgeom/extension/mappers.hpp>

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
//   C O R O U T I N E
//===========================================================================//

class Coroutine
{
public:
    Coroutine ()
    {
    }

    ~Coroutine ()
    {
    }

    void    delay   (unsigned int delay)                          { mSteps.push_back(Step(delay, [](){})); }
    void    main    (std::function<void()> f)                     { mSteps.push_back(Step(0, f)); }
    void    main_ret(std::function<int()> f)                      { mSteps.push_back(Step(f)); }
    void    main    (unsigned int delay, std::function<void()> f) { mSteps.push_back(Step(delay, f)); }
    void    worker  (std::function<void()> f)                     { mSteps.push_back(Step(-1, f)); }

    void    worker  (std::function<void()> f0, std::function<void()> f1)                     
    { 
        Step s;
        s.parallel.push_back(f0);
        s.parallel.push_back(f1);
        mSteps.push_back(s); 
    }

    std::function<void()> compile()                               
    { 
        return _chain(mSteps, mSteps.begin()); 
    }
        
protected:
    struct Step
    {
        Step () : delay (0) {}
        Step (unsigned int d, std::function<void()> f) : delay(d), func(f) {}
        Step (std::function<int()> f) : delay(0), func2(f) {}

        unsigned int                        delay;
        std::function<void()>               func;
        std::function<int()>                func2;
        std::vector<std::function<void()>>  parallel;
    };
    typedef std::vector<Step> StepList;
    StepList    mSteps;
 
    /*
        The function has two essential parts: (1) wrap the current step so that it 
        is invoked by the engine properly, (2) wrap the invokation of the _next_
        step is invoked automatically after the current step.

        FUTURE:

        "State" could be added to the coroutine, such as a active variable that 
        the coroutine can be paused/resumed.  This would involve more complex
        interaction with the Engine as a "pause" would need to prevent the next
        step from being added to any queue, while a "resume" would need to know
        what step (and how) to add back into a queue.  Possible, but let's wait
        for a good use case.
     */
    static std::function<void()> 
    _chain (StepList& c, StepList::iterator it)
    {
        if (it != c.end())
        {
            std::function<void()> g = _chain(c, it + 1);
            
            auto pEngine = lx0::Engine::acquire().get();     

            if (it->func)
            {
                auto f = it->func;
                auto i = it->delay; 
                auto h = [f,g]() { f(); g(); };
                              
                if (i == 0)
                    return [=]() { pEngine->sendTask(h); };
                else if (i > 0)
                    return [=]() { pEngine->sendTask(i, h); };
                else
                    return [=]() { pEngine->sendWorkerTask(h); };
            }
            else if (it->func2)
            {
                auto f = it->func2;
                auto h = [f,g]() -> int { 
                    int ret = f();
                    if (ret == 0)
                        g();
                    return ret;
                };
                return [=]() { pEngine->sendEvent(h); };
            }
            else if (!it->parallel.empty())
            {
                //
                // A set of tasks to run in parallel
                //
                auto fv = it->parallel;
                auto total = lx0::uint32( fv.size() );
                
                // Create a sync function to execute the next step (i.e. g) only
                // after all parallel functions have completed
                volatile lx0::uint32* pCount = new lx0::uint32(0);
                auto sync = [pCount,total,g]() { 
                    if (boost::interprocess::detail::atomic_inc32(pCount) == total - 1) {
                        g(); 
                        delete pCount;
                    }
                };

                // Wrap the parallel functions with the sync as a tail
                std::vector<std::function<void()>> hv;
                for (auto it = fv.begin(); it != fv.end(); ++it)
                {
                    auto f = *it;
                    hv.push_back([f,sync]() { f(); sync(); });
                }

                // Return a function to spawn the wrapped parallel functions
                return [pEngine,hv]() {
                    for (auto it = hv.begin(); it != hv.end(); ++it)
                        pEngine->sendWorkerTask(*it);
                };
            }
            else
                throw lx_error_exception("Empty event detected");
        }
        else
            return [](){};
    }
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
        mspFBOffscreen0 = mspRasterizer->acquireFrameBuffer("FB0", width, height);
        mspFBOffscreen1 = mspRasterizer->acquireFrameBuffer("FB1", width, height);

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
        
        //
        // Create the camera last since it is dependent on the bounds of the geometry
        // being viewed.  Therefore, it needs to be created after the geometry is 
        // loaded.
        // 
        mCurrentZoom = 1.0f;
        mspCamera = _createCamera(mInstances.front()->spGeometry->mBBox, mCurrentZoom);
    }

    /* 
        Add an instance to the scene.
     */
    void
    _addInstance (lx0::InstancePtr spInstance)
    {
        mInstances.push_back(spInstance);
        mSceneBounds.merge(spInstance->spGeometry->mBBox);
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
        switch (miRenderAlgorithm % 4)
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
            pass.spFrameBuffer.reset();
            pass.spSourceFBO = mspFBOffscreen0;
            pass.spMaterial = mspRasterizer->acquireMaterial("BlitFBOPixelate");
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
            pass.spMaterial = mspRasterizer->acquireMaterial("BlitFBODistortSinStripes");
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
        for (auto it = mInstances.begin(); it != mInstances.end(); ++it)
            instances.push_back(0, *it);

        //
        // This is a standard rendering loop: begin the frame, then rasterize
        // each layer of the RenderList according via rasterizeList.
        //
        mspRasterizer->beginFrame(algorithm);
        mspRasterizer->rasterizeList(algorithm, instances);
        mspRasterizer->endFrame();
    }

    /*
        Called every frame to give the renderer an opportunity to do 
        any data update such as simulations, animations, or other 
        incremental data updates.
     */
    virtual void 
    updateFrame (lx0::ViewPtr spView) 
    {
        lx0::ProfileSection section(profile.update);

        //
        // Rotate the models on every update, if that option is enabled.
        //
        //@todo Rotate the camera instead
        //
        if (mbRotate)
        {
            mRotation = glm::rotate(mRotation, 1.0f, glm::vec3(0, 0, 1));
            auto spTransform = mspRasterizer->createTransform(mRotation);
            
            for (auto it = mInstances.begin(); it != mInstances.end(); ++it)
                (*it)->spTransform = spTransform;
        }

        //
        // Check if new geometry has loaded and update the active instance.  
        // Recreate the camera as well since the bounds will have changed.
        //
        mspCamera = _createCamera(mInstances.front()->spGeometry->mBBox, mCurrentZoom);

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
        if (evt == "next_material" || evt == "prev_material")
        {
            mCurrentMaterial = (evt == "next_material")
                ? (mCurrentMaterial + 1)
                : (mCurrentMaterial + mMaterials.size() - 1);
            mCurrentMaterial %= mMaterials.size();

            mInstances.front()->spMaterial = mMaterials[mCurrentMaterial];
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

        // Once more for <Light> elements
        //
        auto vLights = spDocument->getElementsByTagName("Light");
        for (auto it = vLights.begin(); it != vLights.end(); ++it)
            _processLight(*it);

        // And again for the <Entities> list
        //
        auto vEntities = spDocument->getElementsByTagName("Entities");
        for (auto it = vEntities.begin(); it != vEntities.end(); ++it)
        {
            int count = (*it)->childCount();
            for (int i = 0; i < count; ++i)
                _processEntity((*it)->child(i));
        }
    }

    void _processEntity (lx0::ElementPtr spElem)
    {
        std::string tag = spElem->tagName();

        if (tag == "PointCloud")
            _addPointCloud(spElem);
        else if (tag == "Geometry")
            _processGeometry(spElem);
        else
            throw lx_error_exception("Unrecognized entity tag '%s'", tag);
    }

    class PointCloud : public lx0::Element::Component
    {
    public:
        ~PointCloud()
        {
        }

        virtual lx0::uint32 flags               (void) const { return lx0::Element::Component::eSkipUpdate; }

        Renderer*                 mpRenderer;
        lx0::RasterizerGL*        mpRasterizer;

        lx0::InstancePtr          mspInstance;
        glgeom::primitive_buffer  mPrimitive;        
        std::vector<float>        mSpeeds;

        Coroutine                 mCoroutine;

        void initialize (Renderer* pRenderer)
        {
            //@todo
            // - Make the courtine a member variable with shared_ptr<> and handle
            // that cancels the event on the dtor
            // - Add a Coroutine protected: _onExit() that resets the shared_ptr<>
            //   so data is not held past processing.

            mpRenderer   = pRenderer;
            mpRasterizer = pRenderer->mspRasterizer.get();

            //
            // Create a co-routine to run through the event queue.  
            //
            // This effectively creates a sequential sequence of steps that can
            // alternate between the main and worker threads as well as pausing
            // without spinning any cycles.
            //
            // This experimental code to find the best way to represent an
            // animated entity composed of a sequence of time-dependent events
            // that can be processed in multiple threads.
            //

            const size_t kCount =  800;
            auto c = &mCoroutine;           

            c->delay(1000);
            c->worker([this,kCount]() {
                lx_message("generate...");

                auto rollxy = lx0::random_die_f(-1, 1, 256);
                auto rollz = lx0::random_die_f(.0, .5, 256);
                auto rolls = lx0::random_die_f(.002f, .01f, 147);

                mSpeeds.reserve(32);
                for (size_t i = 0; i < 32; ++i)
                    mSpeeds.push_back( rolls() );
                
                mPrimitive.type = "points";
                mPrimitive.vertex.positions.reserve(kCount);
                for (size_t i = 0; i < kCount; ++i)
                {
                    glgeom::point3f p( rollxy(), rollxy(), rollz() );                        
                    mPrimitive.vertex.positions.push_back(p);
                }
            });
            // Add a dummy step to test that parallel worker tasks work correctly.
            c->worker(
                []() {
                    for (int i = 0; i < 1000; ++i)
                        std::cout << "#" << std::flush;           
                },
                []() {                
                    for (int i = 0; i < 1000; ++i)
                        std::cout << "@" << std::flush;
                }
            );
            c->worker([]() { std::cout << std::endl; });
            c->main([this]() {
                lx_message("addInstance...");
                auto spGeometry = mpRasterizer->createGeometry(mPrimitive);

                auto pInstance = new lx0::Instance;
                pInstance->spGeometry = spGeometry;
                pInstance->spMaterial = mpRasterizer->acquireMaterial("PointSprite");
                mspInstance.reset(pInstance);

                mpRenderer->_addInstance(mspInstance);
            });
            c->delay(500);
            
            c->main_ret([this]() -> int {
                
                //
                // Update the "particle system"
                //
                int index = 0;
                auto& positions = mPrimitive.vertex.positions;
                for (auto it = positions.begin(); it != positions.end(); ++it, ++index)
                {
                    auto& p = *it;
                    if (p.z > 0.001f)
                    {
                        auto speed = mSpeeds[index % mSpeeds.size()];
                        p.z -= speed;
                    }
                    else
                        p.z = .75f;
                }

                //
                // Recreate the geometry.  This is not necessarily the most efficient means
                // of updating the particles.
                // 
                mspInstance->spGeometry = mpRasterizer->createGeometry(mPrimitive);

                return -1;
            });

            lx0::Engine::acquire()->sendTask(10, c->compile());
        }
    };

    void _addPointCloud (lx0::ElementPtr spElem)
    {
        // Create Element::Component
        // that owns a Coroutine
        // that adds an event sequence

        auto pComp = new PointCloud;
        spElem->attachComponent( pComp );
        pComp->initialize(this);
    }

    void _processMaterial (lx0::ElementPtr spElem)
    {
        lx0::lxvar  elemValue = spElem->value();
        lx0::lxvar  render = elemValue.is_defined() ? elemValue.find("render") : lx0::lxvar::undefined();
        lx0::lxvar  graph = elemValue.is_defined() ? elemValue.find("graph") : lx0::lxvar::undefined();

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

    void _processGeometry (lx0::ElementPtr spElem)
    {
        //
        // Extract the data from the DOM
        //
        std::string sourceFilename = spElem->attr("src").as<std::string>();
        _addGeometry(sourceFilename);
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

    static void 
    test3 (glgeom::primitive_buffer& primitive)
    {    
        using namespace glgeom;

        //
        // The algorithm works by looking at the adjacent UV values for each
        // vertex.  Therefore, we need the vertex -> adjacent vertices info.
        //
        if (primitive.adjacency.vertex_to_vertices.empty())
            compute_adjacency_vertex_to_vertices(primitive);
        if (primitive.vertex.normals.empty())
            compute_vertex_normals(primitive);
       
    
        //
        // Get the data structures ready
        //
        std::vector<vector3f> tangents;
        std::vector<vector3f> bitangents;

        auto& positions = primitive.vertex.positions;
        auto& uvs       = primitive.vertex.uv[0];
        const size_t vertexCount = positions.size();
    
        tangents.resize(vertexCount);
        bitangents.resize(vertexCount);

        //
        // Begin the main loop to compute the values for each vertex
        //
        for (size_t vi = 0; vi < vertexCount; ++vi)
        {
            // Create some aliases
            auto& p0 = positions[vi];
            auto& uv0 = uvs[vi];
            auto& adjacent = primitive.adjacency.vertex_to_vertices[vi];

            //
            // This is the crux of the algorithm:
            //
            // - Compute each edge vector (change in position from the vertex to its neighbor)
            // - Compute the change in UV for each edge vector
            // - We know how the UV varies for each edge direction
            //
            // The next steps require a bit more explanation.  The change in UV (dUV) at
            // a given position (P) in a particular direction (v) can be written as a 
            // function F(P,v) = dUV.  We construct this function by writing it as a weighted
            // sum of the changes in UV along each each, each weighted based on the dot 
            // product between v and the edge vector.
            //
            // F(P,v) = sum( dot(v, e/|e|) * dUV/|e| ) / N
            //
            // Doing some algebraic transformation, we can rewrite the function as:
            //
            // F(P,v) = dot(v, sum((e * dUV)/|e|) / N)
            //
            // The code below is computing the value "sum((e * dUV)/|e|) / N" for both
            // the U and V axes.
            //
            glm::vec3 sum[2];
            const size_t N = adjacent.size();

            for (auto it = adjacent.begin(); it != adjacent.end(); ++it)
            {
                const size_t vn = *it;
                lx_check_error( vn != vi, "Vertex is adjacent to itself. Corrupt adjacency info.")

                vector3f dP = positions[vn] - p0;
                vector2f dUV = uvs[vn] - uv0;

                const float len = glgeom::length(dP);
                const glm::vec3 s0 = (dP * dUV[0]).vec / len;
                const glm::vec3 s1 = (dP * dUV[1]).vec / len;
                
                sum[0] += s0;
                sum[1] += s1;
            }
            sum[0] /= N;
            sum[1] /= N;

            lx_check_error( glgeom::is_finite(glgeom::vector3f(sum[0])) );
            lx_check_error( glgeom::is_finite(glgeom::vector3f(sum[1])) );

            //
            // Lastly, we find where the derivative is 0 in order to find the
            // maximum point of increasing U and V.  This works out to a 
            // 3 equations, 3 unknowns problem.  Solve it for x, y, and z
            // and we end up with the directions of maximal increase for 
            // U and V - which is what the tangent and bitangent should be.
            //
            glgeom::vector3f result[2];
            for (int i = 0; i < 2; ++i)
            {
                auto& r = result[i];
                auto& q = sum[i];

                r.z = -(q.x + q.y - q.z) / (2 * q.z);
                r.y = -(q.z * r.z + q.x) / q.y;
                r.x = -(q.y * r.y + q.z) / q.x;
            }

            //
            // The mesh data isn't always well-formed. Catch what circumstances
            // we can reasonably repair.
            //
            if (!is_finite(result[1]))
                result[1] = glgeom::cross(primitive.vertex.normals[vi], result[0]);
            else if (!is_finite(result[0]))
                result[0] = glgeom::cross(result[1], primitive.vertex.normals[vi]);

            //
            // Store the results
            //
            tangents[vi]    = glgeom::normalize(result[0]);
            bitangents[vi]  = glgeom::normalize(result[1]);

            lx_check_error( glgeom::is_finite(tangents[vi]) );
            lx_check_error( glgeom::is_finite(bitangents[vi]) );
        }

        primitive.vertex.tangents.swap(tangents);
    }

    void
    _addGeometryBlender (const std::string filename, size_t index)
    {
        //
        // This is executed as a task in the main thread; therefore no locks or threading 
        // protection is necessary on the rasterizer or geometry list.  If this were
        // executed outside the main thread, then the calls to createQuadList and the
        // modification to mGeometry would be problematic.
        //
        auto addGeometry = [this,index](glgeom::primitive_buffer* primitive) {

            auto spGeometry = mspRasterizer->createGeometry(*primitive);
            spGeometry->mBBox = primitive->bbox;
            delete primitive;

            // Replace the geometry.  This is being called from the main thread, so no locking
            // should be needed.
            mInstances[index]->spGeometry = spGeometry;
            mInstances[index]->spMaterial = mMaterials.front();
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

                lx_message("Generating UVs...");
                glgeom::compute_uv_mapping(*primitive, 0, [](const glgeom::point3f& p, const glgeom::vector3f& n) -> glgeom::point2f {
                    return glgeom::scale( glgeom::mapper_planar_xy(p), glgeom::vector2f(10, 0));
                });

                // Generate tangent vectors
                test3(*primitive);

                


                // Local copy of addGeometry since lamdbas can only capture from the enclosing scope
                auto f = addGeometry;
                if (!lx0::Engine::acquire()->isShuttingDown())
                    lx0::Engine::acquire()->sendTask([primitive,f](){ f(primitive); });
            }
            lx_message("Model loaded in %2%ms", filename, timer.totalMs());
        };

        lx0::Engine::acquire()->sendWorkerTask(loadGeometry);
    }

    void 
    _addGeometry (const std::string filename)
    {
        //
        // Insert a stub instance into the index.  This will be replaced by
        // the real geometry as soon as the geometry is loaded.
        //                
        auto pInstance = new lx0::Instance;
        pInstance->spGeometry = mspRasterizer->acquireGeometry("basic2d/Empty");
        pInstance->spGeometry->mBBox = glgeom::abbox3f( glgeom::point3f(-1, -1, -1), glgeom::point3f( 1,  1,  1) );

        auto index = mInstances.size();
        mInstances.push_back( lx0::InstancePtr(pInstance) );
                
        if (boost::iends_with(filename, ".blend"))
            _addGeometryBlender(filename, index);
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
        float intervalMs = 1500.0f;

        //
        // Create a lambda function that will incrementally zoom in or out
        // over a given time period.
        //            
        float       startValue    = mCurrentZoom;
        float       endValue      = startValue * zoomFactor;
        lx0::uint32 startTime     = lx0::lx_milliseconds();

        //
        // Apply a sinusoidal smooth (ease-in/ease-out) to the alpha value
        // for a smoother animation.  If the set interval time has expired
        // then end the event reoccurence by setting the final value exactly 
        // and returning -1 (i.e. the time delay before rerunning the event).
        //
        auto func = [this, startValue, endValue, startTime, intervalMs]() -> int {
            

            float  alpha = float(lx0::lx_milliseconds() - startTime) / intervalMs;
            if (!(alpha >= 1.0))
            {
                mCurrentZoom = glm::mix(startValue, endValue, glgeom::sinusodial_smooth(alpha));
                return 1;
            }
            else
            {
                mCurrentZoom = endValue;
                return 0;
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
    
    // Current settings
    bool                          mbRotate;
    glm::mat4                     mRotation;
    float                         mCurrentZoom;
    int                           miRenderAlgorithm;
    size_t                        mCurrentMaterial;

    // Application data
    lx0::EventHandle              mEventHandle;
    std::vector<lx0::MaterialPtr> mMaterials;
    std::vector<lx0::InstancePtr> mInstances;
    glgeom::abbox3f               mSceneBounds;
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
