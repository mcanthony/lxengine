//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

#include <cassert>

#include <lx0/lxengine.hpp>
#include <lx0/engine/mesh.hpp>
#include <glgeom/glgeom.hpp>

namespace lx0 { namespace core {

    Mesh::Mesh (void)
    {
    }

        /*!
        This assume the mesh is centered about a local origin of 0,0,0.
     */
    float   
    Mesh::boundingRadius (void) const
    {
        float rmax = 0.0f;
        for (auto it = mVertices.begin(); it != mVertices.end(); ++it)
        {
            float r = distance_to_origin_squared(it->position);
            if (r > rmax)
                rmax = r;
        }
        return sqrt(rmax);
    }


    /*!
        This assume the mesh is centered about a local origin of 0,0,0.
     */
    glgeom::vector3f  
    Mesh::boundingVector (void) const
    {
        glgeom::vector3f v(0, 0, 0);
        for (auto it = mVertices.begin(); it != mVertices.end(); ++it)
        {
            const auto u = glm::abs(it->position.vec);
            for (int i = 0; i < 3; ++i)
            {
                if (u[i] > v[i])
                    v[i] = u[i];
            }
        }
        return v;
    }

    
    float               
    Mesh::maxExtentScale (lxvar value) const
    {
        if (value.isFloat() || value.isInt())
        {
            float maxExtent = *value;
            if (maxExtent > 0.0f)
            {
                auto extents = 2 * boundingVector();
                float f = maxExtent / std::max(extents.x, std::max(extents.y, extents.z));
                return f;
            }
            else
                lx_warn("Invalid max_extent value %f", maxExtent);
        }
    
        // Empty value means use the original scale
        lx_check_error( value.isUndefined() );
        return 1.0f;
    }

}}