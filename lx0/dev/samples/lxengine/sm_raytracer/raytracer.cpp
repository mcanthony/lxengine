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

using namespace lx0;
using namespace glgeom;


extern glgeom::image3f img;

//===========================================================================//

template <typename T>
color3t<T> shade_ambient (const color3t<T>& env_ambient, 
                          const material_phong_t<T>& mat) 
{
    return mat.emmissive + env_ambient * mat.ambient;
}

//===========================================================================//

class Camera : public Element::Component
{
public:
    camera3f    camera;
    glm::mat4   viewMatrix;
    glm::mat4   projMatrix;
};

//===========================================================================//

class Environment : public Element::Component
{
public:
    Environment() 
        : ambient (0.0f, 0.005f, 0.02f) 
        , shadows (true) 
    {}

    color3f     ambient;
    bool        shadows;
};

//===========================================================================//

class Material : public Element::Component
{
public:
    virtual     bool        allowShadow    (void) const { return true; }
    virtual     color3f     shadeAmbient   (ShaderBuilder::ShaderContext& ctx, const color3f& ambient, const intersection3f& intersection) const { return color3f(0, 0, 0); }
    virtual     color3f     shadeLight     (const point_light_f& light, const vector3f& viewDirection, const intersection3f& intersection) const { return color3f(0, 0, 0); }
};

static glm::vec2   spherical(const glm::vec3& positionOc, glm::vec2 scale)
{
    // Convert to spherical coordinates.  Note in this definition                       
    // theta runs along the z axis and phi is in the xy plane.                          
    float r = glm::length(positionOc);                                                     
    float phi = atan2(positionOc.y, positionOc.x);                                   
    float theta = acos(positionOc.z / r);                                             
                                                                                            
    // Normalize to [0-1) range                                                         
    glm::vec2 uv;                                                              
    uv.x = (phi + glgeom::pi().value) / (2 * glgeom::pi().value);                                                       
    uv.y = theta / glgeom::pi().value;                                                                  
    return uv * scale;      
}

static glm::vec3   checker (
    const glm::vec3& color0, 
    const glm::vec3& color1, 
    const glm::vec2& uv) 
{
    glm::vec2 t = glm::abs( glm::fract(uv) );                                                        
    glm::ivec2 s = glm::ivec2(glm::trunc(glm::vec2(2,2) * t));                                                    
                                                                                        
    if ((s.x + s.y) % 2 == 0)                                                         
        return color0;                                                              
    else                                                                            
        return color1;    
}


class GenericMaterial 
    : public Material
{
public:
    GenericMaterial(ShaderBuilder::ShadeFunction shader)
    {
        mShader = shader;
    }

    virtual     color3f     shadeAmbient   (ShaderBuilder::ShaderContext& ctx, const color3f& ambient, const intersection3f& intersection) const 
    { 
        return glgeom::color3f( mShader(ctx) );
    }

    virtual     color3f     shadeLight     (const point_light_f& light, const vector3f& viewDirection, const intersection3f& intersection) const 
    { 
        return color3f(0, 0, 0); 
    }

protected:
    ShaderBuilder::ShadeFunction mShader;
};

class PhongMaterial
    : public Material
    , public material_phong_f           // Multiple inheritance of classes without virtual methods is ok
{
public:
    virtual     color3f     shadeAmbient   (ShaderBuilder::ShaderContext& ctx, const color3f& ambient, const intersection3f& intersection) const { return shade_ambient(ambient, *this); }
    virtual     color3f     shadeLight     (const point_light_f& light, const vector3f& viewDirection, const intersection3f& intersection) const 
    { 
        return glgeom::shade_light(light, *this, -viewDirection, intersection); 
    }
};

