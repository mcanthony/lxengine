//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2012 athile@athile.net (http://www.athile.net)

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
#include <array>

// Lx0 headers
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/shaderbuilder.hpp>
#include <lx0/subsystem/javascript.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>
#include <lx0/prototype/misc.hpp>

#include <glgeom/glgeom.hpp>
#include <glgeom/extension/samplers.hpp>
#include <glgeom/prototype/camera.hpp>
#include <glgeom/prototype/std_lights.hpp>
#include <glgeom/prototype/material_phong.hpp>
#include <glgeom/prototype/image.hpp>

#include <glm/gtc/matrix_inverse.hpp>

#include <boost/thread.hpp>

#include "raytracer.hpp"
#include "glgeom_ext.hpp"
#include "parsers.hpp"

using namespace lx0;
using namespace glgeom;

extern glgeom::image3f img;
extern glgeom::abbox2i imgRegion;

template <typename T>
class lxarray
{
public:
    lxarray()
        : mSize (0)
    {
    }

    bool    empty() const { return (mSize == 0); }
    size_t  size() const { return mSize; }

    T& front() { return mData[0]; }

    void push_back (const T& t)
    {
        mData[mSize] = t;
        mSize++;
    }

    T& operator[] (size_t i) { return mData[i]; }

    size_t                        mSize;
    __declspec(align(16)) T       mData[32];
};

//===========================================================================//

template <typename T>
color3t<T> shade_ambient (const color3t<T>& env_ambient, 
                          const material_phong_t<T>& mat) 
{
    return mat.emissive + env_ambient * mat.ambient;
}


//===========================================================================//

class Context
{
public:
    Context(std::shared_ptr<Material> spDefaultMaterial)
        : mspMaterial( spDefaultMaterial )
    {
    }

    std::shared_ptr<Material>   mspMaterial;
};

//===========================================================================//

class SpatialIndex
{
public:
    ~SpatialIndex()
    {
        mGeometry.clear();
    }

    void    add (std::shared_ptr<Geometry> spGeometry)
    {
        mGeometry.push_back(spGeometry);
    }

    void    remove() { assert(0); }
    void    modify() { assert(0); }

    bool    intersect   (const ray3f& ray, intersection3f& isect);
    size_t  intersect   (const ray3f& ray, lxarray<std::pair<Geometry*, intersection3f>>& hits);

protected:
    std::vector<std::shared_ptr<Geometry>>  mGeometry;
};

bool 
SpatialIndex::intersect (const ray3f& ray, intersection3f& closest)
{
    int count = 0;

    for (auto it = mGeometry.begin(); it != mGeometry.end(); ++it)
    {
        intersection3f isect;
        if ((*it)->intersect(ray, isect) && isect.distance < closest.distance)
        {
            ++count;
            closest = isect;
        }
    }

    return (count > 0);
}

size_t 
SpatialIndex::intersect (const ray3f& ray, lxarray<std::pair<Geometry*, intersection3f>>& hits)
{
    for (auto it = mGeometry.begin(); it != mGeometry.end(); ++it)
    {
        auto pGeom = it->get();

        intersection3f isect;
        if (pGeom->intersect(ray, isect))
            hits.push_back(std::make_pair(pGeom, isect));
    }

    return hits.size();
}

//===========================================================================//


struct controller_t2
{
    controller_t2(const size_t threadCount, std::vector<std::function<void()>>& taskPool)
    {
        tasks.swap(taskPool);

        // signals MUST be reserved in advance since the tasks will be using
        // pointers into the array (i.e. reallocation due to resizing would
        // move the objects).
        signals.resize(threadCount);

        threads.resize(threadCount);

        count = 0;
        active = 0;
        mDone = false;
    }

    bool done()
    {
        return mDone;
    }

    void cancel() 
    {
        for (auto it = signals.begin(); it != signals.end(); ++it)
            *it = 1;
        join_all();
    }
    void join_all() 
    {
        group.join_all();
    }

    template <typename F>
    void create_thread(F f)
    {
        auto p = group.create_thread(f);
        signals[count] = 0;
        threads[count] = p;
        ++count;
    }
    
