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

#include <boost/algorithm/string.hpp>
#include <lx0/lxengine.hpp>
#include <lx0/util/misc.hpp>
#include <lx0/subsystem/physics.hpp>
#include "physics/mwphysics.hpp"

#include <bullet/btBulletDynamicsCommon.h>

using namespace lx0;

//===========================================================================//

class ControllerImp : public lx0::Controller
{
public:
    ControllerImp (lx0::Document* pDoc) 
        : mpDocument(pDoc) 
    {}

    virtual void handleEvent (std::string evt, lx0::lxvar params)
    {
        if (boost::starts_with(evt, "move_") || boost::starts_with(evt, "rotate_"))
        {
            float step = params;
            auto  spElem = mpDocument->getElementsByTagName("Player")[0];
            auto& position = spElem->value()["position"].unwrap2<glgeom::point3f>();
            auto& target   = spElem->value()["target"].unwrap2<glgeom::point3f>();

            _handleMove(spElem, evt, step, position, target);
            spElem->notifyValueChanged();
        }
        else if (evt == "toggle_gravity")
        {
            auto spMwPhysics = mpDocument->getComponent<MwPhysicsDoc>();
            spMwPhysics->enableGravity( !spMwPhysics->gravityEnabled() );
        }
    }

protected:
    void _handleMove (ElementPtr spElem, std::string evt, float step, glgeom::point3f& position, glgeom::point3f& target)
    {
        auto spMwPhysics = spElem->document()->getComponent<MwPhysicsDoc>();

        if (evt == "move_forward")
        {
            auto dir = glgeom::normalize(target - position);
            spMwPhysics->movePlayer(spElem, dir * step);
        }
        else if (evt == "move_right")
        {
            auto dir = glgeom::normalize(target - position);
            dir = glgeom::cross(dir, glgeom::vector3f(0, 0, 1));
            spMwPhysics->movePlayer(spElem,  dir * step);
        }
        else if (evt == "move_up")
        {
            spMwPhysics->movePlayer(spElem, glgeom::vector3f(0,0,1) * step);
        }
        else if (evt == "rotate_horizontal")
        {
            const auto view = target - position;
            const glgeom::vector3f rotated = rotate(view, glgeom::vector3f(0, 0, 1), step);
            target = position + rotated;
        }
        else if (evt == "rotate_vertical")
        {
            const auto view = target - position;
            const auto right = normalize(cross(view, glgeom::vector3f(0, 0, 1)));
            const glgeom::vector3f rotated = rotate(view, right, step);
            if (fabs(normalize(rotated).z) < .95)
                target = position + rotated;
        }
    }

    lx0::Document* mpDocument;
};

lx0::Controller*        create_controller(DocumentPtr spDoc)    { return new ControllerImp(spDoc.get()); }