class NormalMaterial
    : public Material
{
public:
    virtual     color3f     shadeAmbient   (ShaderBuilder::ShaderContext& ctx, const color3f& ambient, const intersection3f& intersection) const 
    { 
        return color3f( abs(intersection.normal).vec );
    }
    virtual     color3f     shadeLight     (const point_light_f& light, const vector3f& viewDirection, const intersection3f& intersection) const 
    { 
        return color3f(0, 0, 0); 
    }
};


class LightGradientMaterial
    : public Material
{
public:
    virtual     bool        allowShadow    (void) const { return false; }

    virtual     color3f     shadeAmbient   (ShaderBuilder::ShaderContext& ctx, const color3f& ambient, const intersection3f& intersection) const 
    { 
        return color3f(0, 0, 0);
    }
    virtual     color3f     shadeLight     (const point_light_f& light, const vector3f& viewDirection, const intersection3f& intersection) const 
    { 
        // 
        // L = unit vector from light to intersection point; the "incidence vector" I is the
        //     vector pointing in the opposite direction of L.
        // N = surface normal at the point of intersection
        //
        const vector3f  L     (normalize(light.position - intersection.positionWc));
        const vector3f& N     (intersection.normal);
        const float     NdotL ( dot(N, L) );
                
        const float diffuseSample = (NdotL + 1.0f) / 2.0f;
        const auto diffuse = mGradient.get(int(diffuseSample * (mGradient.width() - 1)), 0);

        return diffuse * light.color;
    }

    void    setTexture (std::string s)
    {
        lx0::load_png(mGradient, s.c_str());
    }

protected:
    glgeom::image3f mGradient;
};

//===========================================================================//

class Geometry : public Element::Component
{
public:
    virtual ~Geometry(){}

    bool intersect (const ray3f& ray, intersection3f& isect) 
    {
        if (_intersect(ray, isect))
        {
            assert( isect.distance >= 0 );
            return true;
        }
        else
            return false;
    }

    bool setMaterial (ElementPtr spElem, std::string matName)
    {
        if (!matName.empty())
        {
            auto spMatElem = spElem->document()->getElementById(matName);
            if (spMatElem)
                mspMaterial = spMatElem->getComponent<Material>("raytrace");
            return true;
        }
        else
            return false;
    }
    std::shared_ptr<Material>   mspMaterial;

protected:
    virtual bool _intersect (const ray3f&, intersection3f& isect) { return false; }
};

typedef std::shared_ptr<Geometry> GeometryPtr;

//===========================================================================//

class Plane : public Geometry
{
public:
    glgeom::plane3f geom;

protected:
    virtual bool _intersect (const ray3f& ray, intersection3f& isect) 
    {
        return  glgeom::intersect(ray, geom, isect);
    }
};

//===========================================================================//

class Cube : public Geometry
{
public:
    glgeom::cube3f geom;

protected:
    virtual bool _intersect (const ray3f& ray, intersection3f& isect) 
    {
        return  glgeom::intersect(ray, geom, isect);
    }
};

//===========================================================================//

class Sphere : public Geometry
{
public:
    glgeom::sphere3f geom;

protected:
    virtual bool _intersect (const ray3f& ray, intersection3f& isect) 
    {
        return  glgeom::intersect(ray, geom, isect);
    }
};

//===========================================================================//

class Cone : public Geometry
{
public:
    glgeom::cone3f geom;

protected:
    virtual bool _intersect (const ray3f& ray, intersection3f& isect) 
    {
        return  glgeom::intersect(ray, geom, isect);
    }
};

//===========================================================================//

class Cylinder : public Geometry
{
public:
    glgeom::cylinder3f geom;

protected:
    virtual bool _intersect (const ray3f& ray, intersection3f& isect) 
    {
        return  glgeom::intersect(ray, geom, isect);
    }
};

//===========================================================================//

class Light 
    : public Element::Component
    , public point_light_f
{
public:
    ~Light()
    {
        lx_log("Light dtor");
    }
};
typedef std::shared_ptr<Light> LightPtr;


//===========================================================================//