    bool                               mDone;
    volatile int                       active;
    boost::thread_group                group;
    size_t                             count;
    std::vector<boost::thread*>        threads;
    std::vector<int>                   signals;
    std::vector<std::function<void()>> tasks;
};

struct controller_t
{
public:
    controller_t() {}
    controller_t(controller_t2* p) : pImp(p) {}
    controller_t(const controller_t& that) : pImp(that.pImp) {}

    bool done() { return pImp->done(); }
    void cancel()
    {
        pImp->cancel();
    }
    void join_all()
    {
        pImp->join_all();
    }

protected:
    std::shared_ptr<controller_t2> pImp;
};


controller_t
run_tasks (const size_t threadCount, std::vector<std::function<void()>>& taskPool, std::function<void()> final)
{
    controller_t2* pGroup(new controller_t2(threadCount, taskPool));
    
    auto& tasks = pGroup->tasks;
    for (size_t offset = 0; offset < threadCount; ++offset)
    {
        volatile int* pSignal = &pGroup->signals[offset];
        auto& active  = pGroup->active;
        pGroup->create_thread( [&tasks, offset, pGroup, final, pSignal, threadCount]()
        {
            ++(pGroup->active);
            for (size_t i = offset; i < tasks.size(); i += threadCount)
            {
                if (*pSignal)
                    break;

                tasks[i]();
            }
            --(pGroup->active);

            if (pGroup->active == 0)
                final();
            pGroup->mDone = true;
        });
    }
    return controller_t(pGroup);
}

controller_t
run_tasks (const size_t threadCount, std::vector<std::function<void()>>& taskPool)
{
    return run_tasks(threadCount, taskPool, [](){});
}


controller_t quickBarrier;

struct TaskQueue
{
    struct C
    {
        virtual ~C() {}
        virtual bool exec () = 0;
    };
    struct C1 : public C
    {
        C1(const std::function<void()>& g) : f(g) {}
        virtual bool exec () { f(); return true; }
        std::function<void()> f;
    };

    struct C2 : public C
    {
        C2(const std::function<bool()>& g) : f(g) {}
        virtual bool exec () { return f(); }
        std::function<bool()> f;
    };

    void push_back_cond (std::function<bool()> f) 
    { 
        queue.push_back( std::unique_ptr<C>(new C2(f)) );
    }
    void push_back (std::function<void()> f) 
    { 
        queue.push_back( std::unique_ptr<C>(new C1(f)) );
    } 

    void exec_next (void)
    {
        if (!queue.empty())
        {
            auto& task = queue.front();
            
            bool done = task->exec();
            
            if (done)
                queue.pop_front();
        }
    }

    std::deque<std::unique_ptr<C>> queue;
};

//===========================================================================//

static bool 
shadow_term (const glgeom::point3f& lightPosition, const intersection3f& intersection, std::function<void (const ray3f&, intersection3f&)> intersectFunc)
{
    const vector3f L     (lightPosition - intersection.positionWc);
    const float    distL (length(L));
    const vector3f Ln    (L / distL);
    const ray3f    ray   (intersection.positionWc + 1e-3f * Ln, Ln);

    intersection3f isect;
    intersectFunc(ray, isect);

    return !!(isect.distance < distL);
}

//===========================================================================//

static
glgeom::color3f 
_sampleAdaptive (int x, int y, std::function<glgeom::color3f(float, float)> traceFunc)
{
    float offsets1[] = 
    {
        -.25f, -.25f,
        .25f, -.25f,
        -.25f,  .25f,
        .25f,  .25f
    };
    float offsets2[] = 
    {
        .00f, -.25f,
        -.25f, -.00f,
        .00f,  .00f,
        .25f, -.00f,
        .00f,  .25f,
    };

    int j = 0;
    glgeom::color3f c = traceFunc(x + offsets1[j*2+0], y + offsets1[j*2+1]); 
    glgeom::color3f sum  = c;
    glm::vec3 min = c.vec;
    glm::vec3 max = c.vec;

    for (j = 1; j < 4; j++)
    {
        glgeom::color3f c = traceFunc(x + offsets1[j*2+0], y + offsets1[j*2+1]); 
        sum  += c;
        min = glm::min(c.vec, min);
        max = glm::max(c.vec, max);
    }

    glm::vec3 diff = max - min;

    if (std::max(diff.x, std::max(diff.y, diff.z)) > 0.05f)
    {
        for (j = 0; j < 5; j++)
        {
            glgeom::color3f c = traceFunc(x + offsets2[j*2+0], y + offsets2[j*2+1]); 
            sum  += c;
        }
        sum /= 9.0f;
    }
    else
        sum /= 4.0f;

    return sum;
}

