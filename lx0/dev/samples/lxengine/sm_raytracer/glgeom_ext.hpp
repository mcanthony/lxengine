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
#include <glgeom/prototype/transform_srt.hpp>

namespace glgeom {
    namespace ext {

        template <typename P>
        int   solve_quadratic (const P& a, const P& b, const P& c, P& t0, P& t1)
        {
            if (a == 0)
            {
                // No solution
                return 0;
            }
            else
            {
                const auto discriminant = b * b - 4 * a * c;
                if (discriminant < 0)
                {
                    // No real solution; only two imaginary solutions
                    return 0;
                }

                const auto a2 = (2 * a);
                const auto e = -b / a2;
        
                if (discriminant == 0)
                {
                    // One solution
                    t0 = e;
                    return 1;
                }
                else
                {
                    // Two solutions
                    const auto f = sqrt(discriminant) / a2;
                    t0 = e + f;
                    t1 = e - f;
                    return 2;
                }
            }
        }

        template <typename P>
        bool _intersect_unit_cone_inner (const glgeom::ray3t<P>& ray, glgeom::intersection3t<P>& intersection)
        {
            const auto& d = ray.direction;
            const auto& o = ray.origin;

            std::vector<intersection3t<P>> hits;

            //
            // * Compute the intersection with the side
            //
            // The unit cone equation is:
            //
            //      x^2 + y^2 = z^2, where 0 <= z <= 1
            //
            // Substituting in a ray defined a o + dt:
            //
            //  (o.x + d.x * t)^2 + (o.y + d.y * t)^2 - (o.z + d.z * t) ^ 2 = 0
            //  (d.x^2 + d.y^2 - d.z^2) * t^2 + 2 * (o.x * d.x + o.y * d.y - o.z * d.z) * t + (o.x^2 + o.y^2 - o.z^2) 
            //
            // Via the quadratic equation, t can be solved for.
            //  
            //  t = (-b +/- sqrt(b^2 - 4ac)) / 2a
            //  
            // If there was one or two real roots, then substitute back into the ray to find
            // the resulting z value:
            //
            // z = v.z + u.z * t;
            //
            // If 0 <= z <= 1, then the intersection is valid.
            //
            // The normal is always normalize(p.x, p.y, -length(p.xy)) where p is the intersection point.
            //
            {
                const P a = (d.x * d.x + d.y * d.y - d.z * d.z);
                const P b = 2 * (o.x * d.x + o.y * d.y - o.z * d.z);
                const P c = (o.x * o.x + o.y * o.y - o.z * o.z);
                P t[2];
                const int solutions = solve_quadratic<P>(a, b, c, t[0], t[1]);
                for (int i = 0; i < solutions; ++i)
                {
                    if (t[i] > 0)
                    {
                        auto p = o + d * t[i];
                        if (p.z > 0 && p.z <= 1)
                        {
                            intersection3t<P> intersection;
                            intersection.position = p;
                            intersection.normal = normalize(vector3f(p.x, p.y, -glm::length(glm::vec3(p.x, p.y, 0))));
                            intersection.distance = t[i];

                            hits.push_back(intersection);
                        }
                    }
                }
            }
    
            // * Compute the intersection with the cap
            //
            // Simply intersect the ray with the x/y plane at z = 1, then check that
            // the distance to (0, 0, 1) is <= 1.  If so, the cap has been intersected and the normal
            // at that point is always 
            //
            if (d.z != 0)
            {
                const auto t = (1 - o.z) / d.z;
                if (t > 0)
                {
                    const auto p = o + d * t;

                    if (p.x * p.x + p.y * p.y <= P(1))
                    {
                        intersection3t<P> intersection;
                        intersection.position = p;
                        intersection.normal = vector3f(0, 0, 1);
                        intersection.distance = t;
                        hits.push_back(intersection);
                    }
                }
            }

            // * Generalized cone intersection 
            //
            // Transform the ray such that the cone is a unit code, do the intersection, and then
            // apply the inverse transformation.
            //

            if (!hits.empty())
            {
                std::sort(hits.begin(), hits.end(), [](const intersection3t<P>& a, const intersection3t<P>&b) {
                    return a.distance < b.distance;
                });
                intersection = hits.front();
            }
            return !hits.empty();
        }

