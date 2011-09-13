//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

#include <functional>
#include <glgeom/glgeom.hpp>
#include <lx0/lxengine.hpp>
#include "../lambdabuilder.hpp"

using namespace glm;
using namespace lx0;
using namespace lx0::subsystem::shaderbuilder_ns::detail;
/*
static float compute_C (const vec2& dS)
{
    return -dS.x / dS.y;
}

static vec3 compute_D (const vec3& p, const vec3& q, const vec2& dS)
{
    return (q - p) / dS.x;
}

static vec3 normal_from_height_map (const glm::vec3& p, const glm::vec3& T, const glm::vec3& B, std::function<float (const glm::vec3&)> Sample)
{
    const float r = 0.01f;
    vec3 dx (2, 0, Sample(p - T * r) - Sample(p + T * r));
    vec3 dy (0, 2, Sample(p - B * r) - Sample(p + B * r));
    return normalize( cross( normalize(dx), normalize(dy) ) );
}

static glm::vec3 
computeBumpNormal (const glm::vec3 Pobj, const glm::vec3& Nobj, std::function<glm::vec2 (glm::vec3)> M, std::function<float (float, float)> Sample)
{
    using namespace glm;
    
    //
    // Choose two sample points in the local region
    //
    vec3 q0 = normalize( vec3(1, 1, 1) - Nobj );
    vec3 q1 = normalize( cross(Nobj, q0) );
    q0 = Pobj + q0 * 0.01f;
    q1 = Pobj + q1 * 0.01f;

    //
    // Compute the mapping coordinates at the points
    //
    const vec2 s = M(Pobj);
    const vec2 dS0 = M(q0) - s;
    const vec2 dS1 = M(q1) - s;

    //
    // Compute the constants used in the T,B,N calculations
    //
    const float C0 = compute_C(dS0);
    const vec3  D0 = compute_D(Pobj, q0, dS0);
    
    const float C1 = compute_C(dS1);
    const vec3  D1 = compute_D(Pobj, q1, dS1);

    //
    // Compute the tangent, binormal, and normal: i.e. the basis of tangent space
    //
    const float K = C0 / C1;
    const vec3  T = -(D1 * K + D0) / (1 - K);
    const vec3  B = (T - D1) / C1;
    const vec3  N = Nobj;

    //
    // Transform the tangent space normal into object space
    //
    vec3 Nbump = normal_from_height_map(Pobj, T, B, Sample);
    vec3 Nnew = Nbump.x * T + Nbump.y * B + Nbump.z * N;
    return Nnew;
}
*/





glm::vec3 
computeBumpNormal2 (const glm::vec3 Pobj, const glm::vec3& Nobj, std::function<float (glm::vec3)> F)
{
    //
    // Create a plane at the surface point with a normal directed towards the normal and an
    // arbitrary tangent and binormal vector;
    //
    glm::vec3 tangent;
    glm::vec3 binormal;
    glgeom::perpendicular_axes_smooth(glgeom::vector3_cast(Nobj), glgeom::vector3_cast(tangent), glgeom::vector3_cast(binormal));

    //
    // Sample the rate of change of the function F in terms of the tangent and binormal directions
    //
    const float r = 0.0025f;
    vec3 sT = 2 * r * tangent + Nobj * (F(Pobj + r * tangent) - F(Pobj - r * tangent));
    vec3 sB = 2 * r * binormal + Nobj * (F(Pobj + r * binormal) - F(Pobj - r * binormal));
    vec3 sN = normalize( cross( normalize(sT), normalize(sB) ) );

    return sN;
}

#if 0
LambdaBuilder::FunctionVec3 shade_bump (lxvar param, lxvar node)
{
    /*auto sample = _buildFloat(_value(param, node, "value"));
    auto uv     = _buildVec2(_value(param, node, "uv"));

    return [] (const ShaderBuilder::ShaderContext& ctx) -> glm::vec3
    {
        auto ctxCopy = ctx;
        auto M = [ctxCopy](glm::vec3 Pobj) -> glm::vec2()
        {
            ctxCopy.fragVertexOc = Pobj;
            auto result = uv(ctxCopy);
            ctxCopy.fragVertexOc = ctx.fragVertexOc;
        };
        return computeBumpNormal2(ctx.fragVertexOc, ctx.fragNormalOc, Mapper);
    };*/

#if 0
    /*
        The fundamental problem with implementing this is that the bump
        mapping function needs to access samples from outside the given
        point sample being traced.

        The bump map function should take a value of anything that can
        generate a single value: i.e. a totally generic function.  We
        then need to be able to resample as if the point in space were
        different.

        This is *possible* with the mapper, since we can (theoretically)
        modify the context to represent a different point being traced
        and then call the mapper again to get the new value.

        The problem is with the sampler: we need to get the adjacent 
        texels if we don't compute the normals once up front.  Computing
        them up front may make sense for a texture, but not for a 
        procedural.  

        Could we just choose another point in WS and have the sampler
        work from that?  (I.e. a texture2d would then invoke a mapper)

        The problem is that this converts between coordinate systems and
        back much more frequently than is necessary in the common case 
        (where the s,t sample coordinates could be nudged directly).

        if (sampler2d)
            node.evalDelta(ds, dt);
        if (sampler3d)
            node.evalDelta(ds, dt, 0);
        else
            node.eval()
    */

    auto sample = _buildFloat(_value(param, node, "value"));
    auto uv     = _buildVec2(_value(param, node, "uv"));

    return [] (const ShaderBuilder::ShaderContext& ctx) -> glm::vec3
    {
        auto ctxCopy = ctx;
        auto M = [ctxCopy](glm::vec3 Pobj) -> glm::vec2()
        {
            ctxCopy.fragVertexOc = Pobj;
            auto result = uv(ctxCopy);
            ctxCopy.fragVertexOc = ctx.fragVertexOc;
        };
        auto Sample = [ctxCopy](glm::vec3 Pobj) -> float
        {
            ctxCopy.fragVertexOc = Pobj;
            auto result = sample(ctxCopy);
            ctxCopy.fragVertexOc = ctx.fragVertexOc;
        };
        return computeBumpNormal(ctx.fragVertexOc, ctx.fragNormalOc, Mapper, Sample);
    };
#endif
}
#endif