static
glgeom::color3f 
_sampleJitter (int x, int y, std::function<glgeom::color3f(float, float)> traceFunc)
{
    static auto r = lx0::random_die_f(-.5f, .5f, 0);
    static const int kSamples = 16;

    glgeom::color3f sum;
    for (int i = 0; i < kSamples; ++i)
    {
        sum += traceFunc(x + r(), y + r()); 
    }

    return sum / float(kSamples);
}

//===========================================================================//

/*
    Dev Notes:

    This class still needs clean-up as it has too many disparate responsibilities:
    - Building the runtime components from Model (i.e. Doc/Element) changes
    - Running the ray-tracer (i.e. responding to the Engine update()'s)
 */
class RayTracer : public Document::Component
{
public: 
    RayTracer()
    {
        lxvar graph;
        graph["_type"] = "phong";
        std::shared_ptr<Material>  spDefaultMaterial( new Material(mShaderBuilder.buildShaderLambda(graph)) );

        mspEnvironment.reset(new Environment);
        mspContext.reset(new Context(spDefaultMaterial));

        // VS2010 bug: if this line is moved to the end of the function, VS seems to destroy the lambda object
        // *before* calling push_back().
        mUpdateQueue.push_back([this]() { _init(); });

        registerGeometryParsers([&](std::string name, GeometryParser* pParser) {
            mGeometryParsers[name] = std::unique_ptr<GeometryParser>(pParser);
        });
    }

    ~RayTracer()
    {
        lx_log("");
    }

    virtual void onAttached (DocumentPtr spDocument) 
    {
        //
        // Pull in any <Include> elements into the main document
        //
        lx0::processIncludeDocument(spDocument);

        //
        // Process the document
        //
        spDocument->iterateElements([&](ElementPtr spElem) -> bool { 
            _onElementAddRemove(spElem, true); 
            return false; 
        });
    }
    virtual void onElementAdded (DocumentPtr spDocument, ElementPtr spElem) 
    {
    }

    virtual void onUpdate (DocumentPtr spDocument)
    {
        mUpdateQueue.exec_next();
    }

    std::vector<std::function<void()>>
    _buildScan (const int tilesX, const int tilesY)
    {
        std::vector<std::function<void()>> tasks;
        tasks.reserve(tilesY);

        for (int y = 0; y < tilesY; ++y)
        {
            const int sy = (y * img.height()) / tilesY;
            const int ey = std::min( ((y + 1) * img.height() ) / tilesY, img.height());

            tasks.push_back([=]() 
            {
                for (int x = 0; x < tilesX; ++x)
                {
                    int sx = (x * img.width()) / tilesX;
                    int ex = std::min( ((x + 1) * img.width() ) / tilesX, img.width());

                    glm::vec2 p( (sx + ex) / 2, (sy + ey) / 2 ); 
                    auto c = _trace(p.x, p.y);

                    for (int iy = sy; iy < ey; iy ++)
                    {
                        for (int ix = sx; ix < ex; ix ++)
                        {
                            if ((ix%2) + (iy%2) != 1)
                            {
                                img.set(ix, iy, c);
                                imgRegion.merge(glm::ivec2(0, iy));
                            }
                        }
                    }
                }
            });
        }
        return tasks;
    }

    std::function<glgeom::color3f (int, int)>
    _acquireSampleFunction (std::string name)
    {
        static std::map<std::string, std::function<glgeom::color3f (int, int)>> table;
        if (table.empty())
        {
            auto traceFunc = [this](float x, float y) { return _trace(x, y); };

            table["adaptive"] = [traceFunc](int x, int y) { return _sampleAdaptive(x, y, traceFunc); };
            table["jitter"]   = [traceFunc](int x, int y) { return _sampleJitter(x, y, traceFunc); };
            table["single"]   = [traceFunc](int x, int y) { return traceFunc(float(x + .5f), float(y + .5f)); };
        }

        auto it = table.find(name);
        if (it != table.end())
            return it->second;
        else
        {
            lx_warn("Unrecognized sample function '%s'", name.c_str());
            return table.find("single")->second;
        }
    }



