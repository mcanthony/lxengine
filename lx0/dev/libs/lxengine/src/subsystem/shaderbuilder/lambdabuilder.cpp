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

#include <glgeom/glgeom.hpp>
#include "lambdabuilder.hpp"

using namespace lx0;
using namespace lx0::subsystem::shaderbuilder_ns;
using namespace lx0::subsystem::shaderbuilder_ns::detail;

//===========================================================================//
// L O W - L E V E L   I M P S
//
// (move these to glgeom once complete!)
//===========================================================================//

static glm::vec2   spherical(const glm::vec3& positionOc, glm::vec2 scale)
{
    // Convert to spherical coordinates.  Note in this definition                       
    // theta runs along the z axis and phi is in the xy plane.                          
    float r = glm::length(positionOc);                                                     
    float phi = atan2(positionOc.y, positionOc.x);                                   
    float theta = acos(positionOc.z / r);                                             
                                                                                            
    // Normalize to [0-1) range                                                         
    glm::vec2 uv;                                                              
    uv.x = (phi + glgeom::pi().value) / (2 * glgeom::pi().value);                                                       
    uv.y = theta / glgeom::pi().value;                                                                  
    return uv * scale;      
}

static glm::vec3   checker (
    const glm::vec3& color0, 
    const glm::vec3& color1, 
    const glm::vec2& uv) 
{
    glm::vec2 t = glm::abs( glm::fract(uv) );                                                        
    glm::ivec2 s = glm::ivec2(glm::trunc(glm::vec2(2,2) * t));                                                    
                                                                                        
    if ((s.x + s.y) % 2 == 0)                                                         
        return color0;                                                              
    else                                                                            
        return color1;    
}

//===========================================================================//

typedef LambdaBuilder::FunctionFloat FunctionFloat;
typedef LambdaBuilder::FunctionVec2  FunctionVec2;
typedef LambdaBuilder::FunctionVec3  FunctionVec3;
typedef LambdaBuilder::FunctionVec4  FunctionVec4;
typedef LambdaBuilder::Context       Context;

static FunctionFloat buildFloat (lxvar param);
static FunctionVec2  buildVec2 (lxvar param);
static FunctionVec3  buildVec3 (lxvar param);
static FunctionVec4  buildVec4 (lxvar param);

static 
FunctionVec3
buildVec3 (lxvar param)
{
    if (param.is_map())
    {
        if (param["_type"] == "checker")
        {
            auto color0 = buildVec3(param["color0"]);
            auto color1 = buildVec3(param["color1"]);
            auto uv = buildVec2(param["uv"]);
            return [color0, color1, uv] (const Context& i) -> glm::vec3 {
                return checker(color0(i), color1(i), uv(i));
            };
        }
    }
    else if (param.is_array())
    {
        glm::vec3 v;
        v.x = param[0];
        v.y = param[1];
        v.z = param[2];
        return [v](const Context& i) { return v; };
    }

    return FunctionVec3();
}

static
FunctionVec2
buildVec2 (lxvar param)
{
    if (param.is_map())
    {
        if (param["_type"] == "spherical")
        {
            auto scale = buildVec2(param["scale"]);
            return [scale](const Context& i) { 
                return spherical(i.fragVertexOc, scale(i)); 
            };
        }
    }
    else if (param.is_array())
    {
        glm::vec2 v;
        v.x = param[0];
        v.y = param[1];
        return [v](const Context& i) { return v; };
    }
    return 
        FunctionVec2();
}

//===========================================================================//

LambdaBuilder::ShadeFunction
LambdaBuilder::buildShader (lxvar graph)
{
    return buildVec3(graph);
}
