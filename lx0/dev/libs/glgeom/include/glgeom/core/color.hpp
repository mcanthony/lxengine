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

#ifndef GLGEOM_COLOR_HPP
#define GLGEOM_COLOR_HPP

#include <glm/glm.hpp>

namespace glgeom
{
    namespace core
    {
        namespace color
        {
            namespace detail
            {
                //===========================================================================//
                //!
                /*!
                 */
                template <class P>
                class color3t
                {
                public:
                    typedef P       type;

                    color3t (void) { /* use glm default ctor */ }
                    color3t (type _r, type _g, type _b) : vec(_r, _g, _b) {}

                    union
                    {
                        struct
                        {
                            type r, g, b;
                        };
                        struct
                        {
                            glm::detail::tvec3<P> vec;
                        };
                    };

                protected:
                };

                //===========================================================================//
                //!
                /*!
                 */
                template <class P>
                class color4t
                {
                public:
                    typedef P       type;

                    color4t (void) : vec(0, 0, 0, 1) { }
                    color4t (type _r, type _g, type _b, type _a) : vec(_r, _g, _b, _a) {}

                    inline type&   operator[] (int i)          { return vec[i]; }
                    inline type    operator[] (int i) const    { return vec[i]; }

                    union
                    {
                        struct
                        {
                            type r, g, b, a;
                        };
                        struct
                        {
                            glm::detail::tvec4<P> vec;
                        };
                    };

                protected:
                };
            }

            typedef detail::color3t<glm::detail::uint8>     color3b;
            typedef detail::color3t<glm::detail::uint16>    color3s;
            typedef detail::color3t<glm::detail::uint32>    color3i;
            typedef detail::color3t<glm::detail::uint64>    color3l;
            typedef detail::color3t<float>                  color3f;
            typedef detail::color3t<double>                 color3d;

            typedef detail::color4t<glm::detail::uint8>     color4b;
            typedef detail::color4t<glm::detail::uint16>    color4s;
            typedef detail::color4t<glm::detail::uint32>    color4i;
            typedef detail::color4t<glm::detail::uint64>    color4l;
            typedef detail::color4t<float>                  color4f;
            typedef detail::color4t<double>                 color4d;

            namespace detail
            {
                static_assert(sizeof(color3b) == 1 * 3, "color3b has unexpected structure size");
                static_assert(sizeof(color3s) == 2 * 3, "color3s has unexpected structure size");
                static_assert(sizeof(color3i) == 4 * 3, "color3i has unexpected structure size");
                static_assert(sizeof(color3l) == 8 * 3, "color3l has unexpected structure size");
                static_assert(sizeof(color3f) == sizeof(float) * 3, "color3f has unexpected structure size");
                static_assert(sizeof(color3d) == sizeof(double) * 3, "color3d has unexpected structure size");


                static_assert(sizeof(color4b) == 1 * 4, "color4b has unexpected structure size");
                static_assert(sizeof(color4s) == 2 * 4, "color4s has unexpected structure size");
                static_assert(sizeof(color4i) == 4 * 4, "color4i has unexpected structure size");
                static_assert(sizeof(color4l) == 8 * 4, "color4l has unexpected structure size");
                static_assert(sizeof(color4f) == sizeof(float) * 4, "color4f has unexpected structure size");
                static_assert(sizeof(color4d) == sizeof(double) * 4, "color4d has unexpected structure size");
            }
        }
    }

    using namespace glgeom::core::color;
}

#endif