class Context
{
public:
    Context()
        : mspMaterial( new PhongMaterial )
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
        mspEnvironment.reset(new Environment);
        mspContext.reset(new Context);

        mUpdateQueue.push_back([&]() { return _init(), true; });

        mHandlers.insert(std::make_pair("Plane", [&](ElementPtr spElem) {
            auto pGeom = new Plane;
            pGeom->geom.normal = spElem->value().find("normal").convert();
            pGeom->geom.d      = spElem->value().find("d").convert();
            
            pGeom->setMaterial(spElem, query(spElem->attr("material"), ""));

            std::shared_ptr<Plane> spComp(pGeom);
            mGeometry.push_back(spComp);
            spElem->attachComponent("raytrace", spComp);
        }));

        mHandlers.insert(std::make_pair("Sphere", [&](ElementPtr spElem) {
            auto pGeom = new Sphere;
            pGeom->geom.center = spElem->value().find("center").convert();
            pGeom->geom.radius = spElem->value().find("radius").convert(.5f);

            pGeom->setMaterial(spElem, query(spElem->attr("material"), ""));
            
            std::shared_ptr<Sphere> spComp(pGeom);
            mGeometry.push_back(spComp);
            spElem->attachComponent("raytrace", spComp);
        }));

        mHandlers.insert(std::make_pair("Cone", [&](ElementPtr spElem) {
            auto pGeom = new Cone;
            pGeom->geom.base = spElem->value().find("base").convert();
            pGeom->geom.radius = spElem->value().find("radius").convert();
            pGeom->geom.axis = spElem->value().find("axis").convert();
            pGeom->setMaterial(spElem, query(spElem->attr("material"), ""));

            std::shared_ptr<Cone> spComp(pGeom);
            mGeometry.push_back(spComp);
            spElem->attachComponent("raytrace", spComp);
        }));

        mHandlers.insert(std::make_pair("Cylinder", [&](ElementPtr spElem) {
            auto pGeom = new Cylinder;
            pGeom->geom.base = spElem->value().find("base").convert();
            pGeom->geom.radius = spElem->value().find("radius").convert();
            pGeom->geom.axis = spElem->value().find("axis").convert();
            pGeom->setMaterial(spElem, query(spElem->attr("material"), ""));

            std::shared_ptr<Cylinder> spComp(pGeom);
            mGeometry.push_back(spComp);
            spElem->attachComponent("raytrace", spComp);
        }));

        mHandlers.insert(std::make_pair("Cube", [&](ElementPtr spElem) {
            auto pGeom = new Cube;
            pGeom->geom.center = spElem->value().find("center").convert(point3f(0, 0, 0));
            pGeom->geom.scale  = spElem->value().find("scale").convert(vector3f(1, 1, 1));
                
            pGeom->setMaterial(spElem, query(spElem->attr("material"), ""));
                
            std::shared_ptr<Cube> spComp(pGeom);
            mGeometry.push_back(spComp);
            spElem->attachComponent("raytrace", spComp);
        }));

        mHandlers.insert(std::make_pair("Material", [&](ElementPtr spElem) {
            auto pMat = new PhongMaterial;
            pMat->emmissive = spElem->value().find("emmissive").convert(color3f(0, 0, 0));
            pMat->diffuse = spElem->value().find("diffuse").convert(color3f(1, 1, 1));
            pMat->specular = spElem->value().find("specular").convert(color3f(0, 0, 0));
            pMat->specular_n = spElem->value().find("specular_n").convert(8.0f);

            spElem->attachComponent("raytrace", pMat);
        }));

        mHandlers.insert(std::make_pair("Material2", [&](ElementPtr spElem) {
            lx0::lxvar  graph = spElem->value().find("graph");
            //auto pMat = new GenericMaterial(graph);
            
            auto shader = mShaderBuilder.buildShaderLambda(graph);
            auto pMat = new GenericMaterial(shader);

            spElem->attachComponent("raytrace", pMat);
        }));

