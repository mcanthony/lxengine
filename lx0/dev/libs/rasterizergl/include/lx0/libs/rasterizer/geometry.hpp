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

#pragma once

namespace lx0
{
    namespace subsystem
    {
        namespace rasterizer_ns
        {

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class Geometry
            {
            public:
                Geometry()
                    : mType          (0)
                    , mtbFlatShading (boost::indeterminate)
                    , mVboIndices    (0)
                    , mVao           (0)
                    , mVboPosition   (0)
                    , mVboNormal     (0)
                    , mCount         (0)
                    , mVboColors     (0)
                    , mTexFlags      (0) 
                    , mFaceCount     (0)
                {
                    for (int i = 0; i < 8; ++i)
                        mVboUVs[i] = 0;
                }
                virtual ~Geometry();
                
                virtual void    activate    (RasterizerGL*, GlobalPass& pass);

                GLenum          mType;
                
                glgeom::abbox3f mBBox;

                // Options
                boost::tribool  mtbFlatShading;

                // Data
                GLuint  mVao;
                GLsizei mCount;
                
                GLuint  mVboPosition;
                GLuint  mVboNormal;
                GLuint  mVboColors;
                GLuint  mVboUVs[8];

                GLuint  mVboIndices;
                GLuint  mTexFlags;
                GLuint  mFaceCount;
            };           
        }
    }
}
