//===========================================================================//
/*
                                   GLGeom

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

#ifndef GLGEOM_INTERSECTION_HPP
#define GLGEOM_INTERSECTION_HPP

#include <glm/glm.hpp>

#include <glgeom/core/ray.hpp>
#include <glgeom/core/plane.hpp>
#include <glgeom/core/sphere.hpp>

namespace glgeom
{
    namespace detail
    {
        //===========================================================================//
        //!
        /*!
         */
        template <typename P>
        class intersect3t
        {
        public:
            typedef P           type;
            typedef point3t     point;
            typedef vector3t    vector;

            type        distance;
            point       position;
            vector      normal;
        };
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // S P H E R E   I N T E R S E C T I O N S
    //
    ///////////////////////////////////////////////////////////////////////////////

    //---------------------------------------------------------------------------//
    //! Simple binary intersection test: do the primitives intersect or not?
    /*!
        Tests if the two primitives intersect as fast as can be computed.  If there
        is an intersection, the intersection point data is not computed.
     */
    template <typename P>
    bool intersect (const detail::ray3t<P>& ray, const detail::plane3t<P>& plane);

    //---------------------------------------------------------------------------//
    //!
    /*!
     */
    template <typename P>
    void intersect (const detail::ray3t<P>& ray, const detail::plane3t<P>& plane, detail::intersect3t<P>& intersect);


    ///////////////////////////////////////////////////////////////////////////////
    //
    // P L A N E   I N T E R S E C T I O N S
    //
    ///////////////////////////////////////////////////////////////////////////////

    //---------------------------------------------------------------------------//
    //! Simple binary intersection test: do the primitives intersect or not?
    /*!
        Tests if the two primitives intersect as fast as can be computed.  If there
        is an intersection, the intersection point data is not computed.
     */
    template <typename P>
    bool intersect (const detail::ray3t<P>& ray, const detail::sphere3t<P>& sphere);

    //---------------------------------------------------------------------------//
    //!
    /*!
     */
    template <typename P>
    void intersect (const detail::ray3t<P>& ray, const detail::sphere3t<P>& sphere, detail::intersect3t<P>& intersect);

}

#endif
