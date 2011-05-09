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

// Standard headers
#define NOMINMAX
#include <iostream>
#include <string>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

// Library headers
#include <boost/program_options.hpp>

// Lx0 headers
#include <lx0/core/core.hpp>
#include <lx0/core/math/matrix4.hpp>
#include <lx0/core/util/util.hpp>
#include <lx0/canvas/canvas.hpp>
#include <lx0/prototype/prototype.hpp>
#include <lx0/view.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <glgeom/glgeom.hpp>
#include <glgeom/prototype/camera.hpp>

#include <windows.h>
#include <gl/gl.h>

using namespace lx0::core;
using namespace lx0::canvas::platform;
using namespace glgeom;

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


class image3f
{
public:
    image3f() : mWidth(0), mHeight(0) {}
    image3f(int w, int h) : mWidth(w), mHeight(h) { mPixels.resize(mWidth * mHeight); }

    bool    empty() const { return (mWidth == 0 && mHeight == 0); }

    void    set (int x, int y, const color3f& c) { mPixels[y * mWidth + x] = c; }
    color3f& get (int x, int y) { return mPixels[y * mWidth + x]; }
    float*  ptr (void) { return &mPixels[0].r; }

    int     width() const { return mWidth; }
    int     height() const { return mHeight; }

protected:
    int                  mWidth;
    int                  mHeight;
    std::vector<color3f> mPixels;
};

//===========================================================================//

image3f img(512, 512);

class Renderer
{
public:
    void initialize()
    {
        GLuint id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        mId = id;
    }  

    void 
    resize (int width, int height)
    {

    }

    void 
    render (void)	
    {
        glEnable(GL_TEXTURE_2D);
        
        glBindTexture(GL_TEXTURE_2D, mId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0, GL_RGB, GL_FLOAT, img.ptr());
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glColor3f(0, 1, 1);
        glBegin(GL_QUADS);
            glTexCoord2f(0, 1);
            glVertex3f(-1, -1, 0);

            glTexCoord2f(1, 1);
            glVertex3f(1, -1, 0);

            glTexCoord2f(1, 0);
            glVertex3f(1, 1, 0);

            glTexCoord2f(0, 0);
            glVertex3f(-1, 1, 0);
        glEnd();
    }

protected:
    GLuint mId;
};

//===========================================================================//

class LxCanvasImp : public ViewImp
{
public:
    virtual void        createWindow    (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height);
    virtual void        destroyWindow   (void);
    virtual void        show            (View* pHostView, Document* pDocument);

    virtual     void        _onElementAdded             (ElementPtr spElem) {}
    virtual     void        _onElementRemoved           (ElementPtr spElem) {}

    virtual     void        updateBegin     (void) {}
    virtual     void        updateFrame     (DocumentPtr spDocument);
    virtual     void        updateEnd       (void) {}

protected:
    std::auto_ptr<CanvasGL> mspWin;
    CanvasHost              mHost;
    Renderer                mRenderer;
};

void 
LxCanvasImp::createWindow (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height)
{
    width = 512;
    height = 512;

    mspWin.reset( new CanvasGL("LxEngine Raytracer Sample", 16, 16, width, height, false) );
    handle = mspWin->handle();

    mRenderer.initialize();
    mRenderer.resize(width, height);

    mspWin->slotRedraw += [&]() { mRenderer.render(); };
}

void
LxCanvasImp::destroyWindow (void)
{
    mspWin->destroy();
}

void 
LxCanvasImp::show (View* pHostView, Document* pDocument)
{
    mspWin->show();
}

// Belongs in lx0::prototype::control_structures
class timed_gate_block_imp
{
public:
    timed_gate_block_imp (unsigned int delta)
        : mDelta    (delta)
        , mTrigger  (0)
    {
    }
    void operator() (std::function<void()> f)
    {
        auto now = lx0::util::lx_milliseconds();
        if (now > mTrigger)
        {
            f();
            mTrigger = now + mDelta;
        }
    }

protected:
    unsigned int mTrigger;
    unsigned int mDelta;
};
#define timed_gate_block(d,e) \
    static timed_gate_block_imp _timed_block_inst ## __LINE__ (d); \
    _timed_block_inst ## __LINE__ ([&]() e )

void 
LxCanvasImp::updateFrame (DocumentPtr spDocument) 
{
    if (mspWin->keyboard().bDown[KC_ESCAPE])
        Engine::acquire()->sendMessage("quit");
 
    timed_gate_block (50, { 
        mspWin->invalidate();
    });
}

//===========================================================================//

class Camera : public Element::Component
{
public:
    camera3f camera;
};

class Material : public Element::Component
{
public:
    glgeom::color3f color;
};

class Geometry : public Element::Component
{
public:
    virtual ~Geometry(){}

    bool intersect (const ray3f& ray, intersect3f& isect) 
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
    virtual bool _intersect (const ray3f&, intersect3f& isect) { return false; }
};

typedef std::shared_ptr<Geometry> GeometryPtr;

class Plane : public Geometry
{
public:
    glgeom::plane3f geom;

protected:
    virtual bool _intersect (const ray3f& ray, intersect3f& isect) 
    {
        return  glgeom::intersect(ray, geom, isect);
    }
};