    static
    std::vector<std::function<void()>>
    _buildScan (std::function<glgeom::color3f (int, int)> sampleFunc)
    {
        std::vector<std::function<void()>> tasks;
        tasks.reserve(img.height());

        for (int y = 0; y < img.height(); ++y)
        {
            tasks.push_back([y, sampleFunc]() 
            {
                for (int x = 0; x < img.width(); ++x)
                {
                    img.set(x, y, sampleFunc(x, y)); 
                }
                imgRegion.merge(glm::ivec2(0, y));
            });
        }

        return tasks;
    }

    void _init()
    {
        lx_log("Initializing ray tracer");

        image_fill_checker(img);

        if (!mCamera)
        {
            lx_warn("No camera defined.  Nothing to render.");
            Engine::acquire()->sendEvent("quit");
            return;
        }

        mspTraceContext.reset(new TraceContext);
        mspTraceContext->frustum = frustum_from_camera(mCamera->camera);

        const auto maxDimension = std::max(img.width(), img.height());

        mUpdateQueue.push_back([&]() { mRenderTime = lx0::lx_milliseconds(); });
        if (maxDimension > 96 )
        {
            mUpdateQueue.push_back([this]() { quickBarrier = run_tasks(1, _buildScan(48, 48)); }); 
            mUpdateQueue.push_back_cond([]() { return const_cast<controller_t&>(quickBarrier).done(); });
        }
        if (maxDimension > 256)
        {
            mUpdateQueue.push_back([this]() { quickBarrier = run_tasks(1, _buildScan(128, 128)); });
            mUpdateQueue.push_back_cond([]() { return const_cast<controller_t&>(quickBarrier).done(); });
        }

        mUpdateQueue.push_back([&]()
        {
            const std::string samplerName = Engine::acquire()->globals().find("sampler").as<std::string>();

            auto tasks = _buildScan( _acquireSampleFunction(samplerName) );
            auto final = [=]() { std::cout << "Done (" << lx0::lx_milliseconds() - mRenderTime << " ms)." << std::endl; };

            quickBarrier = run_tasks(8, tasks, final);
            Engine::acquire()->slotRunEnd += []() {
                const_cast<controller_t&>(quickBarrier).cancel();
            };        
        });

        auto varOutputFile = Engine::acquire()->globals().find("output");
        if (varOutputFile.is_defined())
        {
            mUpdateQueue.push_back([varOutputFile]() { 
                quickBarrier.join_all();
                std::cout << "Saving image...";
                lx0::save_png(img, varOutputFile.as<std::string>().c_str());
                std::cout << "done.\n";
                Engine::acquire()->sendEvent("quit");
            });
        }

        if (false)
        {
            mUpdateQueue.push_back([]() { Engine::acquire()->sendEvent("quit"); });
        }
    }

    float _shadowTermSingle (const glgeom::point3f& lightPosition, const intersection3f& intersection)
    {
        auto intersectFunc = [this](const ray3f& ray, intersection3f& isect) 
        {
            mIndex.intersect(ray, isect);
        };
        return shadow_term(lightPosition, intersection, intersectFunc) ? 0.0f : 1.0f;
    }

    float _shadowTerm (const point_light_f& light, const intersection3f& intersection)
    {
        if (mspEnvironment->shadows)
        {
            const float radius   = light.glow.radius;
            const float baseTerm = _shadowTermSingle(light.position, intersection);

            //
            // Check if the light is an area light, if so take multiple samples
            //
            if (radius > 0.0f)
            {
                //
                // Compute a 3-space disc about the light oriented toward the interesction point
                // 
                const vector3f L    (normalize(light.position - intersection.positionWc));
                const disc3f   disc (light.position, L, radius);
                
                auto sampler = [this, &intersection](const glgeom::point3f& pt) -> float
                {
                    return _shadowTermSingle (pt, intersection);
                };

                //
                // Take several samples along the circumference of the disc to get an 
                // some sort of guess at the variance.  If the light is not completely
                // visble or completely obscured, then generate far more samples using
                // a random distribution on the disc to come up with a estimate as to
                // what percentage of the disc is visible from the intersection point.
                //
                const size_t kInitial = 6;
                const size_t kFull = 512;
                const float  kEpsilon = 1e-4f;

                const float term = sample_disc_circumference<float>(disc, kInitial, sampler);
                float value = glm::mix(baseTerm, term, 1.0f / float(kInitial + 1)); 
                if (value > 1.0f - kEpsilon || value < kEpsilon )
                    return value;
                else
                    return sample_disc_random<float>(disc, kFull, lx0::random_unit, sampler);
            }
            else
                return baseTerm;
        }
        else
            return 1.0f;
    }

