//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

    Copyright (c) 2010-2012 athile@athile.net (http://www.athile.net)

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

#include <lx0/_detail/forward_decls.hpp>
#include <glgeom/glgeom.hpp>
#include <glgeom/extension/primitive_buffer.hpp>

namespace lx0 { namespace core { namespace lxvar_ns {

    namespace detail
    {
        class lxvar;

        void _convert(lxvar& v, glm::vec3& u);

        void _convert(lxvar& v, glgeom::point2f& p);
        void _convert(lxvar& v, glgeom::point3f& p);
        void _convert(lxvar& v, glgeom::point3d& p);
        void _convert(lxvar& v, glgeom::vector3f& u);
        void _convert(lxvar& v, glgeom::vector3d& u);
        void _convert(lxvar& v, glgeom::color3f& p);
        void _convert(lxvar& v, glgeom::color3d& p);

        void _convert(lxvar& v, glgeom::primitive_buffer& prim);
    }

}}

    lxvar lxvar_from    (const glgeom::vector3f& v);
    lxvar lxvar_from    (const glgeom::point3f& p);
}
