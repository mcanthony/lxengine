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
#include "lamdba_fragments/lambda_fragments.hpp"

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
    const ShaderBuilder::ShaderContext& ctx,
    const glm::vec3& ambient,
    const glm::vec3& diffuse,
    const glm::vec3& specular,
    float            specularEx,
    float            reflectivity,
    float            opacity,
    const glm::vec3& normalDiffuse
    )
{
    using namespace glm;

    // Since the eye is at <0,0,0>, the direction vector to the vertex and 
    // the vertex position in eye coordinates are equivalent.
    //
    vec3 N = normalize(normalDiffuse);
    vec3 V = -normalize(ctx.fragVertexEc);
       
    vec3 c = ambient;                        // ambient term
    
    for (int i = 0; i < ctx.unifLightCount; ++i)
    {
        vec3 L = ctx.unifLightPosition[i] - ctx.fragVertexEc;
        float d = length(L);
        L = normalize(L);
    
        float atten = 1;    // / (unifLightAtten[i].x + unifLightAtten[i].y * d  + unifLightAtten[i].x * d * d);
        vec3 lc = ctx.unifLightColor[i] * atten;
    
        c += diffuse * lc * max(dot(N,L), 0.0f);                   // diffuse term

        vec3 H = normalize(L + V);
        c += lc * specular * pow(max(dot(N,H), 0.0f), specularEx);  // specular term
    }
    
    if (reflectivity > 0.0f)
    {
        vec3 NWc = glm::normalize(ctx.fragNormalWc);

        // Compute reflection ray, nudge it out, cast, blend
        vec3 R = -glm::reflect(glm::normalize(ctx.unifEyeWc - ctx.fragVertexWc), NWc);
        glgeom::ray3f ray(ctx.fragVertexWc + R * 1e-3f, R);
        vec3 cr = ctx.traceFunc(ray).vec;
        c = glm::mix(c, cr, reflectivity);
    }

    //
    // Work-in-progress opacity: need index of refraction, shadowing, and Beer's Law accounted for
    //
    if (opacity < 1.0f)
    {
        vec3 D = (ctx.fragVertexWc - ctx.unifEyeWc);
        glgeom::ray3f ray(ctx.fragVertexWc + D * 1e-3f, D);
        vec3 co = ctx.traceFunc(ray).vec;
        c = glm::mix(co, c, opacity);
    }

    return c;
}

//===========================================================================//

void
TextureCache::add (const std::string& id, Image3fPtr spImage)
{
    mCache.insert(std::make_pair(id, spImage));
}

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

void
Cache::add (const std::string& id, Image3fPtr spImage)
{
    mCache.insert(std::make_pair(id, spImage));
}

Cache::Image3fPtr  
Cache::acquire (const std::string& id)
{
    auto it = mCache.find(id);
    if (it == mCache.end())
        return Cache::Image3fPtr();
    else
        return it->second;
}



static
lxvar _value (lxvar& param, lxvar& node, const char* name)
{
    auto value = param.find(name);
    if (value.is_undefined())
        return node["input"][name][1];
    else
        return value;
}

//===========================================================================//

LambdaBuilder::LambdaBuilder (NodeMap& nodeMap) 
    : mNodes (nodeMap)
{
    _init();
}