    color3f _trace2 (const Environment& env, const ray3f& ray, int depth)
    {
        if (depth > 4)
            return color3f(.5f, .5f, .5f);

        lxarray<std::pair<Geometry*, intersection3f>> hits;
        mIndex.intersect(ray, hits);
        
        auto c = color3f(0,0,0);
        if (!hits.empty())
        {
            intersection3f* pIntersection = &hits.front().second;
            Geometry* pGeom = hits.front().first;

            for (size_t i = 0; i < hits.size(); ++i)
            {
                auto& it = hits[i];
                if (it.second.distance < pIntersection->distance)
                {
                    pIntersection = &it.second;
                    pGeom = it.first;
                }
            }
            const intersection3f& intersection = *pIntersection;
            const Material* pMat ( pGeom->mspMaterial ? pGeom->mspMaterial.get() : mspContext->mspMaterial.get());

            glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(mCamera->viewMatrix));

            ShaderBuilder::ShaderContext ctx;
            ctx.unifViewMatrix = mCamera->viewMatrix;

            ctx.unifLightCount = 0;
            for (auto it = mLights.begin(); it != mLights.end(); ++it)
            {
                float shadowTerm = pMat->allowShadow() 
                    ? _shadowTerm(*(*it), intersection) 
                    : 1.0f;

                if (shadowTerm > 0.0f)
                {
                    glm::vec3 lightPosEc = glm::vec3(mCamera->viewMatrix * glm::vec4((*it)->position.vec, 1.0f));
                    ctx.unifLightPosition.push_back(lightPosEc);
                    ctx.unifLightColor.push_back( shadowTerm * (*it)->color.vec);
                    ctx.unifLightCount ++;
                }
            }

            ctx.unifEyeWc = ray.origin.vec;

            ctx.fragVertexWc = intersection.positionWc.vec;
            ctx.fragVertexOc = intersection.positionOc.vec;
            ctx.fragNormalWc = intersection.normal.vec;
            ctx.fragNormalOc = intersection.normal.vec;
            ctx.fragNormalEc = normalMatrix * ctx.fragNormalOc;
            ctx.fragVertexEc = glm::vec3(mCamera->viewMatrix * glm::vec4(ctx.fragVertexOc, 1));
            
            ctx.traceFunc = [&](const ray3f& ray) -> color3f {
                return _trace2(env, ray, depth + 1);
            };

            c = pMat->shade(ctx, env.ambient, intersection);
        }
        return c;
    }

    color3f _trace (float x, float y)
    {
        const auto& env = *mspEnvironment;

        if (x == 0 && int(y) % 4 == 0 && int(10.0f * fmodf(y, 1.0f)) == 0)
            std::cout << "Tracing row " << y << "..." << std::endl;
        
        ray3f ray = compute_frustum_ray<float>(mspTraceContext->frustum, x, float(img.height()) - y, img.width(), img.height());
        
        return _trace2(env, ray, 0);
    }

