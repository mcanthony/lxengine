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

image3f img(128, 128);

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
 
    timed_gate_block (20, { 
        mspWin->invalidate();
    });
}

//===========================================================================//

namespace lx0 { namespace core { namespace lxvar_ns { namespace detail {
    void _convert (lxvar& v, vector3d& u)
    {
        u.x = v.at(0).convert();
        u.y = v.at(1).convert();
        u.z = v.at(2).convert();
    }
    void _convert (lxvar& v, point3d& u)
    {
        u.x = v.at(0).convert();
        u.y = v.at(1).convert();
        u.z = v.at(2).convert();
    }
}}}}

class Camera : public std::enable_shared_from_this<Camera>
{
public:
    camera3f camera;
};

class Geometry : public std::enable_shared_from_this<Geometry>
{
public:
    virtual ~Geometry(){}
};

class Plane : public Geometry
{
public:
    glgeom::plane3d geom;
};

class Sphere : public Geometry
{
public:
    glgeom::sphere3d geom;
};


class ScanIterator
{
public:
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

class RayTracer : public Document::Component
{
public: 
    RayTracer()
        : mIterator (img.width(), img.height(), [&](int x, int y) { _trace(x, y); })
    {
        mHandlers.insert(std::make_pair("Plane", [&](ElementPtr spElem) {
            auto pGeom = new Plane;
            pGeom->geom.normal = (vector3d)spElem->value().find("normal").convert();
            pGeom->geom.d      = spElem->value().find("d").convert();
            mGeometry.push_back(std::shared_ptr<Plane>(pGeom));
        }));

        mHandlers.insert(std::make_pair("Sphere", [&](ElementPtr spElem) {
            auto pGeom = new Sphere;
            pGeom->geom.center = spElem->value().find("center").convert();
            pGeom->geom.radius = spElem->value().find("radius").convert();
            mGeometry.push_back(std::shared_ptr<Sphere>(pGeom));
        }));

        mHandlers.insert(std::make_pair("Camera", [&](ElementPtr spElem) {
            if (!mCamera)
            {
                auto pCam = new Camera;
                pCam->camera.position = spElem->value().find("position").convert();
                point3f target = spElem->value().find("look_at").convert();
                pCam->camera.orientation = orientation_from_to_up(pCam->camera.position, target, vector3f(0, 0, 1));
                mCamera = std::shared_ptr<Camera>(pCam);
            }
        }));
    }

    virtual void onAttached (DocumentPtr spDocument) 
    {
        spDocument->iterateElements([&](ElementPtr spElem) -> bool { _onElementAddRemove(spElem, true); return false; });
    }
    virtual void onElementAdded (DocumentPtr spDocument, ElementPtr spElem) 
    {
    }
    virtual void onElementRemoved (Document*   pDocument, ElementPtr spElem) 
    {
    }
    virtual void onUpdate (DocumentPtr spDocument)
    {
        if (!mIterator.done())
            mIterator.next();
    }

    void _trace (int x, int y)
    {
        if (x == 0)
            std::cout << "Tracing row " << y << "..." << std::endl;

        auto& c = img.get(x, y);
        switch ( 2*(y%2) + (x%2) )
        {
        default:
        case 0: c = glgeom::color3f(.3f, .9f, .7f); break;
        case 1: c = glgeom::color3f(.7f, .7f, .7f); break;
        case 2: c = glgeom::color3f(.4f, .4f, .9f); break;
        case 3: c = glgeom::color3f(.6f, 1, 1); break;
        };
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

    ScanIterator                                                   mIterator;
    std::shared_ptr<Camera>                                        mCamera;
    std::vector<std::shared_ptr<Geometry>>                         mGeometry;

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
