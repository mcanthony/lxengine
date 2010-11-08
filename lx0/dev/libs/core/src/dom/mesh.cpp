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

#include <lx0/core.hpp>
#include <lx0/mesh.hpp>

namespace lx0 { namespace core {

    /*!
        Deserialize the input src var into a Mesh object
     */
    Mesh::Mesh (lxvar& v)
    {
        mFlags.mVertexNormals = false;

        // if src.type() == Mesh, set *this to a clone

        lx_check_error(v.isMap());
        lx_check_error(v.containsKey("type"));
        lx_check_error(v.containsKey("vertices"));
        lx_check_error(v.containsKey("faces"));

        // Deserialize the vertices list
        {
            lxvar lxverts = v.find("vertices");
            const int vertexCount = lxverts.size();
            mVertices.reserve(vertexCount);

            for (int i = 0; i < vertexCount; ++i)
            {
                lxvar src = lxverts.at(i);

                Vertex v;
                v.position.x = src.at(0).asFloat();
                v.position.y = src.at(1).asFloat();
                v.position.z = src.at(2).asFloat();

                if (src.size() >= 6)
                {
                    mFlags.mVertexNormals  = true;
                    v.normal.x = src.at(3).asFloat();
                    v.normal.y = src.at(4).asFloat();
                    v.normal.z = src.at(5).asFloat();
                }
                else
                {
                    static int warn_once = 0;
                    if (!warn_once++)
                        lx_warn("Meshes are always expected to provide vertex normals");
                }
                
                mVertices.push_back(v);
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
           
                q.index[0] = lxfaces.at(i).at(0).asInt();
                q.index[1] = lxfaces.at(i).at(1).asInt();
                q.index[2] = lxfaces.at(i).at(2).asInt();
                q.index[3] = (lxfaces.at(i).size() == 4)
                    ? lxfaces.at(i).at(3).asInt()
                    : -1;

                mFaces.push_back(q);
            }
        }
    }

        /*!
        This assume the mesh is centered about a local origin of 0,0,0.
     */
    float   
    Mesh::boundingRadius (void)
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
    vector3   
    Mesh::boundingVector (void)
    {
        vector3 v(0, 0, 0);
        for (auto it = mVertices.begin(); it != mVertices.end(); ++it)
        {
            vector3 u = abs_( cast<vector3&>(it->position) );
            for (int i = 0; i < 3; ++i)
            {
                if (u[i] > v[i])
                    v[i] = u[i];
            }
        }
        return v;
    }

}}