        mHandlers.insert(std::make_pair("NormalMaterial", [&](ElementPtr spElem) {
            auto pMat = new NormalMaterial;
            spElem->attachComponent("raytrace", pMat);
        }));

        mHandlers.insert(std::make_pair("LightGradientMaterial", [&](ElementPtr spElem) {
            auto pMat = new LightGradientMaterial;
            pMat->setTexture( spElem->value().find("texture").as<std::string>() );
            spElem->attachComponent("raytrace", pMat);
        }));

        mHandlers.insert(std::make_pair("Light", [&](ElementPtr spElem) {
            auto pLight = new Light;
            pLight->position = spElem->value().find("position").convert();
            pLight->color    = spElem->value().find("color").convert();
            
            LightPtr spLight(pLight);
            spElem->attachComponent("raytrace", spLight);
            mLights.push_back(spLight);
        }));

        mHandlers.insert(std::make_pair("Camera", [&](ElementPtr spElem) {
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
        }));

        mHandlers.insert(std::make_pair("Environment", [&](ElementPtr spElem) {
            mspEnvironment->shadows = query(spElem->value().find("shadows"), true);
        }));
    }

    virtual void onAttached (DocumentPtr spDocument) 
    {
        lx0::processIncludeDocument(spDocument);

        spDocument->iterateElements([&](ElementPtr spElem) -> bool { 
            _onElementAddRemove(spElem, true); 
            return false; 
        });
    }
    virtual void onElementAdded (DocumentPtr spDocument, ElementPtr spElem) 
    {
        lx_debug("Element '%s' added", spElem->tagName().c_str());
        _onElementAddRemove(spElem, true); 
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
                
            for (auto it = mGeometry.begin(); it != mGeometry.end(); ++it)
            {
                intersection3f isect;
                if ((*it)->intersect(ray, isect) && isect.distance < distL)
                    return true;
            }
            return false;
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
        for (auto it = mGeometry.begin(); it != mGeometry.end(); ++it)
        {
            intersection3f isect;
            if ((*it)->intersect(ray, isect))
                hits.push_back(std::make_pair(*it, isect));
        }

        
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


            glm::mat3 normalMatrix = glm::mat3(glm::gtx::matrix_inverse::inverseTranspose(mCamera->viewMatrix));

            ShaderBuilder::ShaderContext ctx;
            ctx.unifLightCount = 0;
            for (auto it = mLights.begin(); it != mLights.end(); ++it)
            {
                if (!pMat->allowShadow() || !_shadowTerm(*(*it), intersection))
                {
                    ctx.unifLightPosition.push_back( (*it)->position.vec );
                    ctx.unifLightColor.push_back( (*it)->color.vec );
                    ctx.unifLightCount ++;
                }
            }
            ctx.fragVertexOc = intersection.positionOc.vec;
            ctx.fragNormalOc = intersection.normal.vec;
            ctx.fragNormalEc = normalMatrix * ctx.fragNormalOc;
            ctx.fragVertexEc = glm::vec3(mCamera->viewMatrix * glm::vec4(ctx.fragVertexOc, 1));

            c = pMat->shadeAmbient(ctx, env.ambient, intersection);
            for (auto it = mLights.begin(); it != mLights.end(); ++it)
            {
                if (!pMat->allowShadow() || !_shadowTerm(*(*it), intersection))
                    c += pMat->shadeLight(*(*it), ray.direction, intersection);
            }
        }
        else
            c *= .5f;

        return c;
    }

protected:
    void _onElementAddRemove (ElementPtr spElem, bool bAdd)
    {
        const std::string tag = spElem->tagName();

        auto it = mHandlers.find(spElem->tagName());
        if (it != mHandlers.end())
            it->second(spElem);
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
    std::vector<std::shared_ptr<Geometry>>  mGeometry;
    std::vector<LightPtr>                   mLights;

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
