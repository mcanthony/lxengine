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

#ifndef GLGEOM_EXT_HPP
#define GLGEOM_EXT_HPP

#include <algorithm>
#include <vector>

namespace glgeom 
{
    namespace ext 
    {
        namespace misc_ns
        {
            //===========================================================================//
            //! Quad represented in origin-axis format
            /*!
                A rectangular quad in 3-space represented by an origin point and
                two axes.
             */
            template <typename T>
            class quad_oa_3t
            {
            public:
                quad_oa_3t() {}
                quad_oa_3t(const point3t<T>& o, const vector3t<T>& x, const vector3t<T>& y)
                    : origin(o)
                    , x_axis(x)
                    , y_axis(y)
                {
                }

                point3t<T>      origin;
                vector3t<T>     x_axis;
                vector3t<T>     y_axis;
            };

            template <typename T>
            point3t<T>
            compute_quad_pt (const quad_oa_3t<T>& quad, T x, T y, int width, int height)
            {
                T s = (x + 0.5f) / width;
                T t = (y + 0.5f) / height;
                return quad.origin + quad.x_axis * s + quad.y_axis * t;
            }

            typedef quad_oa_3t<float>   quad_oa_3f;
            typedef quad_oa_3t<double>  quad_oa_3d;

            //===========================================================================//
            //! View frustum
            /*!
                View frustum defined by an eye point, near quad, and far distance
             */
            template <typename T>
            class frustum3t
            {
            public:
                typedef T               type;
                typedef quad_oa_3t<T>   quad;
                typedef point3t<T>      point3;

                point3      eye;
                quad        near_quad;
                type        far_dist;
            };

            template <typename T>
            frustum3t<T>
            frustum_from_camera (const camera3t<T>& cam)
            {
                const auto forward = camera_forward_vector(cam);
                const auto right   = camera_right_vector(cam);
                const auto up      = camera_up_vector(cam);
    
                const auto width   = cam.near_plane * tan(cam.field_of_view);
                const auto height  = width / cam.aspect_ratio;

                frustum3t<T> frustum;
                frustum.eye       = cam.position;
                frustum.near_quad.x_axis = right * width;
                frustum.near_quad.y_axis = up * height;
                frustum.near_quad.origin = frustum.eye + forward * cam.near_plane - right * (width / 2) - up * (height / 2);
                frustum.far_dist  = cam.far_plane;

                return frustum;
            }

            template <typename T>
            ray3t<T>
            compute_frustum_ray (const frustum3t<T>& frustum, T x, T y, int width, int height)
            {
                auto pt = compute_quad_pt(frustum.near_quad, x, y, width, height);
                return ray3t<T>(frustum.eye, normalize(pt - frustum.eye));
            }

            typedef frustum3t<float>        frustum3f;
            typedef frustum3t<double>       frustum3d;

            //-----------------------------------------------------------------------------//
            
            /*!
                \todo Templatize
                \todo Either add the color as a suffice to the function name or pass the
                    colors as parameters
             */
            inline void
            image_fill_checker (image3f& img)
            {
                const glgeom::color3f c0(.05f, .05f, 0.0f);
                const glgeom::color3f c1(   0,    0, 0.05f);

                for (int iy = 0; iy < img.height(); ++iy)
                {
                    for (int ix = 0; ix < img.width(); ++ix)
                    {
                        const auto& c = (ix % 2) + (iy % 2) == 1 ? c0 : c1; 
                        img.set(ix, iy, c); 
                    }
                }
            }

        }
    }  
    using namespace glgeom::ext::misc_ns;
}

#endif

