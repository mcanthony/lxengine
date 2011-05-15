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

#include "raytracer.hpp"

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

// Standard headers
#define NOMINMAX
#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

// Lx0 headers
#include <lx0/core/core.hpp>
#include <lx0/core/math/matrix4.hpp>
#include <lx0/core/util/util.hpp>
#include <lx0/prototype/prototype.hpp>
#include <lx0/view.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>

#include <glgeom/glgeom.hpp>
#include <glgeom/prototype/camera.hpp>
#include <glgeom/prototype/std_lights.hpp>
#include <glgeom/prototype/material_phong.hpp>
#include <glgeom/prototype/image.hpp>

#include "glgeom_ext.hpp"


using namespace lx0::core;
using namespace glgeom;

extern glgeom::image3f img;

//===========================================================================//

void
image_fill_checker (image3f& img)
{
    const glgeom::color3f c0(.05f, .05f, 0.0f);
    const glgeom::color3f c1(   0,    0, 0.05f);

    for (int iy = 0; iy < img.height(); ++iy)
    {
        for (int ix = 0; ix < img.width(); ++ix)
        {
            const auto& c = (ix % 2) + (iy % 2) == 1 ? c0 : c1; 
            img.set(ix, iy, c); 
        }
    }
}

//===========================================================================//

//! Quad represented in origin-axis format
/*!
    A rectangular quad in 3-space represented by an origin point and
    two axes.
 */
template <typename T>
class quad_oa_3t
{
public:
    quad_oa_3t() {}
    quad_oa_3t(const point3t<T>& o, const vector3t<T>& x, const vector3t<T>& y)
        : origin(o)
        , x_axis(x)
        , y_axis(y)
    {
    }

    point3t<T>      origin;
    vector3t<T>     x_axis;
    vector3t<T>     y_axis;
};

template <typename T>
point3t<T>
compute_quad_pt (const quad_oa_3t<T>& quad, T x, T y, int width, int height)
{
    T s = (x + 0.5f) / width;
    T t = (y + 0.5f) / height;
    return quad.origin + quad.x_axis * s + quad.y_axis * t;
}

typedef quad_oa_3t<float>   quad_oa_3f;
typedef quad_oa_3t<double>  quad_oa_3d;

template <typename T>
class frustum3t
{
public:
    typedef T               type;
    typedef quad_oa_3t<T>   quad;
    typedef point3t<T>      point3;

    point3      eye;
    quad        near_quad;
    type        far_dist;
};

template <typename T>
frustum3t<T>
frustum_from_camera (const camera3t<T>& cam)
{
    const auto forward = camera_forward_vector(cam);
    const auto right   = camera_right_vector(cam);
    const auto up      = camera_up_vector(cam);
    
    const auto width   = cam.near_plane * tan(cam.field_of_view);
    const auto height  = width / cam.aspect_ratio;

    frustum3t<T> frustum;
    frustum.eye       = cam.position;
    frustum.near_quad.x_axis = right * width;
    frustum.near_quad.y_axis = up * height;
    frustum.near_quad.origin = frustum.eye + forward * cam.near_plane - right * (width / 2) - up * (height / 2);
    frustum.far_dist  = cam.far_plane;

    return frustum;
}

template <typename T>
ray3t<T>
compute_frustum_ray (const frustum3t<T>& frustum, T x, T y, int width, int height)
{
    auto pt = compute_quad_pt(frustum.near_quad, x, y, width, height);
    return ray3t<T>(frustum.eye, normalize(pt - frustum.eye));
}

typedef frustum3t<float>        frustum3f;
typedef frustum3t<double>       frustum3d;

//===========================================================================//

class Camera : public Element::Component
{
public:
    camera3f camera;
};

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

class Material 
    : public Element::Component
    , public material_phong_f           // Multiple inheritance of classes without virtual methods is ok
{
public:
};

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

