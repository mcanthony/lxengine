//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

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

#include <glgeom/glgeom.hpp>
#include <glgeom/ext/primitive_buffer.hpp>
#include <glgeom/prototype/std_lights.hpp>

class instance
{
public:
    std::string              material;      //<! name of associated material
    glgeom::primitive_buffer primitive;
    glgeom::mat4f            transform;
};

class scene_group
{
public:
    void merge(scene_group& that);

    std::vector<glgeom::point_light_f>  lights;
    std::vector<instance>       instances;
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
}

class Tes3Imp;

class Tes3Io
{
public:
            Tes3Io();
            ~Tes3Io();
    void    initialize  (const char* path);
    void    cell        (const char* id, scene_group& group);

protected:
    Tes3Imp*    mpImp;
};
