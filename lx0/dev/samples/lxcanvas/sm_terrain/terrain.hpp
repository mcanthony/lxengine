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

#include <lx0/element.hpp>
#include <lx0/prototype/prototype.hpp>

#include "main.hpp"
#include "rasterizergl.hpp"

namespace Terrain
{
    using namespace lx0::core;
    using namespace lx0::prototype;

    class Runtime : public Element::Component
    {
    public:
                    Runtime (ElementPtr spElem);

        tuple3      calcColor(float s, float t);
        float       calcHeight(float s, float t);
    };


    class Render : public Renderable
    {
    public:
        Render();

        virtual void generate(ElementPtr spElement,
                      RasterizerGL& rasterizer,
                      Camera& cam1,
                      RasterizerGL::CameraPtr spCamera, 
                      RasterizerGL::LightSetPtr spLightSet, 
                      std::vector<RasterizerGL::ItemPtr>& list);

    protected:
        RasterizerGL::MaterialPtr       _ensureMaterial (RasterizerGL& rasterizer);
        RasterizerGL::ItemPtr           _buildTile (ElementPtr spElement, 
                                            RasterizerGL& rasterizer, 
                                            RasterizerGL::CameraPtr spCamera, 
                                            RasterizerGL::LightSetPtr spLightSet, 
                                            int regionX, int regionY);
        RasterizerGL::GeometryPtr       _buildTileGeom2 (ElementPtr spElement, RasterizerGL& rasterizer,  int regionX, int regionY);

        std::shared_ptr<Terrain::Runtime>                        mspTerrain;
        RasterizerGL::MaterialWPtr                               mwpMaterial;
        std::map<std::pair<short, short>, RasterizerGL::ItemPtr> mMap;
    };
}