class Light 
    : public Element::Component
    , public point_light_f
{
public:
};
typedef std::shared_ptr<Light> LightPtr;


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

        mUpdateQueue.push_back([&]() { return _init(), true; });

        mHandlers.insert(std::make_pair("Plane", [&](ElementPtr spElem) {
            auto pGeom = new Plane;
            pGeom->geom.normal = spElem->value().find("normal").convert();
            pGeom->geom.d      = spElem->value().find("d").convert();
            
            pGeom->setMaterial(spElem, spElem->attr("material").query(""));

            std::shared_ptr<Plane> spComp(pGeom);
            mGeometry.push_back(spComp);
            spElem->attachComponent("raytrace", spComp);
        }));

        mHandlers.insert(std::make_pair("Sphere", [&](ElementPtr spElem) {
            auto pGeom = new Sphere;
            pGeom->geom.center = spElem->value().find("center").convert();
            pGeom->geom.radius = spElem->value().find("radius").convert(.5f);

            pGeom->setMaterial(spElem, spElem->attr("material").query(""));
            
            std::shared_ptr<Sphere> spComp(pGeom);
            mGeometry.push_back(spComp);
            spElem->attachComponent("raytrace", spComp);
        }));

        mHandlers.insert(std::make_pair("Cone", [&](ElementPtr spElem) {
            auto pGeom = new Cone;
            pGeom->geom.base = spElem->value().find("base").convert();
            pGeom->geom.radius = spElem->value().find("radius").convert();
            pGeom->geom.axis = spElem->value().find("axis").convert();
            pGeom->setMaterial(spElem, spElem->attr("material").query(""));

            std::shared_ptr<Cone> spComp(pGeom);
            mGeometry.push_back(spComp);
            spElem->attachComponent("raytrace", spComp);
        }));

        mHandlers.insert(std::make_pair("Cylinder", [&](ElementPtr spElem) {
            auto pGeom = new Cylinder;
            pGeom->geom.base = spElem->value().find("base").convert();
            pGeom->geom.radius = spElem->value().find("radius").convert();
            pGeom->geom.axis = spElem->value().find("axis").convert();
            pGeom->setMaterial(spElem, spElem->attr("material").query(""));

            std::shared_ptr<Cylinder> spComp(pGeom);
            mGeometry.push_back(spComp);
            spElem->attachComponent("raytrace", spComp);
        }));

        mHandlers.insert(std::make_pair("Cube", [&](ElementPtr spElem) {
                auto pGeom = new Cube;
                pGeom->geom.center = spElem->value().find("center").convert(point3f(0, 0, 0));
                pGeom->geom.scale  = spElem->value().find("scale").convert(vector3f(1, 1, 1));
                
                pGeom->setMaterial(spElem, spElem->attr("material").query(""));
                
                std::shared_ptr<Cube> spComp(pGeom);
                mGeometry.push_back(spComp);
                spElem->attachComponent("raytrace", spComp);
        }));

        mHandlers.insert(std::make_pair("Material", [&](ElementPtr spElem) {
            auto pMat = new Material;
            pMat->emmissive = spElem->value().find("emmissive").convert(color3f(0, 0, 0));
            pMat->diffuse = spElem->value().find("diffuse").convert(color3f(1, 1, 1));
            pMat->specular = spElem->value().find("specular").convert(color3f(0, 0, 0));
            pMat->specular_n = spElem->value().find("specular_n").convert(8.0f);

            spElem->attachComponent("raytrace", pMat);
        }));

        mHandlers.insert(std::make_pair("Light", [&](ElementPtr spElem) {
            auto pLight = new Light;
            pLight->position = spElem->value().find("position").convert();
            pLight->color    = spElem->value().find("color").convert();
            spElem->attachComponent("raytrace", pLight);
            mLights.push_back(LightPtr(pLight));
        }));

        mHandlers.insert(std::make_pair("Camera", [&](ElementPtr spElem) {
            if (!mCamera)
            {
                auto pCam = new Camera;
                pCam->camera.near_plane = sqrtf(2);
                pCam->camera.position = spElem->value().find("position").convert();
                point3f target = spElem->value().find("look_at").convert();
                pCam->camera.orientation = orientation_from_to_up(pCam->camera.position, target, vector3f(0, 0, 1));
                mCamera = std::shared_ptr<Camera>(pCam);
            }
        }));

        mHandlers.insert(std::make_pair("Environment", [&](ElementPtr spElem) {
            mspEnvironment->shadows = spElem->value().find("shadows").query(true);
        }));
    }

    virtual void onAttached (DocumentPtr spDocument) 
    {
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

        mUpdateQueue.push_back([&]() { return mRenderTime = lx0::util::lx_milliseconds(), true; });
        mUpdateQueue.push_back([=]() { return passQuick->done() ? true : (passQuick->next(), false); });
        mUpdateQueue.push_back([=]() { return passMedium->done() ? true : (passMedium->next(), false); });
        mUpdateQueue.push_back([=]() { return passHigh->done() ? true : (passHigh->next(), false); });
        mUpdateQueue.push_back([&]() { return std::cout << "Done (" << lx0::util::lx_milliseconds() - mRenderTime << " ms)." << std::endl, true; });
    }

    bool _shadowTerm (const point_light_f& light, const intersection3f& intersection)
    {
        if (mspEnvironment->shadows)
        {
            const vector3f L     (light.position - intersection.position);
            const float    distL (length(L));
            const vector3f Ln    (L / distL);
            const ray3f    ray   (intersection.position + 1e-3f * Ln, Ln);
                
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
            
            material_phong_f defaultMaterial;
            const material_phong_f& mat( spGeom->mspMaterial ? *spGeom->mspMaterial : defaultMaterial);

            c = mat.emmissive + env.ambient * mat.ambient;
            for (auto it = mLights.begin(); it != mLights.end(); ++it)
            {
                if (!_shadowTerm(*(*it), intersection))
                    c += shade(*(*it), mat, intersection);
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

    unsigned int                            mRenderTime;
    std::shared_ptr<Environment>            mspEnvironment;
    std::shared_ptr<Camera>                 mCamera;
    std::vector<std::shared_ptr<Geometry>>  mGeometry;
    std::vector<LightPtr>                   mLights;

    struct TraceContext
    {
        frustum3f   frustum;
    };

    std::shared_ptr<TraceContext>       mspTraceContext;
};


lx0::core::DocumentComponent* create_raytracer() { return new RayTracer; }
