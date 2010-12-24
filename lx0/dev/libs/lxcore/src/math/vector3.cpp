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

#include <lx0/core/math/vector3.hpp>

#include <OGRE/OgreQuaternion.h>

namespace lx0 { namespace core {

    
    /*!
        Returns true if the vectors point in the same direction or opposite directions (180 degrees apart).
     */
    bool is_codirectional (const vector3& u, const vector3& v) 
    {
        vector3 un = normalize(u);
        vector3 vn = normalize(v);
        float cosA = dot(u, v);
        return fabs(cosA - 1.0f) <= 10.0f * std::numeric_limits<float>::epsilon(); 
    }

    vector3  
    rotate (const vector3& v, const vector3& axis, float r)
    {
        Ogre::Quaternion q(Ogre::Radian(r), *reinterpret_cast<const Ogre::Vector3*>(&axis));
        Ogre::Vector3 u = q * (*reinterpret_cast<const Ogre::Vector3*>(&v));
        return *reinterpret_cast<vector3*>(&u);
    }

}}
