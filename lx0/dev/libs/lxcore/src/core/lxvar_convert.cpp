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

#include <lx0/core.hpp>
#include <lx0/lxvar.hpp>
#include <lx0/lxvar_convert.hpp>
#include <lx0/core/math/point3.hpp>

#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreVector3.h>

namespace lx0 { namespace core {

    namespace detail
    {
        void _convert(lxvar& v, point3& p)
        {
            p.x = v.at(0).asFloat();
            p.y = v.at(1).asFloat();
            p.z = v.at(2).asFloat();
        }
        
        void _convert(lxvar& value, vector3& v)
        {
            lx_check_error(value.size() == 3);
            v.x = value.at(0).asFloat();
            v.y = value.at(1).asFloat();
            v.z = value.at(2).asFloat();
        }

        //===========================================================================//

        void _convert(lxvar& v, Ogre::ColourValue& u)
        {
            lx_check_error(v.size() == 3);
            u = Ogre::ColourValue(v.at(0).asFloat(), v.at(1).asFloat(), v.at(2).asFloat());
        }

        void _convert(lxvar& v, Ogre::Vector3& u)
        {
            lx_check_error(v.size() == 3);
            u = Ogre::Vector3(v.at(0).asFloat(), v.at(1).asFloat(), v.at(2).asFloat());
        }

        void _convert(lxvar& value, Ogre::Quaternion& q)
        {
            lx_check_error(value.size() == 4);
            q.x = value.at(0).asFloat();
            q.y = value.at(1).asFloat();
            q.z = value.at(2).asFloat();
            q.w = value.at(3).asFloat();
        }

    }
}}
