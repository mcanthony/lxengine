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
#include <boost/thread.hpp>

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/rasterizer.hpp>
#include <lx0/subsystem/shaderbuilder.hpp>
#include <lx0/subsystem/javascript.hpp>
#include <lx0/util/blendload.hpp>
#include <lx0/util/misc.hpp>

#include <v8/v8.h>
#include "../../../libs/lxengine/src/subsystem/javascript/v8bind.hpp"

#include <glgeom/ext/primitive_buffer.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>


//===========================================================================//
//   L O C A L   F U N C T I O N S
//===========================================================================//

namespace lx0 { namespace core { namespace lxvar_ns { namespace detail {

    static void _convertTriMesh (lxvar& json, glgeom::primitive_buffer& prim)
    {
        lx_check_error(json["_meshType"].as<std::string>() == "TriMesh");

        const int faceCount = json["_faces"].size();
        const int vertexCount = json["_vertices"].size();

        prim.type = "trilist"; 
        prim.vertex.positions.reserve(vertexCount);
        prim.vertex.normals.reserve(vertexCount);                    
        for (int i = 0; i < vertexCount; ++i)
        {
            lxvar vertex = json["_vertices"][i];
            lxvar v = vertex["position"];
            lxvar n = vertex["normal"];
            lxvar c = vertex["color"];
            prim.vertex.positions.push_back( v.convert() );
            //prim.vertex.normals.push_back( n.convert() );
            //prim.vertex.colors.push_back( c.convert() );
        }

        prim.indices.reserve(faceCount * 3);
        for (int i = 0; i < faceCount; ++i)
        {
            lxvar f = json["_faces"][i]["indices"];
            prim.indices.push_back((int)f[0]);
            prim.indices.push_back((int)f[1]);
            prim.indices.push_back((int)f[2]);
            //prim.indices.push_back((int)f[3]);
        }

    }

    static void _convertPointList (lxvar& json, glgeom::primitive_buffer& prim)
    {
        lx_check_error(json["_meshType"].as<std::string>() == "PointList");

        const int vertexCount = json["_vertices"].size();

        prim.type = "pointlist"; 
        prim.vertex.positions.reserve(vertexCount);
        for (int i = 0; i < vertexCount; ++i)
        {
            lxvar vertex = json["_vertices"][i];
            lxvar v = vertex["position"];
            prim.vertex.positions.push_back( v.convert() );
        }
    }

    static void _convertLineList (lxvar& json, glgeom::primitive_buffer& prim)
    {
        lx_check_error(json["_meshType"].as<std::string>() == "LineList");

        const int vertexCount = json["_vertices"].size();

        prim.type = "linelist"; 
        prim.vertex.positions.reserve(vertexCount);
        for (int i = 0; i < vertexCount; ++i)
        {
            lxvar vertex = json["_vertices"][i];
            lxvar v = vertex["position"];
            prim.vertex.positions.push_back( v.convert() );
        }
    }

    void _convert (lxvar& json, glgeom::primitive_buffer& prim)
    {
        std::string type = json["_meshType"].as<std::string>();
        
        if (type == "TriMesh")
            _convertTriMesh(json, prim);
        else if (type == "PointList")
            _convertPointList(json, prim);
        else if (type == "LineList")
            _convertLineList(json, prim);
        else
            throw lx_error_exception("Unrecognized _meshType '%s'", type);
    }

}}}}

//===========================================================================//
//   R E N D E R A B L E
//===========================================================================//

/*
    Work-in-progress class towards a complex renderable, potentially composed of
    multiple generated and specified meshes.  Logically it is one mesh, but
    may be composed of multiple instances.
 */
class Renderable
{
public:
    lx0::lxvar          mRenderProperties;        // e.g. shadows : { cast : true, receive : false }
    //MeshPtr         mspGeometry;
    //MaterialPtr     mspMaterial;
    //InstanceCache   mInstanceCache;
    lx0::InstancePtr    mspInstance;
};

_LX_FORWARD_DECL_PTRS(Renderable);


struct MaterialData
{
    lx0::lxvar          renderProperties;
    lx0::MaterialPtr spMaterial;
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
        , mbRotate            (true)
        , miRenderAlgorithm   (0)
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

        mspFBOffscreen = mspRasterizer->createFrameBuffer(512, 512);

        //
        // Add an empty light set; the Document will populate it
        // 
        mspLightSet = mspRasterizer->createLightSet();

        //
        // Process the data in the document being viewed
        // 
        _processConfiguration();
        _processDocument( spView->document() );
        
        lx_check_error( !mMaterials.empty() );
        lx_check_error( !mGeometry.empty() );

        //
        // Build the renderable
        //
        mspRenderable.reset(new Renderable);
        mspRenderable->mspInstance.reset(new lx0::Instance);
        mspRenderable->mspInstance->spTransform = mspRasterizer->createTransform(mRotation);
        mspRenderable->mspInstance->spMaterial = mMaterials[mCurrentMaterial].spMaterial;
        mspRenderable->mspInstance->spGeometry = mGeometry[mCurrentGeometry];

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
        switch (miRenderAlgorithm % 2)
        {
        default:
        case 0:
            pass.spFrameBuffer = mspFBOffscreen;
            pass.spCamera   = mspCamera;
            pass.spLightSet = mspLightSet;
            pass.optClearColor = std::make_pair(true, glgeom::color4f(0, 0, 0, 0));
            algorithm.mPasses.push_back(pass);

            pass.spFrameBuffer.reset();
            pass.spSourceFBO = mspFBOffscreen;
            algorithm.mPasses.push_back(pass);
            break;

        case 1:
            pass.spCamera   = mspCamera;
            pass.spLightSet = mspLightSet;
            algorithm.mPasses.push_back(pass);
            break;
        }

