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
#include "element_scene.hpp"
#include "physics_document.hpp"

using namespace lx0::subsystem::physics_ns::detail;
using namespace lx0;

//===========================================================================//
//
//===========================================================================//

SceneElem::SceneElem (DocumentPtr spDocument, ElementPtr spElem, PhysicsDoc* pPhysics)
    : mpDocPhysics (pPhysics)
{
    lx_check_error( spElem->getComponent<SceneElem>("physics").get() == nullptr );
    _reset(spElem);
}

void SceneElem::onAttributeChange(ElementPtr spElem, std::string name, lxvar value)
{
    _reset(spElem);
}

void SceneElem::_reset (ElementPtr spElem)
{
    auto windVelocity = spElem->attr("wind_velocity");
    auto windDirection = spElem->attr("wind_direction");
    auto gravity = spElem->attr("gravity");

    if (windVelocity.is_defined())
    {
        float velocity = query(windVelocity, 0.0f);
        mpDocPhysics->setWindVelocity(velocity);
    }
    if (windDirection.is_defined())
    {
        lx_check_error(windDirection.is_array());

        glgeom::vector3f dir(query(windDirection.at(0), 0.0f), query(windDirection.at(1), 0.0f), query(windDirection.at(2), 0.0f) );
        mpDocPhysics->setWindDirection(dir);
    }
    if (gravity.is_defined())
    {
        glgeom::vector3f val = gravity.convert();
        mpDocPhysics->mspDynamicsWorld->setGravity(btVector3(val.x, val.y, val.z));
    }
}
