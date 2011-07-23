//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

    MIT License: http://www.opensource.org/licenses/mit-license.php

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

#include <string>
#include <lx0/core/lxvar/lxvar.hpp>
#include <glgeom/ext/primitive_buffer.hpp>
#include <glgeom/prototype/std_lights.hpp>

class texture_handle
{
public:
    std::string name;
    
    std::string format;
    std::function< std::shared_ptr<std::istream>() > callback;
};

//===========================================================================//
//    G L G E O M   E X T E N S I O N S 
//===========================================================================//

class instance
{
public:
    instance() 
        : spPrimitive( new glgeom::primitive_buffer )
        , spTransform( new glm::mat4 )
    {}
    instance(const instance& that) 
        : material (that.material)
        , spPrimitive (that.spPrimitive)
        , spTransform (that.spTransform)
    {}

    lx0::lxvar                                material;      
    std::shared_ptr<glgeom::primitive_buffer> spPrimitive;
    std::shared_ptr<glgeom::mat4f>            spTransform;
};

class scene_group
{
public:
    void merge(scene_group& that);

    std::vector<glgeom::point_light_f>  lights;
    std::vector<instance>               instances;
    std::vector<texture_handle>         textures;
};

template <typename T>
void append_to (std::vector<T>& dst, std::vector<T>& v)
{
    dst.reserve( dst.size() + v.size() );
    for (auto it = v.begin(); it != v.end(); ++it)
        dst.push_back(*it);
}

inline void
scene_group::merge (scene_group& that)
{
    append_to(instances, that.instances);
    append_to(lights, that.lights);
    append_to(textures, that.textures);
}
