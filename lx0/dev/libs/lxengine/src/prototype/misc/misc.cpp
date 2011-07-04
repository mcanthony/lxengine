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

// Standard headers
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>
#include <iomanip>

// Lx0 headers
#include <lx0/lxengine.hpp>
#include <lx0/core/lxvar/lxvar.hpp>
#include <lx0/util/misc/util.hpp>
#include <lx0/prototype/misc.hpp>

#include "extern/lodepng/lodepng.h"

using namespace lx0::core;

namespace lx0 { namespace prototype { namespace misc_ns {

    void 
    load_png (glgeom::image3f& image, const char* filename)
    {
        // Leverage the old code based on the Image4b class.  This should
        // be rewritten to use the GLGeom primitives directly.
        Image4b img;
        load_png(img, filename);

        image = glgeom::image3f(img.mWidth, img.mHeight);
        for (int y = 0; y < image.height(); ++y)
        {
            for (int x = 0; x < image.width(); ++x)
            {
                const auto& p = img.mData[y * image.width() + x]; 
                glgeom::color3f c;
                c.r = p.r / 255.0f;
                c.g = p.g / 255.0f;
                c.b = p.b / 255.0f;
                image.set(x, y, c);
            }
        }

    }

    void
    save_png (const glgeom::image3f& image, const char* filename)
    {
        // Transcode to a 8-bit RGBA image
        std::vector<lx0::uint8> rgba(image.width() * image.height() * 4);
        {
            lx0::uint8* p = &rgba[0];
            for (int y = 0; y < image.height(); ++y)
            {
                for (int x = 0; x < image.width(); ++x)
                {
                    *p++ = static_cast<lx0::uint8>( glm::clamp(image.get(x, y).r * 255.0f, 0.0f, 255.0f) );
                    *p++ = static_cast<lx0::uint8>( glm::clamp(image.get(x, y).g * 255.0f, 0.0f, 255.0f) );
                    *p++ = static_cast<lx0::uint8>( glm::clamp(image.get(x, y).b * 255.0f, 0.0f, 255.0f) );
                    *p++ = 255;
                }
            }
        }

        LodePNG::Encoder encoder;
        std::vector<lx0::uint8> buffer;
        encoder.encode(buffer, &rgba[0], image.width(), image.height());
        LodePNG::saveFile(buffer, filename);
    }

    void 
    load_png (Image4b& image, const char* filename)
    {
        // Let the LodePNG library do the hard work...
        //
        //
        
        std::vector<unsigned char> buffer;
        LodePNG::loadFile(buffer, filename);

        std::vector<unsigned char> pixels;
        LodePNG::Decoder decoder;
        decoder.decode(pixels, buffer.empty() ? 0 : &buffer[0], (unsigned int)buffer.size());

        if (decoder.hasError())
        {
            lx_error("Could not load png file '%s'.\nError code from LodePNG: %u\nError message:\n%s", 
                filename, 
                decoder.getError(),
                LodePNG_error_text(decoder.getError())
                );
        }

        image.mWidth = decoder.getWidth();
        image.mHeight = decoder.getHeight();
        image.mData.reset( new Image4b::Pixel[image.mWidth * image.mHeight] );

        unsigned char* p = &pixels[0];
        for (auto i = 0; i < image.mWidth * image.mHeight; ++i)
        {
            image.mData[i].r = p[0];
            image.mData[i].g = p[1];
            image.mData[i].b = p[2];
            image.mData[i].a = p[3];
            p += 4;
        }
    }


    /*!
        Returns the unnormalized vector between the camera position and target.
     */
    glgeom::vector3f
    view_vector (const Camera2& camera)
    {
        lx_check_error( !is_zero_length(camera.mTarget - camera.mPosition) );
        return camera.mTarget - camera.mPosition;
    }

    //! Computes the view matrix for the given camera
    /*!
        Note that matrix4 is has column-major order.  This the same as 
        DirectX and the transpose of the layout used by OpenGL.  
        Use glLoadTransposeMatrixf() if directly loading this matrix into
        OpenGL.
     */
    void    
    view_matrix (const Camera2& camera, glm::mat4& viewMatrix)
    {
        viewMatrix = glm::lookAt(camera.mPosition.vec, camera.mTarget.vec, camera.mWorldUp.vec);
    }

    //!
    void    
    move_forward (Camera2& camera, float step)
    {
        const glgeom::vector3f view = normalize( view_vector(camera) ) * step;
        camera.mTarget += view;
        camera.mPosition += view;
    }

    //!
    void    
    move_up (Camera2& camera, float step)
    {
        lx_check_error( is_unit_length( camera.mWorldUp ) );

        const glgeom::vector3f view  = normalize( view_vector(camera) );
        const glgeom::vector3f right = normalize( cross(view, camera.mWorldUp) );
        const glgeom::vector3f viewUp = normalize( cross(right, view) );

        camera.mTarget += viewUp * step;
        camera.mPosition += viewUp * step;
    }

    //!
    void    
    move_vertical (Camera2& camera, float step)
    {
        lx_check_error( is_unit_length( camera.mWorldUp ) );

        camera.mTarget += camera.mWorldUp * step;
        camera.mPosition += camera.mWorldUp * step;
    }

    //!
    void    
    move_right (Camera2& camera, float step)
    {
        const glgeom::vector3f view  = normalize( view_vector(camera) );
        const glgeom::vector3f right = normalize( glgeom::cross(view, camera.mWorldUp) );
        camera.mTarget += right * step;
        camera.mPosition += right * step;
    }

    //! Rotate the camera horizontally about the world "up" axis
    void    
    rotate_horizontal (Camera2& camera, float angle)
    {
        const glgeom::vector3f view = camera.mTarget - camera.mPosition;
        const glgeom::vector3f rotated = rotate(view, camera.mWorldUp, angle);
        camera.mTarget = camera.mPosition + rotated;
    }

    //! Rotate the camera vertically (about the rightward facing axis of the camera)
    void    
    rotate_vertical (Camera2& camera, float angle)
    {
        const glgeom::vector3f view  = camera.mTarget - camera.mPosition;
        const glgeom::vector3f right = normalize( cross(view, camera.mWorldUp) );
        const glgeom::vector3f rotated = rotate(view, right, angle);
        camera.mTarget = camera.mPosition + rotated;
    }

}}}