        lx0::RenderList instances;
        instances.push_back(0, mspRenderable->mspInstance);

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
        if (mbRotate)
        {
            mRotation = glm::rotate(mRotation, 1.0f, glm::vec3(0, 0, 1));
            mspRenderable->mspInstance->spTransform = mspRasterizer->createTransform(mRotation);
        }
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
            
            mspRenderable->mspInstance->spGeometry = mGeometry[mCurrentGeometry];
            
            //
            // Recreate the camera after geometry changes since the camera position
            // is based on the bounds of the geometry being viewed.
            //
            mspCamera           = _createCamera(mspRenderable->mspInstance->spGeometry->mBBox);
        }
        else if (evt == "next_material" || evt == "prev_material")
        {
            mCurrentMaterial = (evt == "next_material")
                ? (mCurrentMaterial + 1)
                : (mCurrentMaterial + mMaterials.size() - 1);
            mCurrentMaterial %= mMaterials.size();

            mspRenderable->mspInstance->spMaterial = mMaterials[mCurrentMaterial].spMaterial;
        }
        else if (evt == "toggle_rotation")
        {
            mbRotate = !mbRotate;
        }
        else if (evt == "toggle_wireframe")
        {
            // This should be handled as a global rendering algorithm override instead
            for (auto it = mMaterials.begin(); it != mMaterials.end(); ++it)
                (*it).spMaterial->mWireframe = !(*it).spMaterial->mWireframe;
        }
        else if (evt == "cycle_renderalgorithm")
        {
            miRenderAlgorithm++;
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
            //_addGeometry(config["model_filename"]);
        }

        if (config.find("shader_filename").is_defined())
        {
            std::string source     = lx0::string_from_file(config["shader_filename"]);
            lx0::lxvar  parameters = config.find("params_filename").is_defined() 
                ? lx0::lxvar_from_file( config["params_filename"] )
                : lx0::lxvar::undefined();

            _addMaterial("", source, parameters, lx0::lxvar::undefined());
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
            _processGeometry(spDocument, *it);

        // And once more for <Light> elements
        //
        auto vLights = spDocument->getElementsByTagName("Light");
        for (auto it = vLights.begin(); it != vLights.end(); ++it)
            _processLight(*it);
    }

    void _processMaterial (lx0::ElementPtr spElem)
    {
        lx0::lxvar  render = spElem->value().find("render");
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
        _addMaterial(material.uniqueName, material.source, material.parameters, render);
    }

    void _processGeometry (lx0::DocumentPtr spDocument, lx0::ElementPtr spElem)
    {
        //
        // Extract the data from the DOM
        //
        std::string sourceFilename = spElem->attr("src").as<std::string>();
        _addGeometry(spDocument, sourceFilename);
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
    lx0::CameraPtr _createCamera (const glgeom::abbox3f& bbox)
    {
        lx_check_error(!bbox.empty());

        const glgeom::vector3f viewDirection(-1, 2, -1.5f);
        const float            viewDistance (bbox.diagonal() * .9f);

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

        MaterialData data;
        data.renderProperties = render;
        data.spMaterial = mspRasterizer->createMaterial(uniqueName, source, params2);

        mMaterials.push_back(data);
    }

    void _addGeometry (lx0::DocumentPtr spDocument, const std::string filename)
    {
        if (boost::iends_with(filename, ".blend"))
        {
            //
            // This is executed as a task in the main thread; therefore no locks or threading 
            // protection is necessary on the rasterizer or geometry list.
            //
            auto addGeometry = [this](glgeom::primitive_buffer* primitive) {
                    auto spGeometry = mspRasterizer->createQuadList(primitive->indices, 
                                                                    primitive->face.flags, 
                                                                    primitive->vertex.positions, 
                                                                    primitive->vertex.normals, 
                                                                    primitive->vertex.colors);
                    spGeometry->mBBox = primitive->bbox;
                    mGeometry.push_back(spGeometry);
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
        else if (boost::iends_with(filename, ".js"))
        {
            lx_message("Loading geometry script '%1%'", filename);

            auto spJavascriptDoc = spDocument->getComponent<lx0::IJavascriptDoc>();
            
            //
            // The result of the script is converted to an lxvar as an intermediate step.  This is
            // a bit inefficient; but since V8 is linked to as a static library and this plug-in is
            // not the same as the module as the EXE, mixing V8 between this module and the EXE will
            // cause problems.
            //
            auto source = lx0::string_from_file(filename);     
            lx0::lxvar result = spJavascriptDoc->run(source);

            if (result.is_defined())
            {
                glgeom::primitive_buffer primitive = result.convert();
                
                glgeom::abbox3f bbox;
                glgeom::compute_bounds(primitive, bbox);               

                auto spModel = mspRasterizer->createGeometry(primitive);
                spModel->mBBox = bbox;
                mGeometry.push_back(spModel);
            }
            else
                throw lx_error_exception("Script failure!");
        }
    }

    lx0::ShaderBuilder            mShaderBuilder;
    lx0::RasterizerGLPtr          mspRasterizer;
    lx0::FrameBufferPtr           mspFBOffscreen;
    lx0::CameraPtr                mspCamera;
    lx0::LightSetPtr              mspLightSet;
    RenderablePtr                 mspRenderable;

    bool                          mbRotate;
    int                           miRenderAlgorithm;
    glm::mat4                     mRotation;

    size_t                        mCurrentMaterial;
    std::vector<MaterialData>     mMaterials;

    size_t                        mCurrentGeometry;
    std::vector<lx0::GeometryPtr> mGeometry;
};


extern "C" _declspec(dllexport) void initializePlugin()
{
    auto spEngine = lx0::Engine::acquire();   
    spEngine->registerViewComponent("Renderer", []() { return new Renderer; });
}