class Sphere : public Geometry
{
public:
    glgeom::sphere3f geom;

protected:
    virtual bool _intersect (const ray3f& ray, intersect3f& isect) 
    {
        return  glgeom::intersect(ray, geom, isect);
    }
};

class Light : public Element::Component
{
public:
    glgeom::point3f position;
    glgeom::color3f color;
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

    bool done() { return !(y < height && x < width); }
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

void
image_fill_checker (image3f& img)
{
    const glgeom::color3f c0(.05f, .05f, 0.0f);
    const glgeom::color3f c1(   0,    0, 0.05f);

    for (int iy = 0; iy < img.height(); ++iy)
    {
        for (int ix = 0; ix < img.width(); ++ix)
        {
            img.get(ix, iy) = (ix % 2) + (iy % 2) == 1 ? c0 : c1;
        }
    }
}

class RayTracer : public Document::Component
{
public: 
    RayTracer()
        : mInited (false)
    {
        mFirstPass = ScanIterator(48, 48, [&](int x, int y) {
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
                        img.get(ix, iy) = c;
                }
            }
        });

        mSecondPass = ScanIterator(128, 128, [&](int x, int y) {
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
                        img.get(ix, iy) = c;
                }
            }
        });
        
        mIterator = ScanIterator(img.width(), img.height(), [&](int x, int y) { 
            img.get(x, y) = _trace(x, y); 
        });
        
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
            pGeom->geom.radius = spElem->value().find("radius").convert();

            pGeom->setMaterial(spElem, spElem->attr("material").query(""));
            
            std::shared_ptr<Sphere> spComp(pGeom);
            mGeometry.push_back(spComp);
            spElem->attachComponent("raytrace", spComp);
        }));

        mHandlers.insert(std::make_pair("Material", [&](ElementPtr spElem) {
            auto pMat = new Material;
            pMat->color = spElem->value().find("color").convert();
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
    }

    virtual void onAttached (DocumentPtr spDocument) 
    {
        spDocument->iterateElements([&](ElementPtr spElem) -> bool { 
            _onElementAddRemove(spElem, true); return false; 
        });
    }
    virtual void onElementAdded (DocumentPtr spDocument, ElementPtr spElem) 
    {
    }
    virtual void onElementRemoved (Document*   pDocument, ElementPtr spElem) 
    {
    }
    virtual void onUpdate (DocumentPtr spDocument)
    {
        if (!mInited)
        {
           image_fill_checker(img);

            mspTraceContext.reset(new TraceContext);
            mspTraceContext->frustum = frustum_from_camera(mCamera->camera);

            mInited = true;
        }
        else if (!mFirstPass.done())
            mFirstPass.next();
        else if (!mSecondPass.done())
            mSecondPass.next();
        else if (!mIterator.done())
            mIterator.next();
    }

    color3f _trace (int x, int y)
    {
        if (x == 0)
            std::cout << "Tracing row " << y << "..." << std::endl;
        
        ray3f ray = compute_frustum_ray<float>(mspTraceContext->frustum, x, img.height() - y, img.width(), img.height());
        
        std::vector<std::pair<GeometryPtr, intersect3f>> hits;
        for (auto it = mGeometry.begin(); it != mGeometry.end(); ++it)
        {
            intersect3f isect;
            if ((*it)->intersect(ray, isect))
                hits.push_back(std::make_pair(*it, isect));
        }

        auto c = color3f(0, 0, 0);
        if (!hits.empty())
        {
            intersect3f* pHit = &hits.front().second;
            GeometryPtr spGeom = hits.front().first;

            for (auto it = hits.begin(); it != hits.end(); ++it)
            {
                if (it->second.distance < pHit->distance)
                {
                    pHit = &it->second;
                    spGeom = it->first;
                }
            }
            
            glgeom::color3f matDiffuse(1, 1, 1);
            if (spGeom->mspMaterial)
                matDiffuse = spGeom->mspMaterial->color;

            for (auto it = mLights.begin(); it != mLights.end(); ++it)
            {
                vector3f lightVec(normalize(pHit->position - (*it)->position));
                float diffuseFactor = dot(pHit->normal, -lightVec);
            
                c += diffuseFactor * matDiffuse * (*it)->color;
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

    bool                                    mInited;
    ScanIterator                            mFirstPass;
    ScanIterator                            mSecondPass;
    ScanIterator                            mIterator;

    std::shared_ptr<Camera>                 mCamera;
    std::vector<std::shared_ptr<Geometry>>  mGeometry;
    std::vector<LightPtr>                   mLights;

    struct TraceContext
    {
        frustum3f   frustum;
    };

    std::shared_ptr<TraceContext>       mspTraceContext;
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
        EnginePtr   spEngine   = Engine::acquire();
        spEngine->addViewPlugin("LxCanvas", [] (View* pView) { return new LxCanvasImp; });
        
        DocumentPtr spDocument = spEngine->loadDocument("media2/appdata/sm_raytracer/basic_single_sphere_defaults.xml");
        spDocument->attachComponent("ray", new RayTracer);
        ViewPtr     spView     = spDocument->createView("LxCanvas", "view");
        spView->show();

        exitCode = spEngine->run();
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        lx_fatal("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
