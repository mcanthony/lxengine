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

#pragma once

#include <glgeom/glgeom.hpp>

namespace lx0 { namespace prototype {

    struct Image4b
    {
        struct Pixel
        {
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
        };

        int                      mWidth;
        int                      mHeight;
        std::unique_ptr<Pixel[]> mData;
    };

    void    load_png (Image4b& image, const char* filename);


    struct Camera
    {
        glgeom::vector3f forward () const { return mTarget - mPosition; }

        glgeom::point3f    mPosition;
        glgeom::point3f    mTarget;
        
        glgeom::vector3f   mWorldUp;        //! Reference vector for the "up" direction in the world

        float              mFov;
        float              mNear;
        float              mFar;
    };

            glgeom::vector3f    view_vector         (const Camera& camera);
            void                view_matrix         (const Camera& camera, glm::mat4& viewMatrix);
    inline  glm::mat4           view_matrix         (const Camera& camera) { glm::mat4 m; view_matrix(camera, m); return m; } 

            void                move_forward        (Camera& camera, float step);
    inline  void                move_backward       (Camera& camera, float step) { move_forward(camera, -step); }
            void                move_up             (Camera& camera, float step);
    inline  void                move_down           (Camera& camera, float step) { move_up(camera, -step); }
            void                move_vertical       (Camera& camera, float step);
            void                move_right          (Camera& camera, float step);
    inline  void                move_left           (Camera& camera, float step) { move_right(camera, -step); }
            
            void                rotate_horizontal   (Camera& camera, float angle);
            void                rotate_vertical     (Camera& camera, float angle);

}}


namespace lx0 
{
    namespace prototype
    {
        namespace general
        {
            namespace detail
            {

            }
        }
    }
}