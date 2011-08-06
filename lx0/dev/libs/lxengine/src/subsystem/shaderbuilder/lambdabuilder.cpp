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
#include <glgeom/ext/mappers.hpp>
#include <glgeom/ext/patterns.hpp>

#include <lx0/core/log/log.hpp>
#include <lx0/prototype/misc.hpp>

#include "lambdabuilder.hpp"

using namespace lx0;
using namespace lx0::subsystem::shaderbuilder_ns;
using namespace lx0::subsystem::shaderbuilder_ns::detail;

//===========================================================================//
// L O W - L E V E L   I M P S
//
// (move these to glgeom once complete!)
//===========================================================================//

static glm::vec3 shade_normal (
    const glm::mat4& unifViewMatrix,
    const glm::vec3& fragNormalEc)
{
    using namespace glm;

    vec4 normalWc = vec4(fragNormalEc, 1.0) * unifViewMatrix;	
    vec3 N = abs(normalize(vec3(normalWc)));    	
    return N;
}

static glm::vec3 shade_normal2 (
    const glm::mat4& unifViewMatrix,
    const glm::vec3& fragNormalEc)
{
    using namespace glm;

    vec4 normalWc = vec4(fragNormalEc, 1.0) * unifViewMatrix;    	
    vec3 N = normalize(vec3(normalWc.x, normalWc.y, normalWc.z));
    vec3 A = abs(N);
    
    vec3 c = vec3(0,0,0);
    vec3 F = ceil(N);
    vec3 B = -floor(N);
    c += A.x * (F.x * vec3(1,0,0) + B.x * vec3(0,1,1)); 
    c += A.y * (F.y * vec3(0,1,0) + B.y * vec3(1,0,1)); 
    c += A.z * (F.z * vec3(0,0,1) + B.z * vec3(1,1,0));
    return c;
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

TextureCache::Image3fPtr  
TextureCache::acquire (const std::string& filename)
{
    auto it = mCache.find(filename);
    if (it == mCache.end())
    {
        std::shared_ptr<glgeom::image3f> spTexture( new glgeom::image3f );
        lx0::load_png(*spTexture, filename.c_str());
        mCache.insert(std::make_pair(filename, spTexture));
        
        return spTexture;
    }
    else
        return it->second;
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

/*
    Calling this a "sampler" is not wholly accurate at the moment, as it is
    returning an image - not some generalization of an image.  However, 
    eventually it may make sense to construct a sampler interface that 
    allows a RGBA image to be sampled as a RGB, etc.

    This has other limitations such as only supporting PNG as the moment.
 */
std::shared_ptr<glgeom::image3f>    
LambdaBuilder::_buildSampler (lxvar param)
{
    auto filename = param.as<std::string>();
    return mTextureCache.acquire(filename);
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
                return glgeom::mapper_spherical(i.fragVertexOc, scale(i)); 
            };
        }
        else if (type == "cube")
        {
            auto scale = _buildVec2(_value(param, node, "scale"));
            return [scale](const Context& i) { 
                return glgeom::mapper_cube(i.fragVertexOc, i.fragNormalOc, scale(i)); 
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
                return glgeom::pattern_checker(color0(i), color1(i), uv(i));
            };
        }
        else if (type == "wave")
        {
            auto color0 = _buildVec3(_value(param, node, "color0"));
            auto color1 = _buildVec3(_value(param, node, "color1"));
            auto width = _buildFloat(_value(param, node, "width"));
            auto uv = _buildVec2(_value(param, node, "uv"));   
            return [color0, color1, width, uv] (const Context& i) {
                return glgeom::pattern_wave(color0(i), color1(i), width(i), uv(i));
            };
        }
        else if (type == "spot")
        {
            auto color0 = _buildVec3(_value(param, node, "color0"));
            auto color1 = _buildVec3(_value(param, node, "color1"));
            auto radius = _buildFloat(_value(param, node, "radius"));
            auto uv = _buildVec2(_value(param, node, "uv"));   
            return [color0, color1, radius, uv] (const Context& i) {
                return glgeom::pattern_spot(color0(i), color1(i), radius(i), uv(i));
            };
        }
        else if (type == "normal")
        {
            return [](const Context& i) {
                return shade_normal (i.unifViewMatrix, i.fragNormalEc);
            };
        }
        else if (type == "normal2")
        {
            return [](const Context& i) {
                return shade_normal2 (i.unifViewMatrix, i.fragNormalEc);
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
        else if (type == "texture2d")
        {
            auto spTexture = _buildSampler(param["texture"]);
            auto uv = _buildVec2(_value(param, node, "uv")); 

            return [spTexture, uv] (const Context& i) -> glm::vec3 {
                glm::vec2 uv2 = uv(i);
                return spTexture->get(
                    uv2.x * float(spTexture->width() - 1) + 0.5f,
                    uv2.y * float(spTexture->height() - 1) + 0.5f
                ).vec;
            };
        }
        else if (type == "lightgradient")
        {
            auto spTexture = _buildSampler(param["texture"]);

            return [spTexture](const Context& ctx) -> glm::vec3 {

                glm::vec3 color(0,0,0);
                for (int i = 0; i < ctx.unifLightCount; ++i)
                {
                    // 
                    // L = unit vector from light to intersection point; the "incidence vector" I is the
                    //     vector pointing in the opposite direction of L.
                    // N = surface normal at the point of intersection
                    //
                    const glm::vec3  L  (glm::normalize(ctx.unifLightPosition[i] - ctx.fragVertexEc)); 
                    const glm::vec3& N  (ctx.fragNormalEc);
                    const float      NdotL ( glm::dot(N, L) );
                
                    const float diffuseSample = (NdotL + 1.0f) / 2.0f;
                    color += spTexture->get(int(diffuseSample * (spTexture->width() - 1)), 0).vec * ctx.unifLightColor[i];
                }
                return color;
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
