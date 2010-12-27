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
#include <lx0/core/core.hpp>
#include <lx0/core/data/lxvar.hpp>
#include <lx0/core/math/vector3.hpp>
#include <lx0/core/util/util.hpp>
#include <lx0/prototype/prototype.hpp>

using namespace lx0::core;

namespace lx0 { namespace prototype { 

    /*!
        Returns the unnormalized vector between the camera position and target.
     */
    vector3
    view_vector (const Camera& camera)
    {
        lx_check_error( !is_zero_length(camera.mTarget - camera.mPosition) );
        return camera.mTarget - camera.mPosition;
    }

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
    move_forward (Camera& camera, float step)
    {
        const vector3 view = normalize( view_vector(camera) ) * step;
        camera.mTarget += view;
        camera.mPosition += view;
    }

    //!
    void    
    move_up (Camera& camera, float step)
    {
        lx_check_error( is_unit_length( camera.mWorldUp ) );

        const vector3 view  = normalize( view_vector(camera) );
        const vector3 right = normalize( cross(view, camera.mWorldUp) );
        const vector3 viewUp = normalize( cross(right, view) );

        camera.mTarget += viewUp * step;
        camera.mPosition += viewUp * step;
    }

    //!
    void    
    move_vertical (Camera& camera, float step)
    {
        lx_check_error( is_unit_length( camera.mWorldUp ) );

        camera.mTarget += camera.mWorldUp * step;
        camera.mPosition += camera.mWorldUp * step;
    }

    //!
    void    
    move_right (Camera& camera, float step)
    {
        const vector3 view  = normalize( view_vector(camera) );
        const vector3 right = normalize( cross(view, camera.mWorldUp) );
        camera.mTarget += right * step;
        camera.mPosition += right * step;
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

