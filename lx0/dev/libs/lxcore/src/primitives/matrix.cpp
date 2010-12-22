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

#include "lx0/core.hpp"
#include "lx0/matrix.hpp"

namespace lx0 { namespace core {

    /*!
        This function creates a matrix that can transform from one coordinate
        system to another.  The input x, y, and z vectors are the new axes in
        terms of the old coordinate system and the origin is the position in
        the old coordinate system that will become 0,0,0 in the new system.
     */
    void
    setOrthonormalBasis (matrix4& mat, const vector3& x, const vector3& y, const vector3& z, const point3& origin)
    {
        // Check that the axes are normalized (i.e. the "normal" part of orthonormal)
        lx_check_error( is_unit_length(x), "X axis is not unit length" );
        lx_check_error( is_unit_length(y), "Y axis is not unit length" );
        lx_check_error( is_unit_length(z), "Z axis is not unit length" );

        // Check that the axes are orthogonal (i.e. the "ortho" part of orthonormal)
        lx_check_error( is_orthogonal(x, y) );
        lx_check_error( is_orthogonal(y, z) );
        lx_check_error( is_orthogonal(x, z) );

        mat.column[0][0] = x.x;
        mat.column[0][1] = x.y;
        mat.column[0][2] = x.z;
        mat.column[0][3] = 0;

        mat.column[1][0] = y.x;
        mat.column[1][1] = y.y;
        mat.column[1][2] = y.z;
        mat.column[1][3] = 0;

        mat.column[2][0] = z.x;
        mat.column[2][1] = z.y;
        mat.column[2][2] = z.z;
        mat.column[2][3] = 0;

        // A 4x4 Matrix effectively acts as 3x3 rotation matrix followed by an offset.
        // In other words, the translation term is added in after the rotation occurs.
        // Therefore, the offset in the matrix itself must be in terms of the new 
        // coordinate system.

        const vector3& v = cast<const vector3&>(origin);
        point3 t;
        t.x = dot(x, v);
        t.y = dot(y, v);
        t.z = dot(z, v);

        mat.column[3][0] = t.x;
        mat.column[3][1] = t.y;
        mat.column[3][2] = t.z;
        mat.column[3][3] = 1;
    }

    /*!
        Create a orthonormal basis (i.e. a new coordinate system) based at
        "position" with the -Z axis pointing toward target and oriented
        such that the angle between the Y axis and reference up vector is
        minimized.

        The choice of -Z as direction of the view vector is arbitrary, but
        is consistent with the OpenGL.

        Adapted from the Mesa3D gluLookAt source code (project.c).
    */
    void 
    lookAt (matrix4& mat, const point3& position, const point3& target, const vector3& eyeUp)
    {
        lx_check_error( !is_zero_length( target - position ),  "Position and target are the same");

        //
        // Compute the eye coordinate system.
        //
        // Note that these vectors are in terms of how the "eye" perceives the world:
        // i.e. the eye has a notion of "forward" and "right", not merely X, Y, and Z.
        //
        const vector3 forward = normalize(target - position);
        const vector3 right   = normalize( cross(forward, eyeUp) );
        const vector3 up      = normalize( cross(right, forward) );

        lx_check_error( !is_codirectional(forward, eyeUp), "Forward vector looking straight up.  Cannot compute matrix correctly.");
        lx_check_error( is_unit_length(forward), "Forward vector incorrect: are position and target the same?" );
        lx_check_error( is_unit_length(right),   "Error.  Is the viewer looking straight up (eyeUp == forward)?" );
        lx_check_error( is_unit_length(up), "Expected vector to be unit length.  Length squared = %f", length_squared(up));

        //
        // Now map the eye vectors to the X/Y/Z basis.   The mapping here is essentially
        // arbitrary (by analogy, it is equally mathematically correct to construct a 
        // virtual world where gravity run in the -Y direction as it is to build one where it runs in
        // the -Z direction, as long the other axes are dealt with accordingly).  
        // The system chosen here is consistent with OpenGL's definition of eye coordinates.
        //
        // This call will set X=right, Y=up, -Z=forward, and the origin is at position.
        // This matrix now can transforms points to this new coordinate system relative
        // to the eye position.
        //
        setOrthonormalBasis(mat, right, up, -forward, -position);
    }


}}
