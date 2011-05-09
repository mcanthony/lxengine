//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)

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

#ifndef RAYTRACER_HPP
#define RAYTRACER_HPP

#include <vector>

#include <glgeom/glgeom.hpp>

namespace lx0 { namespace core { class DocumentComponent; } }

//===========================================================================//


class image3f
{
public:
                            image3f     (void) : mWidth(0), mHeight(0) {}
                            image3f     (int w, int h) : mWidth(w), mHeight(h) { mPixels.resize(mWidth * mHeight); }

    bool                    empty       (void) const { return (mWidth == 0 && mHeight == 0); }

    void                    set         (int x, int y, const glgeom::color3f& c) { mPixels[y * mWidth + x] = c; }
    const glgeom::color3f&  get         (int x, int y)  { return mPixels[y * mWidth + x]; }
    float*                  ptr         (void)          { return &mPixels[0].r; }

    int                     width       (void) const    { return mWidth; }
    int                     height      (void) const    { return mHeight; }

protected:
    int                          mWidth;
    int                          mHeight;
    std::vector<glgeom::color3f> mPixels;
};


lx0::core::DocumentComponent* create_raytracer();

#endif