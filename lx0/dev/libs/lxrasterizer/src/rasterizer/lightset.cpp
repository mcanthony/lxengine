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
#include <lx0/extensions/rasterizer.hpp>
#include <lx0/extensions/rasterizer/lightset.hpp>

using namespace lx0;
using namespace glgeom;

extern OpenGlApi3_2* gl;

static 
void
_selectLights (std::vector<LightPtr>& all, RasterizerGL::ActiveLights& active, RasterizerGL* pRasterizer)
{
    const int kMaxActive = 8;

    if (!all.empty())
    {
        struct Entry
        {
            size_t          index;
            float           distance_squared;
            glgeom::point3f positionEc;
        };
        std::list<Entry> closest;

        const glm::mat4& viewMatrix = pRasterizer->mContext.spCamera->viewMatrix;
        auto cameraPos = glm::inverse(viewMatrix) * glgeom::point3f(0,0,0);

        auto transformed = (viewMatrix * glgeom::point3f(1, 1, 1)).vec;
        auto ratio = length( glm::vec3(1, 1, 1)) / length(transformed);

        //
        // Loop through all the lights and select the N closest.
        //
        for (size_t i = 0; i < all.size(); ++i)
        {
            //
            // Is the camera out the light's area of influence?
            //
            float distanceWc2 = distance_squared(cameraPos, all[i]->position);            
            if (all[i]->radius * all[i]->radius >= distanceWc2)
            {
                Entry e;
                e.index = i;
                e.positionEc = viewMatrix * all[i]->position;
                e.distance_squared = distance_to_origin_squared(e.positionEc);

                size_t j = 0;
                auto jt = closest.rbegin();
                while (jt != closest.rend() && j < kMaxActive)
                {
                    if (e.distance_squared < jt->distance_squared)
                        break;
                     ++jt, ++j;
                }
                if (j  < kMaxActive)
                {
                    closest.insert(jt.base(), e);
                    if (closest.size() > kMaxActive)
                        closest.pop_front();
                }
            }
        }

        //
        // Now that the closest have been selected, push the light data into the
        // active set arrays.
        //
        active.positionsEc.reserve(closest.size());
        active.colors.reserve(closest.size());
        for (auto it = closest.begin(); it != closest.end(); ++it)
        {
            const auto& light = *all[it->index].get();

            active.positionsEc.push_back( it->positionEc );
            active.colors.push_back( light.color );
            active.attenuation.push_back( light.attenuation * glm::vec3(1, ratio, ratio * ratio) );

            active.glowRadius.push_back( light.glow.radius );
            active.glowMultiplier.push_back( light.glow.multiplier );
            active.glowExponent.push_back( light.glow.exponent );
        }
    }

    assert(active.positionsEc.size() <= kMaxActive);
}

void LightSet::activate(RasterizerGL* pRasterizer)
{
    GLint mId;
    gl->getIntegerv(GL_CURRENT_PROGRAM, &mId);

    //
    // This is a temporary bit of code to select only the four closest
    // lights.  The number 4 is a hard-coded value based on the current
    // phong shader implementation.  Obviously, this should be generalized.
    //
    auto it = pRasterizer->mContext.frame.activeLights.find(this);
    if (it == pRasterizer->mContext.frame.activeLights.end())
    {
        RasterizerGL::ActiveLights active;
        _selectLights(mLights, active, pRasterizer);
        it = pRasterizer->mContext.frame.activeLights.insert( std::make_pair(this, active) ).first;
    }
    RasterizerGL::ActiveLights& active = it->second;


    size_t activeCount = active.positionsEc.size();

    {
        GLint unifIndex = gl->getUniformLocation(mId, "unifAmbient");
        if (unifIndex != -1)
        {
            gl->uniform3f(unifIndex, 0.1f, 0.1f, 0.1f);
        }
    }

    {
        GLint unifIndex = gl->getUniformLocation(mId, "unifLightCount");
        if (unifIndex != -1)
        {
            gl->uniform1i(unifIndex, (int)activeCount);
        }
    }

    if (activeCount > 0)
    {
        //
        // Position, color, attenuation
        //
        {
            GLint idx = gl->getUniformLocation(mId, "unifLightPosition[0]");
            if (idx != -1)
                gl->uniform3fv(idx, activeCount, &active.positionsEc[0].x);
        }
        {
            GLint idx = gl->getUniformLocation(mId, "unifLightColor[0]");
            if (idx != -1)
                gl->uniform3fv(idx, activeCount, &active.colors[0].r);
        }
        {
            GLint idx = gl->getUniformLocation(mId, "unifLightAttenuation[0]");
            if (idx != -1)
                gl->uniform3fv(idx, activeCount, &active.attenuation[0].x);
        }


        //
        // Glow parameters
        //
        {
            GLint idx = gl->getUniformLocation(mId, "unifLightGlowRadius[0]");
            if (idx != -1)
                gl->uniform1fv(idx, activeCount, &active.glowRadius[0]);
        }
        {
            GLint idx = gl->getUniformLocation(mId, "unifLightGlowMultiplier[0]");
            if (idx != -1)
                gl->uniform1fv(idx, activeCount, &active.glowMultiplier[0]);
        }
        {
            GLint idx = gl->getUniformLocation(mId, "unifLightGlowExponent[0]");
            if (idx != -1)
                gl->uniform1fv(idx, activeCount, &active.glowExponent[0]);
        }
    }
}
