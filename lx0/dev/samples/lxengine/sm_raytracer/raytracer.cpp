//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)

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

// Lx0 headers
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/shaderbuilder.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>
#include <lx0/prototype/misc.hpp>

#include <glgeom/glgeom.hpp>
#include <glgeom/prototype/camera.hpp>
#include <glgeom/prototype/std_lights.hpp>
#include <glgeom/prototype/material_phong.hpp>
#include <glgeom/prototype/image.hpp>

#include <glm/gtc/matrix_inverse.hpp>

#include "raytracer.hpp"
#include "glgeom_ext.hpp"
#include "parsers.hpp"

using namespace lx0;
using namespace glgeom;

extern glgeom::image3f img;

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

class ScanIterator
{
public:
    ScanIterator()
        : x (0)
        , y (0)
        , width (0)
        , height (0)
    {
    }

    ScanIterator(int w, int h, std::function<void(int,int)> func)
        : x (0)
        , y (0)
        , width (w)
        , height (h)
        , f (func)
    {
    }

    bool done() const { return !(y < height && x < width); }
    void next() 
    {
        f(x, y);

        if (++x >= width)
        {
            x = 0;
            ++y;
        }
    }

protected:
    int x;
    int y;
    int width;
    int height;
    std::function<void (int, int)> f;
};


//===========================================================================//

class SpatialIndex
{
public:
    void    add (std::shared_ptr<Geometry> spGeometry)
    {
        mGeometry.push_back(spGeometry);
    }

    void    remove() { assert(0); }
    void    modify() { assert(0); }

    bool    intersect   (const ray3f& ray, intersection3f& isect);
    size_t  intersect   (const ray3f& ray, std::vector<std::pair<GeometryPtr, intersection3f>>& hits);

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
SpatialIndex::intersect (const ray3f& ray, std::vector<std::pair<GeometryPtr, intersection3f>>& hits)
{
    for (auto it = mGeometry.begin(); it != mGeometry.end(); ++it)
    {
        intersection3f isect;
        if ((*it)->intersect(ray, isect))
            hits.push_back(std::make_pair(*it, isect));
    }

    return hits.size();
}

//===========================================================================//


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
        std::shared_ptr<Material>  spDefaultMaterial( new GenericMaterial(mShaderBuilder.buildShaderLambda(graph)) );

        mspEnvironment.reset(new Environment);
        mspContext.reset(new Context(spDefaultMaterial));

        registerGeometryParsers([&](std::string name, GeometryParser* pParser) {
            mGeometryParsers[name] = std::unique_ptr<GeometryParser>(pParser);
        });

        mUpdateQueue.push_back([&]() { return _init(), true; });
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
        if (!mUpdateQueue.empty())
            if (mUpdateQueue.front()())
                mUpdateQueue.pop_front();
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

        std::shared_ptr<ScanIterator> passQuick(new ScanIterator (48, 48, [&](int x, int y) {
            int sx = (x * img.width()) / 48;
            int sy = (y * img.height()) / 48;
            int ex = std::min( ((x + 1) * img.width() ) / 48, img.width());
            int ey = std::min( ((y + 1) * img.height() ) / 48, img.height());

            auto c = _trace((sx + ex) / 2, (sy + ey) / 2);

            for (int iy = sy; iy < ey; iy ++)
            {
                for (int ix = sx; ix < ex; ix ++)
                {
                    if ((ix%2) + (iy%2) != 1)
                        img.set(ix, iy, c);
                }
            }
        }));

        std::shared_ptr<ScanIterator> passMedium(new ScanIterator (128, 128, [&](int x, int y) {
            int sx = (x * img.width()) / 128;
            int sy = (y * img.height()) / 128;
            int ex = std::min( ((x + 1) * img.width() ) / 128, img.width());
            int ey = std::min( ((y + 1) * img.height() ) / 128, img.height());

            auto c = _trace((sx + ex) / 2, (sy + ey) / 2);

            for (int iy = sy; iy < ey; iy ++)
            {
                for (int ix = sx; ix < ex; ix ++)
                {
                    if ((ix%2) + (iy%2) == 1)
                        img.set(ix, iy, c);
                }
            }
        }));
        
        std::shared_ptr<ScanIterator> passHigh(new ScanIterator (img.width(), img.height(), [&](int x, int y) {
            img.set(x, y, _trace(x, y)); 
        }));

