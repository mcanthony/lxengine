//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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
/*
    The Perlin Noise algorithm is copyright Ken Perlin.

    This implementation is based on, but not fully equivalent to, Malcolm 
    Kesson's freely available reference implementation provided at:
    http://www.fundza.com/c4serious/noise/perlin/perlin.html

 */

#include <lx0/core/core.hpp>
#include <lx0/core/math/vector3.hpp>
#include <lx0/core/math/noise.hpp>

using namespace lx0::core;

namespace { 

    /*!
        Simple linear interpolation helper function.
     */
    float interpolate (float t, float a, float b)
    { 
        return a + t * (b - a); 
    }


    /*!
        What is the hash?   The hash is a "random" value generated for any
        x, y, z that is guarenteed to be constant for any given x, y, z.
        
        This effectively generates a consistent, random value for every
        integer point in space.  The patern is of coursed tiled based on
        the same of the look-up table, but the tiling is sufficiently
        large with respect to the variations in the Perlin noise function
        as to be unnoticable.
     */
    int hash (int x, int y, int z)
    {
        /*
            A 256 element table of random numbers from 0-255.

            This table is from Malcolm Kesson's reference implementation
            of Perlin noise: http://www.fundza.com/c4serious/noise/perlin/perlin.html.
         */
        static const unsigned char p[] =
        { 
            151,
            160,137,91,90,15,
            131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,
            21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
            35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
            74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,
            230,220,105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,
            80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,
            164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,
            118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,
            183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
            172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
            218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,
            145,235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,204,176,
            115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,
            141,128,195,78,66,215,61,156,180,
        };

        // The pattern tiles every 256 samples in any direction
        x &= 255;
        y &= 255;
        z &= 255;

        int h = p[ p[ p[x] + y] + z];
        return h;
    }

    /*!
        Return the dot product gradient vector associated with for the given hash
        and the incoming vector.
     */
    float 
    dot_gradient (int hash, const vector3& v) 
    {
        //
        // Perlin's 2002 improvement to algorithm reduces the gradient
        // value possibilities to a vectors from the cube center
        // to one of the twelve edge centers on the cube.  The previous
        // incarnation of the algorithn used a random gradient.
        //
        // Each corner of the cube has been assigned a random, but
        // consistent hash value.   Since the hash values should be
        // uniform, the modulo should maintain a uniform distribution.
        //

        // The unoptimized code for the gradient look-up:
        /*
            switch (hash % 12)
            {
            default:    lx_error("Unreachable code");

            case 0:  return vector3( 1,  1, 0);
            case 1:  return vector3(-1,  1, 0);
            case 2:  return vector3( 1, -1, 0);
            case 3:  return vector3(-1, -1, 0);
    
            case 4:  return vector3( 1,  0, 1);
            case 5:  return vector3(-1,  0, 1);
            case 6:  return vector3( 1,  0,-1);
            case 7:  return vector3(-1,  0,-1);
    
            case 8:  return vector3( 0,  1, 1);
            case 9:  return vector3( 0, -1, 1);
            case 10: return vector3( 0,  1, -1);
            case 11: return vector3( 0, -1, -1);
            }
        */

        //
        // Since the gradients are fixed, inline the dot product
        // directly into a simple additions:
        //
        switch (hash % 12)
        {
        default:    lx_error("Unreachable code");

        case 0:  return  v.x + v.y;
        case 1:  return -v.x + v.y;
        case 2:  return  v.x - v.y;
        case 3:  return -v.x - v.y;
    
        case 4:  return  v.x + v.z;
        case 5:  return -v.x + v.z;
        case 6:  return  v.x - v.z;
        case 7:  return -v.x - v.z;
    
        case 8:  return  v.y + v.z;
        case 9:  return -v.y + v.z;
        case 10: return  v.y - v.z;
        case 11: return -v.y - v.z;
        }
    }

    //
    // Use the smoothing function from Perlin's algorithm:
    //    6t^5 - 15t^4 + 10t^3
    //  = t^3 * (6t^2 - 15t + 10)
    //
    float 
    smoothInterpolant (float t)
    {
        return t * t * t * (6 * t * t - 15 * t + 10);
    }

