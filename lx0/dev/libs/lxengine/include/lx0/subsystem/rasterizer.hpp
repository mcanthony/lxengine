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

#pragma once

//===========================================================================//
// S T A N D A R D   H E A D E R S
//===========================================================================//

#include <list>
#include <ctime>
#include <boost/logic/tribool.hpp>
#include <glgeom/prototype/material_phong.hpp>
#include <lx0/lxengine.hpp>

#include <GL3/gl3w_modified.hpp>

//===========================================================================//
// F O R W A R D   D E C L A R A T I O N S
//===========================================================================//

namespace lx0 
{
    namespace subsystem
    {
        /*!
            \defgroup lx0_subsystem_rasterizer lx0_subsystem_rasterizer
            \ingroup Subsystem
         */
        namespace rasterizer_ns
        {
            class GlobalPass;

            _LX_FORWARD_DECL_PTRS(Camera);
            _LX_FORWARD_DECL_PTRS(LightSet);
            _LX_FORWARD_DECL_PTRS(Light);
            _LX_FORWARD_DECL_PTRS(Geometry);
            _LX_FORWARD_DECL_PTRS(Transform);
            _LX_FORWARD_DECL_PTRS(Material);
            _LX_FORWARD_DECL_PTRS(Texture);

            _LX_FORWARD_DECL_PTRS(Item);

            _LX_FORWARD_DECL_PTRS(RasterizerGL);

            ///@todo This belong in a detail namespace that's only included internally
            void    check_glerror();
        }
    }

    using namespace lx0::subsystem::rasterizer_ns;
}

//===========================================================================//
// S U B - H E A D E R S
//===========================================================================//

#include <lx0/subsystem/rasterizer/gl/glinterface.hpp>

#include <lx0/subsystem/rasterizer/camera.hpp>
#include <lx0/subsystem/rasterizer/lightset.hpp>
#include <lx0/subsystem/rasterizer/geometry.hpp>
#include <lx0/subsystem/rasterizer/material.hpp>
#include <lx0/subsystem/rasterizer/transform.hpp>
#include <lx0/subsystem/rasterizer/item.hpp>

#include <lx0/subsystem/rasterizer/rasterizergl.hpp>

#include <lx0/subsystem/rasterizer/cache/meshcache.hpp>