        mUpdateQueue.push_back([&]() { return mRenderTime = lx0::lx_milliseconds(), true; });
        if (img.width() > 96 || img.height() > 96)
            mUpdateQueue.push_back([=]() { return passQuick->done() ? true : (passQuick->next(), false); });
        if (img.width() > 256 || img.height() > 256)
            mUpdateQueue.push_back([=]() { return passMedium->done() ? true : (passMedium->next(), false); });
        mUpdateQueue.push_back([=]() { return passHigh->done() ? true : (passHigh->next(), false); });
        mUpdateQueue.push_back([&]() { return std::cout << "Done (" << lx0::lx_milliseconds() - mRenderTime << " ms)." << std::endl, true; });
        
        auto varOutputFile = Engine::acquire()->globals().find("output");
        if (varOutputFile.is_defined())
        {
            mUpdateQueue.push_back([varOutputFile]() -> bool { 
                std::cout << "Saving image...";
                lx0::save_png(img, varOutputFile.as<std::string>().c_str());
                std::cout << "done.\n";
                Engine::acquire()->sendEvent("quit");
                return true;
            });
        }
    }

    bool _shadowTerm (const point_light_f& light, const intersection3f& intersection)
    {
        if (mspEnvironment->shadows)
        {
            const vector3f L     (light.position - intersection.positionWc);
            const float    distL (length(L));
            const vector3f Ln    (L / distL);
            const ray3f    ray   (intersection.positionWc + 1e-3f * Ln, Ln);
                
            intersection3f isect;
            mIndex.intersect(ray, isect);
            return (isect.distance < distL);
        }
        else
            return false;
    }

    color3f _trace (int x, int y)
    {
        const auto& env = *mspEnvironment;

        if (x == 0 && y % 4 == 0)
            std::cout << "Tracing row " << y << "..." << std::endl;
        
        ray3f ray = compute_frustum_ray<float>(mspTraceContext->frustum, x, img.height() - y, img.width(), img.height());
        
        std::vector<std::pair<GeometryPtr, intersection3f>> hits;
        mIndex.intersect(ray, hits);
        
        auto c = color3f(0,0,0);
        if (!hits.empty())
        {
            intersection3f* pIntersection = &hits.front().second;
            GeometryPtr spGeom = hits.front().first;

            for (auto it = hits.begin(); it != hits.end(); ++it)
            {
                if (it->second.distance < pIntersection->distance)
                {
                    pIntersection = &it->second;
                    spGeom = it->first;
                }
            }
            const intersection3f& intersection = *pIntersection;
            const vector3f viewDirection = glgeom::normalize(intersection.positionWc - mCamera->camera.position);
            const Material* pMat ( spGeom->mspMaterial ? spGeom->mspMaterial.get() : mspContext->mspMaterial.get());


            glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(mCamera->viewMatrix));

            ShaderBuilder::ShaderContext ctx;
            ctx.unifViewMatrix = mCamera->viewMatrix;

            ctx.unifLightCount = 0;
            for (auto it = mLights.begin(); it != mLights.end(); ++it)
            {
                if (!pMat->allowShadow() || !_shadowTerm(*(*it), intersection))
                {
                    glm::vec3 lightPosEc = glm::vec3(mCamera->viewMatrix * glm::vec4((*it)->position.vec, 1.0f));
                    ctx.unifLightPosition.push_back(lightPosEc);
                    ctx.unifLightColor.push_back( (*it)->color.vec );
                    ctx.unifLightCount ++;
                }
            }
            ctx.fragVertexOc = intersection.positionOc.vec;
            ctx.fragNormalOc = intersection.normal.vec;
            ctx.fragNormalEc = normalMatrix * ctx.fragNormalOc;
            ctx.fragVertexEc = glm::vec3(mCamera->viewMatrix * glm::vec4(ctx.fragVertexOc, 1));

            c = pMat->shade(ctx, env.ambient, intersection);
        }
        else
            c *= .5f;

        return c;
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
        else if (tag == "Material") 
        {
            lxvar graph;
            graph["_type"] = "phong";
            graph["emissive"] = spElem->value().find("emissive");
            graph["diffuse"] = spElem->value().find("diffuse");
            graph["specular"] = spElem->value().find("specular");
            graph["specularEx"] = spElem->value().find("specular_n");

            auto shader = mShaderBuilder.buildShaderLambda(graph);
            auto pMat = new GenericMaterial(shader);

            spElem->attachComponent(pMat);
        }
        else if (tag == "Material2") 
        {
            lx0::lxvar  graph = spElem->value().find("graph");

            auto shader = mShaderBuilder.buildShaderLambda(graph);
            auto pMat = new GenericMaterial(shader);

            spElem->attachComponent(pMat);
        }
        else if (tag == "Light")
        {
            auto pLight = new Light;
            pLight->position = spElem->value().find("position").convert();
            pLight->color    = spElem->value().find("color").convert();
            
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

    std::deque<std::function<bool (void)>>  mUpdateQueue;

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
