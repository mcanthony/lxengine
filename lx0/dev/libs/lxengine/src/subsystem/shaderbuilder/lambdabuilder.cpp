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
#include <lx0/core/log/log.hpp>
#include "lambdabuilder.hpp"

using namespace lx0;
using namespace lx0::subsystem::shaderbuilder_ns;
using namespace lx0::subsystem::shaderbuilder_ns::detail;

//===========================================================================//
// L O W - L E V E L   I M P S
//
// (move these to glgeom once complete!)
//===========================================================================//

static glm::vec2 mapper_spherical(const glm::vec3& positionOc, glm::vec2 scale)
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

static glm::vec2 mapper_cube(const glm::vec3& fragVertexOc, const glm::vec3& fragNormalOc, glm::vec2 scale)
{
    using namespace glm;

    // The cube projection is based off a theoretical unit cube                         
    // centered at 0,0,0.  However, the face mapping should range                       
    // from [0,1), not [-.5,.5).  Offset accordingly.                                   
    //                                                                                  
    vec2 offset = vec2(0.5, 0.5);                                                       
                                                                                        
    vec3 q = abs( normalize(fragNormalOc) );                                            
    vec2 uv;                                                                            
    if (q.z > q.y && q.z > q.x)                                                         
        uv = glm::vec2(fragVertexOc.x, fragVertexOc.y) + offset;                                                  
    else if (q.y > q.z && q.y > q.x)                                                    
        uv = glm::vec2(fragVertexOc.x, fragVertexOc.z) + offset;                                                  
    else                                                                                
        uv = glm::vec2(fragVertexOc.y, fragVertexOc.z) + offset;                                                  
                                                                                        
    return scale * uv;                                                                  
}

static glm::vec3 shade_checker (
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

static glm::vec3 shade_phong (
    int              unifLightCount,
    const glm::vec3* unifLightPosition,
    const glm::vec3* unifLightColor,
    const glm::vec3& fragVertexEc,
    const glm::vec3& fragNormalEc,
    const glm::vec3& ambient,
    const glm::vec3& diffuse,
    const glm::vec3& specular,
    float            specularEx
    )
{
    using namespace glm;

    // Since the eye is at <0,0,0>, the direction vector to the vertex and 
    // the vertex position in eye coordinates are equivalent.
    //
    vec3 N = normalize(fragNormalEc);
    vec3 V = -normalize(fragVertexEc);
       
    vec3 c = ambient;                        // ambient term
    
    for (int i = 0; i < unifLightCount; ++i)
    {
        vec3 L = unifLightPosition[i] - fragVertexEc;
        float d = length(L);
        L = normalize(L);
    
        float atten = 1;    // / (unifLightAtten[i].x + unifLightAtten[i].y * d  + unifLightAtten[i].x * d * d);
        vec3 lc = unifLightColor[i] * atten;
    
        c += diffuse * lc * max(dot(N,L), 0.0f);                   // diffuse term

        vec3 H = normalize(L + V);
        c += lc * specular * pow(max(dot(N,H), 0.0f), specularEx);  // specular term
    }
               
    return c;
}

//===========================================================================//

static
lxvar _value (lxvar param, lxvar node, const char* name)
{
    auto value = param.find(name);
    if (value.is_undefined())
        return node["input"][name][1];
    else
        return value;
}

LambdaBuilder::FunctionFloat
LambdaBuilder::_buildFloat (lxvar param)
{
    if (param.is_float())
    {
        float v = param;
        return [v](const Context&) { return v; };
    }
    else if (param.is_int())
    {
        int v = param;
        return [v](const Context&) { return float(v); };
    }

    lx_error("Could not build lambda for parameter");
    return FunctionFloat();
}


LambdaBuilder::FunctionVec2
LambdaBuilder::_buildVec2 (lxvar param)
{
    if (param.is_map())
    {
        std::string type = param["_type"].as<std::string>();
        auto& node = mNodes[type];

        if (type == "spherical")
        {
            auto scale = _buildVec2(_value(param, node, "scale"));
            return [scale](const Context& i) { 
                return mapper_spherical(i.fragVertexOc, scale(i)); 
            };
        }
        else if (type == "cube")
        {
            auto scale = _buildVec2(_value(param, node, "scale"));
            return [scale](const Context& i) { 
                return mapper_cube(i.fragVertexOc, i.fragNormalOc, scale(i)); 
            };
        }
        else
            lx_error("Unrecognized node type '%s'", type.c_str());
    }
    else if (param.is_array())
    {
        glm::vec2 v;
        v.x = param[0];
        v.y = param[1];
        return [v](const Context& i) { return v; };
    }

    lx_error("Could not build lambda for parameter");
    return FunctionVec2();
}

LambdaBuilder::FunctionVec3
LambdaBuilder::_buildVec3 (lxvar param)
{
    if (param.is_map())
    {
        std::string type = param["_type"].as<std::string>();
        auto& node = mNodes[type];

        if (type == "checker")
        {
            auto color0 = _buildVec3(_value(param, node, "color0"));
            auto color1 = _buildVec3(_value(param, node, "color1"));
            auto uv = _buildVec2(_value(param, node, "uv"));
            return [color0, color1, uv] (const Context& i) -> glm::vec3 {
                return shade_checker(color0(i), color1(i), uv(i));
            };
        }
        else if (type == "phong")
        {
            auto ambient    = _buildVec3(_value(param, node, "ambient"));
            auto diffuse    = _buildVec3(_value(param, node, "diffuse"));
            auto specular   = _buildVec3(_value(param, node, "specular"));
            auto specularEx = _buildFloat(_value(param, node, "specularEx"));
           
            return [ambient, diffuse, specular, specularEx](const Context& i) {
                return shade_phong(
                    i.unifLightCount,
                    i.unifLightCount ? &i.unifLightPosition[0] : nullptr,
                    i.unifLightCount ? &i.unifLightColor[0] : nullptr,
                    i.fragVertexEc,
                    i.fragNormalEc,
                    ambient(i), 
                    diffuse(i), 
                    specular(i), 
                    specularEx(i)
                );
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

    lx_error("Could not build lambda for parameter");
    return FunctionVec3();
}


//===========================================================================//

LambdaBuilder::ShadeFunction
LambdaBuilder::buildShader (lxvar graph)
{
    return _buildVec3(graph);
}
