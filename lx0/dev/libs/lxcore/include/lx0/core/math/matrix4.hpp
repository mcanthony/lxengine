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

#include <lx0/core/base/cast.hpp>
#include <lx0/core/math/vector3.hpp>
#include <glgeom/glgeom.hpp>

namespace lx0 { namespace core {    

    /*!
        The matrix class itself is a generic 4x4 matrix.

        Elements are indexed via (Row, Column) 0-based indexing.  This adopts
        the traditional mathematical notion of row then column, but the programming
        convention of 0-based indices.  

        The support operations for multiplying and transforming vectors
        assume a row-vector based model where vectors are pre-multiplied with 
        the matrix.
                     [ a  b  c  d ]   [ xa + ye + zi + txw ]
        [ x y z w ]  [ e  f  g  h ] = [ xb + yf + zj + tyw ]
                     [ i  j  k  l ]   [ xc + yg + zk + tzw ]
                     [ tx ty tz 1 ]   [ xd + yh + zl +   w ]
     */
    class matrix4
    {
    public:
        inline float  operator()     (int row, int col) const { return column[col][row]; }
        inline float& operator()     (int row, int col)       { return column[col][row]; }

        union
        {  
            struct
            {
                tuple4 column[4];
            };
            float   elem[4][4];
            float   data[16];
        };
    };

    void    set_identity        (matrix4& m);
    void    set_translation     (matrix4& m, float tx, float ty, float tz);
    void    set_scale           (matrix4& m, float sx, float sy, float sz);
    void    copy                (matrix4& a, const matrix4& b);
    void    mul                 (matrix4& c, const matrix4& a, const matrix4& b);
    void    transpose           (matrix4& a, const matrix4& b);


    void    mul                 (glgeom::vector3f& u, const glgeom::vector3f& v, const matrix4& m);

    void    setOrthonormalBasis (matrix4& mat, const glgeom::vector3f& x, const glgeom::vector3f& y, const glgeom::vector3f& z, const glgeom::point3f& origin);
    void    lookAt              (matrix4& mat, const glgeom::point3f& position, const glgeom::point3f& target, const glgeom::vector3f& referenceUpAxis);

    inline glgeom::vector3f operator*    (const glgeom::vector3f& v, const matrix4& m);
    inline matrix4 operator*    (const matrix4& a, const matrix4& b) { matrix4 c; mul(c, a, b); return c; }

}};