protected:
    void _onElementAddRemove (ElementPtr spElem, bool bAdd)
    {
        const std::string tag = spElem->tagName();

        auto git = mGeometryParsers.find(tag);
        if (git != mGeometryParsers.end())
        {
            auto pGeometry = git->second->create(spElem);
            
            std::shared_ptr<Geometry> spGeometry(pGeometry);
            spGeometry->setMaterial(spElem, query(spElem->attr("material"), ""));
            spElem->attachComponent(spGeometry);
            mIndex.add(spGeometry);
        }
        else if (tag == "Texture")
        {
            std::string id = spElem->attr("id").as<std::string>();
            std::string type = spElem->value().find("type").as<std::string>();
            int width = spElem->value().find("width").as<int>();
            int height = spElem->value().find("height").as<int>();
            std::string funcName = spElem->value().find("function").as<std::string>();

            auto spIJavascriptDoc = spElem->document()->getComponent<IJavascriptDoc>();

            if (type == "cubemap")
            {
                glgeom::cubemap3f* cubemap( new cubemap3f(width, height));
                            
                
                std::cout << "Generating texture (cubemap)...";
                spIJavascriptDoc->runInContext([&](void) 
                {
                    auto func = spIJavascriptDoc->acquireFunction<glm::vec3 (float, float, float)>( funcName.c_str() );
                    glgeom::generate_cube_map(*cubemap, [func](const glm::vec3& p) -> glm::vec3 {
                        return func(p.x, p.y, p.z);
                    });
                });
                std::cout << "done." << std::endl;

                mShaderBuilder.addTexture(id, std::shared_ptr<glgeom::cubemap3f>(cubemap));
            }
            else if (type == "2d")
            {
                glgeom::image3f* image( new image3f(width, height) );

                std::cout << "Generating texture (2d)...";
                spIJavascriptDoc->runInContext([&](void) 
                {
                    auto func = spIJavascriptDoc->acquireFunction<glm::vec3 (float, float)>( funcName.c_str() );
                    glgeom::generate_image3f(*image, [func](glm::vec2 p, glm::ivec2) -> glm::vec3 {
                        return func(p.x, p.y);
                    });
                });
                std::cout << "done." << std::endl;

                mShaderBuilder.addTexture(id, std::shared_ptr<glgeom::image3f>(image));
            }
        }
        else if (tag == "Material") 
        {
            lx0::lxvar  graph = spElem->value().find("graph");

            auto shader = mShaderBuilder.buildShaderLambda(graph);
            auto pMat = new Material(shader);

            spElem->attachComponent(pMat);
        }
        else if (tag == "Light" && query(spElem->attr("enabled"), true) )
        {
            auto pLight = new Light;
            pLight->position = spElem->value().find("position").convert();
            pLight->color    = spElem->value().find("color").convert();
            pLight->glow.radius = query_path( spElem->value(), "glow_radius", 0.0f );

            if (pLight->glow.radius > 0)
                std::cout << "Area light detected.\n";
            
            LightPtr spLight(pLight);
            spElem->attachComponent(spLight);
            mLights.push_back(spLight);
        }
        else if (tag == "Camera") 
        {
            if (!mCamera)
            {
                auto pCam = new ::Camera;
                pCam->camera.near_plane = sqrtf(2);
                pCam->camera.position = spElem->value().find("position").convert();
                point3f target = spElem->value().find("look_at").convert();
                
                pCam->camera.orientation = orientation_from_to_up(pCam->camera.position, target, vector3f(0, 0, 1));
                pCam->viewMatrix = glm::lookAt(pCam->camera.position.vec, target.vec, glm::vec3(0, 0, 1));
                pCam->projMatrix = glm::perspective(60.0f, 1.0f, 0.1f, 1000.0f);

                mCamera = std::shared_ptr<::Camera>(pCam);
            }
        }
        else if (tag == "Environment") 
        {
            mspEnvironment->shadows = query(spElem->value().find("shadows"), true);
        }
        else
            std::cout << "Unprocessed tag: " << tag << std::endl;
    }

    std::map<std::string, std::function<void (ElementPtr spElem)>> mHandlers;

    TaskQueue                               mUpdateQueue;

    lx0::ShaderBuilder                      mShaderBuilder;

    unsigned int                            mRenderTime;
    std::shared_ptr<Context>                mspContext;
    std::shared_ptr<Environment>            mspEnvironment;
    std::shared_ptr<::Camera>               mCamera;
    SpatialIndex                            mIndex;
    std::vector<LightPtr>                   mLights;

    std::map<std::string, std::unique_ptr<GeometryParser>> mGeometryParsers;

    struct TraceContext
    {
        frustum3f   frustum;
    };

    std::shared_ptr<TraceContext>       mspTraceContext;
};


lx0::DocumentComponent* create_raytracer() 
{ 
    return new RayTracer; 
}
