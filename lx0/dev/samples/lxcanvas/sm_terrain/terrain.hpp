//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)

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

#include <lx0/engine/element.hpp>
#include <lx0/prototype/misc.hpp>

#include "main.hpp"
#include <lx0/subsystem/rasterizer.hpp>

namespace Terrain
{
    using namespace lx0;
    using namespace lx0::prototype;

    class Runtime : public Element::Component
    {
    public:
                    Runtime (ElementPtr spElem);

        glgeom::color3f      calcColor(float s, float t);
        float       calcHeight(float s, float t);
    };


    class Render : public Renderable
    {
    public:
        Render();

        virtual void generate(ElementPtr spElement,
                      RasterizerGL& rasterizer,
                      lx0::prototype::Camera& cam1,
                      CameraPtr spCamera, 
                      LightSetPtr spLightSet, 
                      RenderList& list);

    protected:
        MaterialPtr       _ensureMaterial (RasterizerGL& rasterizer);
        ItemPtr           _buildTile (ElementPtr spElement, 
                                            RasterizerGL& rasterizer, 
                                            CameraPtr spCamera, 
                                            LightSetPtr spLightSet, 
                                            int regionX, int regionY);
        GeometryPtr       _buildTileGeom2 (ElementPtr spElement, RasterizerGL& rasterizer,  int regionX, int regionY);

        std::shared_ptr<Terrain::Runtime>                        mspTerrain;
        MaterialWPtr                               mwpMaterial;
        std::map<std::pair<short, short>, ItemPtr> mMap;
    };
}