    /*!
        Perlin Noise

        Perlin noise is essentially a random noise function and an interpolation function.
        The noise function provides incoherent data points while the interpolation function
        smoothes them out into a pseudo-random visually appealing effect.  This is the
        desired result of Perlin noise: a random but smooth, controllable appearance.

        The random noise function needs to be repeatably random: f(x) = y, always for the
        same x.  Otherwise, the Perlin noise function will not produce smooth results 
        (the look-ups to adjacent cells will not return consistent values).  Furthermore,
        the Perlin noise algorithm samples at discrete points on an integer grid, therefore
        the random function does not need to be continuous over real numbers.

        Perlin noise then interpolates between the discrete sampling points' random
        values to create a piece-wise smooth function continuous function in the real 
        number space.  This continuous function is the resulting perlin noise.


        Noise Function

        In the implementation below the noise function is a simple look-up table generated
        from a random number generator.   The function is made to work over three-space by
        generating a hash from recursive look-ups based on each coordinate.

        Interpolation Function

        The interpolation function is split into a "smooth" function and a simple linear
        interpolation.  The fade function manipulates the input 't' value to smooth ease-in
        and ease-out the value transition from each random sample point.  This ensures the
        piece-wise smooth appearance of the resulting noise.
     */
    float perlin_noise_imp(float x, float y, float z) 
    {
        //
        // Perlin noise does a look-up into an "infinite" cubemap.  The each corner of 
        // each cell in the cubemap has a gradient value (i.e. rate of change).  It samples
        // the eight corner values and weights them according to the position with in th
        // cube.  
        //
        // First, find the cell within the cube map and offset within the cube.
        //
        const int cellX = (int)floor(x);
        const int cellY = (int)floor(y);
        const int cellZ = (int)floor(z);

        vector3 uvw = vector3(x, y, z);
        uvw.x -= cellX;
        uvw.y -= cellY;
        uvw.z -= cellZ;

        //
        // For each corner of the cube, generate a "random" but consistent
        // value.  This is the "hash" of each corner and is used to select 
        // a random gradient value.  (Generating a random, but consistent
        // gradient directly would work as well.)
        //
        // The infinite cubemap is actually a finite, repeated
        // (i.e. tiled) cubemap.  This works since the properties of the
        // noise function are such that once any repeated pattern would
        // be discernible, generally the view distance is sufficiently
        // far that the detail is lost anyway.
        //
        // Therefore, for each corner of the cell, generate the hash,
        // get the gradient associated with that hash, and take a dot
        // product of the gradient and the offset within the cell to
        // produce a weighted value for each corner point.
        //
        float g[8];
        for (int i = 0; i < 8; i++) 
        {
            // Get the 0 or 1 offset for each corner to cycle through
            // all eight adjacent corners
            const int cx = (i & 4) ? 1 :0;
            const int cy = (i & 2) ? 1: 0;
            const int cz = (i & 1);

            const int h = hash(cellX + cx, cellY + cy, cellZ + cz);
            vector3 value (uvw.x - cx, uvw.y - cy, uvw.z - cz);           

            g[i] = dot_gradient(h, value);
        }

        //
        // Smooth the interpolant values.  This essentially smooths the linear
        // interpolation to a curve with ease-in / ease-out properties.
        // Without this, the result is discontinuities (i.e. hard 'edges') between
        // cells which adds a noticable tiling to the pattern.
        // 
        vector3 t;
        t.x = smoothInterpolant(uvw.x);
        t.y = smoothInterpolant(uvw.y);
        t.z = smoothInterpolant(uvw.z);

        //
        // Trilinear interpolation of the eight sample points
        //
        // Interpolate across all four edges of X.  Then interpolate the resulting
        // four points across the remaining two edges in Y.  Then interpolate the
        // remaining two points across the sole edge in Z.
        //
        const float blend_x00 = interpolate(t.x, g[0], g[4]);
        const float blend_x10 = interpolate(t.x, g[2], g[6]);
        const float blend_x01 = interpolate(t.x, g[1], g[5]);
        const float blend_x11 = interpolate(t.x, g[3], g[7]);

        const float blend_y0  = interpolate(t.y, blend_x00, blend_x10);
        const float blend_y1  = interpolate(t.y, blend_x01, blend_x11);

        const float blend_z   = interpolate(t.z, blend_y0, blend_y1);

        return blend_z;
    }
  
}

namespace lx0 { namespace core {

    /*!
        Note: consider using GLM's noise functions instead, as they have a faster
        implementation.
     */
    float 
    noise3d_perlin (float x, float y, float z)
    {
        // Returns a number in the -1 to 1 range.  Normalize to
        // 0 to 1.
        return (perlin_noise_imp(x, y, z) + 1.0f) / 2.0f;
    }
}}
