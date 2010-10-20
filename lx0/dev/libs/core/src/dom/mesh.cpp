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


#include <cassert>

#include <lx0/core.hpp>
#include <lx0/mesh.hpp>

namespace lx0 { namespace core {

    void 
    Mesh::deserialize(lxvar v)
    {
        lx_check_error(v.isMap());
        lx_check_error(v.containsKey("type"));
        lx_check_error(v.containsKey("vertices"));
        lx_check_error(v.containsKey("faces"));

        // Temporary limitation
        lx_check_error(v.find("type").equals("quad_list"));

        // Deserialize the vertices list
        {
            lxvar lxverts = v.find("vertices");
            const int vertexCount = lxverts.size();
            mVertices.reserve(vertexCount);
            for (int i = 0; i < vertexCount; ++i)
            {
                mVertices.push_back( asPoint3(lxverts.at(i)) );
            }
        }

        // Deserialize the face list
        {
            lxvar lxfaces = v.find("faces");
            const int faceCount = lxfaces.size();
            mFaces.reserve(faceCount);
            for (int i = 0; i < faceCount; ++i)
            {
                Mesh::Quad q;
                for (int j = 0; j < 4; ++j)
                    q.index[j] = lxfaces.at(i).at(j).asInt();
                mFaces.push_back(q);
            }
        }
    }

}}