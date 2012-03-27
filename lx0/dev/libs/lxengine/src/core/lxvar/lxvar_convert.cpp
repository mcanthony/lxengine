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

#include <lx0/lxengine.hpp>
#include <lx0/core/lxvar/lxvar.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>
#include <glgeom/glgeom.hpp>

namespace lx0 { namespace core { namespace lxvar_ns {

    namespace detail
    {
        template <typename T>
        void _convert_any2f (lxvar& v, T& u)
        {
            lx_check_error(v.is_array());
            lx_check_error(v.size() == 2, "Cannot convert lxvar: array size is not 2 (%s:%d).", __FILE__, __LINE__);
            u[0] = v.at(0).as<float>();
            u[1] = v.at(1).as<float>();
        }

        template <typename T>
        void _convert_any3f (lxvar& v, T& u)
        {
            lx_check_error(v.is_array());
            lx_check_error(v.size() == 3, "Cannot convert lxvar: array size is not 3 (%s:%d).", __FILE__, __LINE__);
            u[0] = v.at(0).as<float>();
            u[1] = v.at(1).as<float>();
            u[2] = v.at(2).as<float>();
        }

        void _convert (lxvar& v,    glm::vec3& u)           { _convert_any3f(v, u); }
        void _convert (lxvar& v,    glgeom::point3f& u)     { _convert_any3f(v, u); }
        void _convert (lxvar& v,    glgeom::point3d& u)     { _convert_any3f(v, u); }
        void _convert (lxvar& v,    glgeom::vector3f& u)    { _convert_any3f(v, u); }
        void _convert (lxvar& v,    glgeom::vector3d& u)    { _convert_any3f(v, u); }
        void _convert (lxvar& v,    glgeom::color3f& u)     { _convert_any3f(v, u); }
        void _convert (lxvar& v,    glgeom::color3d& u)     { _convert_any3f(v, u); }

        void _convert (lxvar& v,    glgeom::point2f& u)     { _convert_any2f(v, u); }


        void _convert (lxvar& json, glgeom::primitive_buffer& prim)
        {
            using namespace glgeom;

            prim.type = json["type"].as<std::string>();
        
            //
            // Convert vertex data
            //
            lxvar vertexData = json["vertex"];
            if (vertexData.is_defined())
            {
                if (vertexData.has_key("positions"))
                {
                    prim.vertex.positions.resize( vertexData["positions"].size() );
                    for (auto i = 0u; i < prim.vertex.positions.size(); ++i)
                        prim.vertex.positions[i] = vertexData["positions"][i];
                }
                if (vertexData.has_key("uv"))
                {
                    std::vector<point2f> channel0;
                    channel0.resize( vertexData["uv"][0].size() );
                    for (auto i = 0u; i < prim.vertex.positions.size(); ++i)
                        channel0[i] = vertexData["uv"][0][i];
                    prim.vertex.uv.push_back(channel0);
                }
            }

            //
            // Convert indices
            //
            lxvar indices = json["indices"];
            if (indices.is_defined())
            {
                prim.indices.resize(indices.size());
                for (auto i = 0u; i < prim.indices.size(); ++i)
                    prim.indices[i] = (lx0::uint16)indices[i].as<int>();
            }
        }

    }
}}


    lxvar lxvar_from    (const glgeom::vector3f& v)
    {
        return lxvar(v.x, v.y, v.z);
    }
    
    
    lxvar lxvar_from    (const glgeom::point3f& v)
    {
        return lxvar(v.x, v.y, v.z);
    }

}
