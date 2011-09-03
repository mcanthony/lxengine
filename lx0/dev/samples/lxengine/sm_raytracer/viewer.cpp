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

// Standard headers
#include <iostream>
#include <boost/format.hpp>

// Lx0 headers
#include <lx0/lxengine.hpp>
#include <lx0/prototype/control_structures.hpp>
#include <lx0/prototype/misc.hpp>
#include <glgeom/prototype/image.hpp>

#include <windows.h>
#include <gl/gl.h>

#include "viewer.hpp"
#include "raytracer.hpp"

using namespace lx0;

extern glgeom::image3f img;
extern glgeom::abbox2i imgRegion;

//===========================================================================//

class UIBindingImp : public lx0::UIBinding
{
public:

    virtual     void        updateFrame     (ViewPtr spView,
                                             const KeyboardState& keyboard)
    {
        if (keyboard.bDown[KC_ESCAPE])
            Engine::acquire()->sendEvent("quit");
 
        timed_gate_block (50, { 
            spView->sendEvent("redraw", lxvar());
        });
    }

    virtual     void        onKeyDown       (ViewPtr spView, int keyCode)
    {
        //
        // Take a screenshot
        //
        if (keyCode == KC_S)
        {
            time_t rawtime;
            time(&rawtime);
            struct tm* timeinfo = localtime(&rawtime);
            
            std::string filename = boost::str( boost::format("raytrace_screenshot-%04d%02d%02d_%02d%02d%02d.png") 
                % (timeinfo->tm_year + 1900) 
                % (timeinfo->tm_mon + 1)
                % (timeinfo->tm_mday)
                % (timeinfo->tm_hour)
                % (timeinfo->tm_min)
                % (timeinfo->tm_sec)
                );
            lx0::save_png(img, filename.c_str());
        }
    }
};

//===========================================================================//

class Renderer : public View::Component
{
public:
    virtual void initialize(ViewPtr spView)
    {
        GLuint id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        mId = id;
    }  

    virtual void render (void)	
    {
        glEnable(GL_TEXTURE_2D);

        //
        // Refresh the sub-region of the texture that has been modified since
        // the last update
        //
        glBindTexture(GL_TEXTURE_2D, mId);
        if (!imgRegion.is_empty())
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 
                0, imgRegion.min.y,
                img.width(), 
                imgRegion.height(),
                GL_RGB, GL_FLOAT, 
                img.rowPtr(imgRegion.min.y)
                );
            imgRegion = glgeom::abbox2i();
        }
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        //
        // Draw the textured quad to fill the window
        //
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

lx0::View::Component* create_renderer() { return new Renderer; }
lx0::UIBinding* create_uibinding() { return new UIBindingImp; }
