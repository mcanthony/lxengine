//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010 athile@athile.net (http://www.athile.net)

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

#include <lx0/point3.hpp>
#include <lx0/vector3.hpp>
#include <lx0/matrix.hpp>

namespace lx0 { namespace prototype {

    struct Camera
    {
        lx0::core::point3  mPosition;
        lx0::core::point3  mTarget;
        lx0::core::vector3 mWorldUp;        //! Reference vector for the "up" direction in the world
    };

            void                view_matrix         (const Camera& camera, lx0::core::matrix4& viewMatrix);
    inline  lx0::core::matrix4  view_matrix         (const Camera& camera) { lx0::core::matrix4 m; view_matrix(camera, m); return m; } 

            void                move_forward        (Camera& camera, float step);
            void                move_up             (Camera& camera, float step);
            void                move_side           (Camera& camera, float step);
            void                rotate_horizontal   (Camera& camera, float angle);
            void                rotate_vertical     (Camera& camera, float angle);

}}
