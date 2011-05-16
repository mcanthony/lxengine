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
#include <lx0/prototype/prototype.hpp>
#include <lx0/subsystem/rasterizer.hpp>

using namespace lx0::subsystem::rasterizer;

/*!
    Represents all the items to render for a particular frame.  The items are organized
    into a set of layers, each with an ordered list of items.  Each layer has its own
    set of settings which may control the optimization, re-ordering, etc. of the list
    for that layer.
 */
class RenderList
{
public:
    typedef std::vector<ItemPtr> ItemList;

    struct Layer
    {
        void*       pSettings;
        ItemList    list;
    };

    typedef std::map<int,Layer> LayerMap;



    void                    push_back   (int layer, ItemPtr spItem);

    LayerMap::iterator      begin       (void)  { return mLayers.begin(); }
    LayerMap::iterator      end         (void)    { return mLayers.end(); }

    ItemPtr   getItem     (unsigned int id);

protected:
    LayerMap    mLayers;
};

class Renderable : public lx0::core::Element::Component
{
public:
    virtual void update(lx0::core::ElementPtr spElement) {}

    virtual void generate(lx0::core::ElementPtr spElement,
                  RasterizerGL& rasterizer,
                  lx0::prototype::Camera& cam1,
                  CameraPtr spCamera, 
                  LightSetPtr spLightSet, 
                  RenderList& list) = 0;
};
