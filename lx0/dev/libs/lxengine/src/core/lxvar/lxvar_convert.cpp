//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010 athile@athile.net (http://www.athile.net)

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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

#include <lx0/lxengine.hpp>
#include <lx0/core/lxvar/lxvar.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>
#include <glgeom/glgeom.hpp>

#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreVector3.h>

namespace lx0 { namespace core { namespace lxvar_ns {

    namespace detail
    {
        template <typename T>
        void _convert_any3f (lxvar& v, T& u)
        {
            lx_check_error(v.isArray());
            lx_check_error(v.size() == 3, "Cannot convert lxvar: array size is not 3 (%s:%d).", __FILE__, __LINE__);
            u[0] = v.at(0).as<float>();
            u[1] = v.at(1).as<float>();
            u[2] = v.at(2).as<float>();
        }

        void _convert (lxvar& v,    glgeom::point3f& u)     { _convert_any3f(v, u); }
        void _convert (lxvar& v,    glgeom::point3d& u)     { _convert_any3f(v, u); }
        void _convert (lxvar& v,    glgeom::vector3f& u)    { _convert_any3f(v, u);}
        void _convert (lxvar& v,    glgeom::vector3d& u)    { _convert_any3f(v, u); }
        void _convert (lxvar& v,    glgeom::color3f& u)     { _convert_any3f(v, u);}
        void _convert (lxvar& v,    glgeom::color3d& u)     { _convert_any3f(v, u); }

        //===========================================================================//

        void _convert(lxvar& v, Ogre::ColourValue& u)
        {
            lx_check_error(v.size() == 3);
            u = Ogre::ColourValue(v.at(0).as<float>(), v.at(1).as<float>(), v.at(2).as<float>());
        }

        void _convert(lxvar& v, Ogre::Vector3& u)
        {
            lx_check_error(v.size() == 3);
            u = Ogre::Vector3(v.at(0).as<float>(), v.at(1).as<float>(), v.at(2).as<float>());
        }

        void _convert(lxvar& value, Ogre::Quaternion& q)
        {
            lx_check_error(value.size() == 4);
            q.x = value.at(0).as<float>();
            q.y = value.at(1).as<float>();
            q.z = value.at(2).as<float>();
            q.w = value.at(3).as<float>();
        }

    }
}}


    lxvar lxvar_from    (const glgeom::vector3f& v)
    {
        return lxvar(v.x, v.y, v.z);
    }
    
    
    lxvar lxvar_from    (const glgeom::point3f& v)
    {
        return lxvar(v.x, v.y, v.z);
    }

}
