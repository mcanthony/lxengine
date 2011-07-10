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

#include <lx0/lxengine.hpp>
#include <lx0/util/misc.hpp>

using namespace lx0;

//===========================================================================//

class ControllerImp : public lx0::Controller
{
public:
    ControllerImp (lx0::Document* pDoc) : mpDocument(pDoc) {}

    virtual void handleEvent (std::string evt, lx0::lxvar params)
    {
        if (evt == "move_forward")
        {
            const float step = params.convert();
            auto spElem = mpDocument->getElementsByTagName("Camera")[0];
 
            lxvar val = spElem->value();
            glgeom::point3f pos = val["position"].convert();
            glgeom::point3f target = val["look_at"].convert();

            auto dir = glgeom::normalize(target - pos);
            pos += dir * step;
            target += dir * step;

            val.insert("position", lx0::lxvar_from(pos));
            val.insert("look_at", lx0::lxvar_from(target));

            spElem->value(val);

            mpDocument->view(0)->sendEvent("redraw");
        }
        else if (evt == "move_right")
        {
            const float step = params.convert();
            auto spElem = mpDocument->getElementsByTagName("Camera")[0];
 
            lxvar val = spElem->value();
            glgeom::point3f pos = val["position"].convert();
            glgeom::point3f target = val["look_at"].convert();

            auto dir = glgeom::normalize(target - pos);
            dir = glgeom::cross(dir, glgeom::vector3f(0, 0, 1));
            pos += dir * step;
            target += dir * step;

            val.insert("position", lx0::lxvar_from(pos));
            val.insert("look_at", lx0::lxvar_from(target));

            spElem->value(val);

            mpDocument->view(0)->sendEvent("redraw");
        }
        else if (evt == "move_up")
        {
            const float step = params.convert();
            auto spElem = mpDocument->getElementsByTagName("Camera")[0];
 
            lxvar val = spElem->value();
            glgeom::point3f pos = val["position"].convert();
            glgeom::point3f target = val["look_at"].convert();

            pos.z += step;
            target.z += step;

            val.insert("position", lx0::lxvar_from(pos));
            val.insert("look_at", lx0::lxvar_from(target));

            spElem->value(val);

            mpDocument->view(0)->sendEvent("redraw");
        }
        else if (evt == "rotate_horizontal")
        {
            const float step = params.convert();
            auto spElem = mpDocument->getElementsByTagName("Camera")[0];
            lxvar val = spElem->value();
            glgeom::point3f position = val["position"];
            glgeom::point3f target = val["look_at"];

            const auto view = target - position;
            const glgeom::vector3f rotated = rotate(view, glgeom::vector3f(0, 0, 1), float(params));
            target = position + rotated;

            val.insert("position", lx0::lxvar_from(position));
            val.insert("look_at", lx0::lxvar_from(target));

            spElem->value(val);
        }
        else if (evt == "rotate_vertical")
        {
            const float step = params.convert();
            auto spElem = mpDocument->getElementsByTagName("Camera")[0];
            lxvar val = spElem->value();
            glgeom::point3f position = val["position"];
            glgeom::point3f target = val["look_at"];

            const auto view = target - position;
            const auto right = cross(view, glgeom::vector3f(0, 0, 1));
            const glgeom::vector3f rotated = rotate(view, right, float(params));
            target = position + rotated;

            val.insert("position", lx0::lxvar_from(position));
            val.insert("look_at", lx0::lxvar_from(target));

            spElem->value(val);
        }

    }

    lx0::Document* mpDocument;
};


lx0::Controller*        create_controller(DocumentPtr spDoc)    { return new ControllerImp(spDoc.get()); }