void LambdaBuilder::_init()
{
    mFuncs3f["cubemap"] = [&](lxvar param, lxvar node) -> FunctionVec3 
    {
        auto spTexture = _buildSamplerCube(param["cubemap"]);
        auto intensity = _buildFloat(_value(param, node, "intensity"));

        return [intensity, spTexture] (const Context& ctx) -> glm::vec3 {
            glm::vec3 uvw = glm::normalize( ctx.fragNormalOc );
            return intensity(ctx) * spTexture->get(uvw).vec;
        };
    };

    mFuncs3f["texture2d"] = [&](lxvar param, lxvar node) -> FunctionVec3
    {
        auto spTexture = _buildSampler2d(param["texture"]);
        auto uv = _buildVec2(_value(param, node, "uv"));
        auto intensity = _buildFloat(_value(param, node, "intensity"));

        return [spTexture, uv, intensity] (const Context& ctx) -> glm::vec3 {
            glm::vec2 uv2 = uv(ctx);
            uv2.x = fmodf(uv2.x, 1.0f);
            uv2.y = fmodf(uv2.y, 1.0f);

            if (uv2.x < 0) uv2.x = 1.0f + uv2.x;
            if (uv2.y < 0) uv2.y = 1.0f + uv2.y;
            
            return intensity(ctx) * spTexture->get(
                int( uv2.x * float(spTexture->width() - 1) + 0.5f ),
                int( uv2.y * float(spTexture->height() - 1) + 0.5f )
            ).vec;
        };
    };

    mFuncs3f["checker"] = [&](lxvar param, lxvar node) -> FunctionVec3
    {
        auto color0 = _buildVec3(_value(param, node, "color0"));
        auto color1 = _buildVec3(_value(param, node, "color1"));
        auto uv = _buildVec2(_value(param, node, "uv"));
        return [color0, color1, uv] (const Context& i) -> glm::vec3 {
            return glgeom::pattern_checker(color0(i), color1(i), uv(i));
        };
    };

    mFuncs3f["wave"] = [&](lxvar param, lxvar node) -> FunctionVec3
    {
        auto color0 = _buildVec3(_value(param, node, "color0"));
        auto color1 = _buildVec3(_value(param, node, "color1"));
        auto width = _buildFloat(_value(param, node, "width"));
        auto uv = _buildVec2(_value(param, node, "uv"));   
        return [color0, color1, width, uv] (const Context& i) {
            return glgeom::pattern_wave(color0(i), color1(i), width(i), uv(i));
        };
    };

    mFuncs3f["spot"] = [&](lxvar param, lxvar node) -> FunctionVec3
    {
        auto color0 = _buildVec3(_value(param, node, "color0"));
        auto color1 = _buildVec3(_value(param, node, "color1"));
        auto radius = _buildFloat(_value(param, node, "radius"));
        auto uv = _buildVec2(_value(param, node, "uv"));   
        return [color0, color1, radius, uv] (const Context& i) {
            return glgeom::pattern_spot(color0(i), color1(i), radius(i), uv(i));
        };
    };

    mFuncs3f["spot_dim"] = [&](lxvar param, lxvar node) -> FunctionVec3
    {
        auto color0 = _buildVec3(_value(param, node, "color0"));
        auto color1 = _buildVec3(_value(param, node, "color1"));
        auto radius = _buildFloat(_value(param, node, "radius"));
        auto uv = _buildVec2(_value(param, node, "uv"));   
        return [color0, color1, radius, uv] (const Context& i) {
            return glgeom::pattern_spot_dim(color0(i), color1(i), radius(i), uv(i));
        };
    };


    mFuncs3f["normal"] = [&](lxvar param, lxvar node) -> FunctionVec3
    {
        return [](const Context& i) {
            return shade_normal (i.unifViewMatrix, i.fragNormalEc);
        };
    };

    mFuncs3f["normal2"] = [&](lxvar param, lxvar node) -> FunctionVec3
    {
        return [](const Context& i) {
            return shade_normal2 (i.unifViewMatrix, i.fragNormalEc);
        };
    };

    mFuncs3f["phong"] = [&](lxvar param, lxvar node) -> FunctionVec3
    {
        auto ambient      = _buildVec3(_value(param, node, "ambient"));
        auto diffuse      = _buildVec3(_value(param, node, "diffuse"));
        auto specular     = _buildVec3(_value(param, node, "specular"));
        auto specularEx   = _buildFloat(_value(param, node, "specularEx"));
        auto reflectivity = _buildFloat(_value(param, node, "reflectivity"));
        auto opacity      = _buildFloat(_value(param, node, "opacity"));
        auto normalDiffuse= _buildVec3(_value(param, node, "normal_diffuse"));
               
        return [ambient, diffuse, specular, specularEx, reflectivity, opacity, normalDiffuse](const Context& ctx) {
            return shade_phong(
                ctx,
                ambient(ctx), 
                diffuse(ctx), 
                specular(ctx), 
                specularEx(ctx),
                reflectivity(ctx),
                opacity(ctx),
                normalDiffuse(ctx)
            );
        };
    };  

    mFuncs3f["lightgradient"] = [&](lxvar param, lxvar node) -> FunctionVec3
    {
        auto spTexture = _buildSampler2d(param["texture"]);

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
    };

    // Generate a normal from a height map function
    mFuncs3f["bump"] = [&](lxvar param, lxvar node) -> FunctionVec3
    {
        auto value = _buildFloat(_value(param, node, "value"));
        auto intensity = _buildFloat(_value(param, node, "intensity"));

        return [value, intensity] (const Context& ctx) -> glm::vec3 {
            auto ctx2 = ctx;
            auto value2 = value;

            auto heightFunc = [&ctx2, value2](const glm::vec3& Pobj) -> float {
                ctx2.fragVertexOc = Pobj;
                return value2(ctx2);
            };
            return computeBumpNormal2(ctx.fragVertexOc, ctx.fragNormalOc, intensity(ctx), heightFunc);
        };
    };
}