        /*!
            Dev Notes:

            - This is obviously inefficient as it adds another transformation_srt layer to the code.
              The unit_cone is defined as radius .5, centered at the origin, height +1 Z.  The inner
              worker method works on a radius 1, *tip* at the origin, height -1 Z.  That fixed
              transformation should be possible to do without requiring the full srt transform
              and inverse transforms.
         */
        template <typename P>
        bool intersect (const glgeom::ray3t<P>& ray, const glgeom::unit_cone_t<P>& cone, glgeom::intersection3t<P>& intersection)
        {
            //
            // Note that the translation is applied after the rotation: therefore, the unit 
            // cone will have been rotated to -1 to 0 on the z axis, meaning it must be pushed
            // up on the world z axis.
            //
            // Note glm::tquat(0, 1, 0, 0) is the quaternion to rotate 180 degrees about the
            // x axis.
            //
            glgeom::transform_srt_3t<P> srt;
            srt.scale = vector3t<P>(.5, .5, 1);
            srt.rotate = glm::detail::tquat<P>(0, 1, 0, 0);
            srt.translate = vector3t<P>(0, 0, .5);

            ray3f rt;
            rt.origin = inverse_transform(ray.origin, srt);
            rt.direction = inverse_transform(ray.direction, srt);

            const bool hit = _intersect_unit_cone_inner(rt, intersection);
            if (hit)
            {
                intersection.normal = transform(intersection.normal, srt);
                intersection.position = transform(intersection.position, srt);
                intersection.distance = length(intersection.position - ray.origin);
            }
            return hit;
        }

        template <typename P>
        bool intersect (const glgeom::ray3t<P>& ray, const glgeom::cone3t<P>& cone, glgeom::intersection3t<P>& intersection)
        {
            //
            // Compute the height of the cone and generate a normalized version of the cone
            // axis.
            //
            const P           height = length(cone.axis);
            const vector3t<P> naxis = cone.axis / height; 

            //
            // Now transform the space such that the cone is a unit cone (radius .5, center 0, 0, 0,
            // axis Z+).
            //
            glgeom::transform_srt_3f srt;
            srt.scale     = vector3f(cone.radius / P(.5), cone.radius / P(.5), height);            
            srt.translate = vector3f(cone.base + P(.5) * cone.axis);

            // Compute the dot product of the cone axis and the Z axis in order to determine the angle the
            // axis rotated by.  Use the cross product to generate a vector perpendicular to
            // before/after axes that can be used to rotate about.   Check however if the rotation is 0
            // or 180 degrees from the Z axis, as there will be no valid perpendicular.
            //
            // const vector3t<P> Z(0, 0, 1);
            // const P dp = dot(Z, normalize(cone.axis));
            // dp == naxis.z
            //
            if (abs(naxis.z - P(1)) < P(1e-4))
            {
                // The axis is the Z axis.  
                // Leave srt.rotate as identity.
            }
            else if (abs(naxis.z + P(1)) < P(1e-4))
            {
                // 180 rotation about the X axis
                srt.rotate = glm::detail::tquat<P>(0, 1, 0, 0);
            }
            else
            {
                // General rotation
                const P angle = acos(naxis.z);
                const vector3t<P> axis = cross(naxis, vector3t<P>(0,0,1));
                srt.rotate = orientation(axis, radians(angle));
            }
                
            ray3f rt;
            rt.origin = inverse_transform(ray.origin, srt);
            rt.direction = inverse_transform(ray.direction, srt);

            const bool hit = intersect(rt, unit_cone_t<P>(), intersection);
            if (hit)
            {
                intersection.normal = transform(intersection.normal, srt);
                intersection.position = transform(intersection.position, srt);
                intersection.distance = length(intersection.position - ray.origin);

                //!\todo Rationalize why normalization should happen here rather than throughout
                intersection.normal = normalize(intersection.normal);
            }
            return hit;
        }
    }
    using namespace ext;
}

#endif

