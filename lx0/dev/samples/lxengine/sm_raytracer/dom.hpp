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

#pragma once

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/shaderbuilder.hpp>
#include <lx0/prototype/misc.hpp>
#include <glgeom/prototype/std_lights.hpp>
#include <glgeom/prototype/image.hpp>

using namespace lx0;
using namespace glgeom;

//===========================================================================//

class Camera : public Element::Component
{
public:
    virtual     const char* name() const { return "raytracer"; }

    camera3f    camera;
    glm::mat4   viewMatrix;
    glm::mat4   projMatrix;
};

//===========================================================================//

class Environment : public Element::Component
{
public:
    virtual     const char* name() const { return "raytracer"; }

    Environment() 
        : ambient (0.0f, 0.005f, 0.02f) 
        , shadows (true) 
    {}

    color3f     ambient;
    bool        shadows;
};


//===========================================================================//

class Material : public Element::Component
{
public:
    virtual     const char* name() const { return "raytracer"; }

    virtual     bool        allowShadow    (void) const { return true; }
    virtual     color3f     shade          (ShaderBuilder::ShaderContext& ctx, const color3f& ambient, const intersection3f& intersection) const { return color3f(0, 0, 0); }
};

class GenericMaterial 
    : public Material
{
public:
    GenericMaterial (ShaderBuilder::ShadeFunction shader)
    {
        mShader = shader;
    }

    virtual     color3f     shade (ShaderBuilder::ShaderContext& ctx, const color3f& ambient, const intersection3f& intersection) const 
    { 
        return glgeom::color3f( mShader(ctx) );
    }

protected:
    ShaderBuilder::ShadeFunction mShader;
};

//===========================================================================//

class Geometry : public Element::Component
{
public:
    virtual     const char* name() const { return "raytracer"; }

    virtual ~Geometry(){}

    bool intersect (const ray3f& ray, intersection3f& isect) 
    {
        if (_intersect(ray, isect))
        {
            assert( isect.distance >= 0 );
            return true;
        }
        else
            return false;
    }

    bool setMaterial (ElementPtr spElem, std::string matName)
    {
        if (!matName.empty())
        {
            auto spMatElem = spElem->document()->getElementById(matName);
            if (spMatElem)
                mspMaterial = spMatElem->getComponent<Material>("raytracer");
            return true;
        }
        else
            return false;
    }
    std::shared_ptr<Material>   mspMaterial;

protected:
    virtual bool _intersect (const ray3f&, intersection3f& isect) { return false; }
};

typedef std::shared_ptr<Geometry> GeometryPtr;

//===========================================================================//

class Plane : public Geometry
{
public:
    glgeom::plane3f geom;

protected:
    virtual bool _intersect (const ray3f& ray, intersection3f& isect) 
    {
        return  glgeom::intersect(ray, geom, isect);
    }
};

//===========================================================================//

class Cube : public Geometry
{
public:
    glgeom::cube3f geom;

protected:
    virtual bool _intersect (const ray3f& ray, intersection3f& isect) 
    {
        return  glgeom::intersect(ray, geom, isect);
    }
};

//===========================================================================//

class Sphere : public Geometry
{
public:
    glgeom::sphere3f geom;

protected:
    virtual bool _intersect (const ray3f& ray, intersection3f& isect) 
    {
        return  glgeom::intersect(ray, geom, isect);
    }
};

//===========================================================================//

class Cone : public Geometry
{
public:
    glgeom::cone3f geom;

protected:
    virtual bool _intersect (const ray3f& ray, intersection3f& isect) 
    {
        return  glgeom::intersect(ray, geom, isect);
    }
};

//===========================================================================//

class Cylinder : public Geometry
{
public:
    glgeom::cylinder3f geom;

protected:
    virtual bool _intersect (const ray3f& ray, intersection3f& isect) 
    {
        return  glgeom::intersect(ray, geom, isect);
    }
};

//===========================================================================//

class Light 
    : public Element::Component
    , public point_light_f
{
public:
    virtual     const char* name() const { return "raytracer"; }

    ~Light()
    {
        lx_log("Light dtor");
    }
};
typedef std::shared_ptr<Light> LightPtr;