void LambdaBuilder::addTexture (std::string id, std::shared_ptr<glgeom::image3f> image)
{
    mTextureCache.add(id, image);
}

void LambdaBuilder::addTexture (std::string id, std::shared_ptr<glgeom::cubemap3f> image)
{
    mCubemapCache.add(id, image);
}

/*
    Calling this a "sampler" is not wholly accurate at the moment, as it is
    returning an image - not some generalization of an image.  However, 
    eventually it may make sense to construct a sampler interface that 
    allows a RGBA image to be sampled as a RGB, etc.

    This has other limitations such as only supporting PNG as the moment.
 */
std::shared_ptr<glgeom::image3f>    
LambdaBuilder::_buildSampler2d (lxvar param)
{
    auto filename = param.as<std::string>();
    return mTextureCache.acquire(filename);
}

std::shared_ptr<glgeom::cubemap3f>
LambdaBuilder::_buildSamplerCube (lxvar param)
{
    auto id = param.as<std::string>();
    return mCubemapCache.acquire(id);
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
    else if (param.is_map())
    {
        std::string type = param["_type"].as<std::string>();
        auto& node = mNodes[type];

        auto it = mFuncs3f.find(type);
        if (it != mFuncs3f.end())
        {
            auto func = (it->second)(param, node);
            return [=](const Context& ctx) { return func(ctx).x; };
        }
    }

    throw lx_error_exception("Could not build lambda for parameter");
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
            throw lx_error_exception("Unrecognized node type '%s'", type.c_str());
    }
    else if (param.is_array())
    {
        glm::vec2 v;
        v.x = param[0];
        v.y = param[1];
        return [v](const Context& i) { return v; };
    }

    throw lx_error_exception("Could not build lambda for parameter");
    return FunctionVec2();
}

LambdaBuilder::FunctionVec3
LambdaBuilder::_buildVec3 (lxvar param)
{
    if (param.is_map())
    {
        std::string type = param["_type"].as<std::string>();
        auto& node = mNodes[type];

        auto it = mFuncs3f.find(type);
        if (it != mFuncs3f.end())
            return (it->second)(param, node);
    }
    else if (param.is_array())
    {
        glm::vec3 v;
        v.x = param[0];
        v.y = param[1];
        v.z = param[2];
        return [v](const Context& i) { return v; };
    }
    else if (param.is_string())
    {
        auto s = param.as<std::string>();

             if (s == "fragNormalEc") return [](const Context& ctx) { return ctx.fragNormalEc; };
        else if (s == "fragNormalOc") return [](const Context& ctx) { return ctx.fragNormalOc; };
        else
            throw lx_error_exception("Unrecognized constant '%s' used in graph", s.c_str());
    }

    throw lx_error_exception("Could not build lambda for parameter");
    return FunctionVec3();
}


//===========================================================================//

LambdaBuilder::ShadeFunction
LambdaBuilder::buildShader (lxvar graph)
{
    return _buildVec3(graph);
}
