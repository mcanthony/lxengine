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

#include "parsers.hpp"
#include <lx0/util/misc/lxvar_convert.hpp>

class ConeParser : public GeometryParser
{
public:
    virtual Geometry* create(ElementPtr spElem) 
    {
        auto pGeom = new Cone;
        pGeom->geom.base = spElem->value().find("base").convert();
        pGeom->geom.radius = spElem->value().find("radius").convert();
        pGeom->geom.axis = spElem->value().find("axis").convert();
        return pGeom;
    }
};

class CubeParser : public GeometryParser
{
public:
    virtual Geometry* create(ElementPtr spElem) 
    {
        auto pGeom = new Cube;
        pGeom->geom.center = spElem->value().find("center").convert(point3f(0, 0, 0));
        pGeom->geom.scale  = spElem->value().find("scale").convert(vector3f(1, 1, 1));
        return pGeom;
    }
};

class CylinderParser : public GeometryParser
{
public:
    virtual Geometry* create(ElementPtr spElem) 
    {
        auto pGeom = new Cylinder;
        pGeom->geom.base = spElem->value().find("base").convert();
        pGeom->geom.radius = spElem->value().find("radius").convert();
        pGeom->geom.axis = spElem->value().find("axis").convert();
        return pGeom;
    }
};

class PlaneParser : public GeometryParser
{
public:
    virtual Geometry* create(ElementPtr spElem) 
    {
        auto pGeom = new Plane;
        pGeom->geom.normal = spElem->value().find("normal").convert();
        pGeom->geom.d      = spElem->value().find("d").convert();
        return pGeom;
    }
};

class SphereParser : public GeometryParser
{
public:
    virtual Geometry* create(ElementPtr spElem) 
    {
        auto pGeom = new Sphere;
        pGeom->geom.center = spElem->value().find("center").convert();
        pGeom->geom.radius = spElem->value().find("radius").convert(.5f);
        return pGeom;
    }
};


void registerGeometryParsers (std::function<void (std::string, GeometryParser*)> registerFunc)
{
    registerFunc("Cone",        new ConeParser);
    registerFunc("Cube",        new CubeParser);
    registerFunc("Cylinder",    new CylinderParser);
    registerFunc("Plane",       new PlaneParser);
    registerFunc("Sphere",      new SphereParser);
}
