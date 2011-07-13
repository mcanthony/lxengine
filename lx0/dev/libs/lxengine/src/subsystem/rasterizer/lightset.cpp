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
#include <lx0/subsystem/rasterizer.hpp>
#include <lx0/subsystem/rasterizer/lightset.hpp>

using namespace lx0;
using namespace glgeom;

void LightSet::activate(RasterizerGL* pRasterizer)
{
    GLint mId;
    glGetIntegerv(GL_CURRENT_PROGRAM, &mId);

    


    //
    // This is a temporary bit of code to select only the four closest
    // lights.  The number 4 is a hard-coded value based on the current
    // phong shader implementation.  Obviously, this should be generalized.
    //
    size_t lightCount = (int)mLights.size();
    std::vector<glgeom::point3f> positionsEc;
    std::vector<glgeom::color3f> colors;
    positionsEc.reserve(4);
    colors.reserve(4);
   
    if (lightCount > 0)
    {
        struct Entry
        {
            size_t          index;
            float           distance_squared;
            glgeom::point3f positionEc;
        };
        std::list<Entry> closest;

        for (size_t i = 0; i < lightCount; ++i)
        {
            Entry e;
            e.index = i;
            e.positionEc = pRasterizer->mContext.spCamera->viewMatrix * mLights[i]->position;
            e.distance_squared = distance_to_origin_squared(e.positionEc);

            size_t j = 0;
            auto jt = closest.rbegin();
            while (jt != closest.rend() && j < 4)
            {
                if (e.distance_squared < jt->distance_squared)
                    break;
                 ++jt, ++j;
            }
            if (j  < 4)
            {
                closest.insert(jt.base(), e);
                if (closest.size() > 4)
                    closest.pop_front();
            }
        }

        for (auto it = closest.begin(); it != closest.end(); ++it)
        {
            positionsEc.push_back( it->positionEc );
            colors.push_back( mLights[it->index]->color );
        }
    }


    {
        GLint unifIndex = glGetUniformLocation(mId, "unifAmbient");
        if (unifIndex != -1)
        {
            glUniform3f(unifIndex, 0.1f, 0.1f, 0.1f);
        }
    }

    {
        GLint unifIndex = glGetUniformLocation(mId, "unifLightCount");
        if (unifIndex != -1)
        {
            glUniform1i(unifIndex, (int)positionsEc.size());
        }
    }

    if (lightCount > 0)
    {
        {
            GLint idx = glGetUniformLocation(mId, "unifLightPosition[0]");
            if (idx != -1)
                glUniform3fv(idx, lightCount, &positionsEc[0].x);
        }
        {
            GLint idx = glGetUniformLocation(mId, "unifLightColor[0]");
            if (idx != -1)
                glUniform3fv(idx, lightCount, &colors[0].r);
        }
    }
}
