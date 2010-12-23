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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

// Standard headers
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>
#include <iomanip>

// Lx0 headers
#include <lx0/core.hpp>
#include <lx0/lxvar.hpp>
#include <lx0/vector3.hpp>
#include <lx0/util.hpp>
#include <lx0/prototype/prototype.hpp>

using namespace lx0::core;

namespace lx0 { namespace prototype { 

    //! Computes the view matrix for the given camera
    /*!
        Note that matrix4 is has column-major order.  This the same as 
        DirectX and the transpose of the layout used by OpenGL.  
        Use glLoadTransposeMatrixf() if directly loading this matrix into
        OpenGL.
     */
    void    
    view_matrix (const Camera& camera, lx0::core::matrix4& viewMatrix)
    {
        lookAt(viewMatrix, camera.mPosition, camera.mTarget, camera.mWorldUp);
    }

    //!
    void    
    move_forward (Camera& camera, const vector3& up, float step)
    {
        lx_fatal();
    }

    //!
    void    
    move_up (Camera& camera, const vector3& up, float step)
    {
        lx_fatal();
    }

    //!
    void    
    move_side (Camera& camera, float step)
    {
        lx_fatal();
    }

    //! Rotate the camera horizontally about the world "up" axis
    void    
    rotate_horizontal (Camera& camera, float angle)
    {
        const vector3 view = camera.mTarget - camera.mPosition;
        const vector3 rotated = rotate(view, camera.mWorldUp, angle);
        camera.mTarget = camera.mPosition + rotated;
    }

    //! Rotate the camera vertically (about the rightward facing axis of the camera)
    void    
    rotate_vertical (Camera& camera, float angle)
    {
        const vector3 view  = camera.mTarget - camera.mPosition;
        const vector3 right = normalize( cross(view, camera.mWorldUp) );
        const vector3 rotated = rotate(view, right, angle);
        camera.mTarget = camera.mPosition + rotated;
    }

}}

