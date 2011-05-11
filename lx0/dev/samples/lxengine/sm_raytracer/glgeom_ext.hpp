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
        bool intersect (const glgeom::ray3t<P>& ray, const glgeom::unit_cone_t<P>& cone, glgeom::intersection3t<P>& intersection)
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

                    if (p.x * p.x + p.y * p.y <= 1)
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
    }
    using namespace ext;
}

#endif

