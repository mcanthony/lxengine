//===========================================================================//
/*
                                   GLGeom

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

    Copyright (c) 2011-2012 athile@athile.net (http://www.athile.net)

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

#include <iostream>
#include <boost/format.hpp>
#include <lx0/lxengine.hpp>
#include <glgeom/prototype/image.hpp>
#include <lx0/prototype/misc.hpp>
#include <lx0/util/gl/glinterface.hpp>

#include <ddraw.h>

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3

extern lx0::OpenGlApi3_2* gl;

static void _dumpTextureToDisk(GLuint id)
{
#ifdef _DEBUG
    static int count = 0;

    int w, h;
    gl->getTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    gl->getTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    
    glgeom::image3f image(w, h);

    gl->pixelStorei(GL_PACK_ALIGNMENT, 1);
    gl->pixelStorei(GL_UNPACK_ALIGNMENT, 1);
    gl->getTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, image.ptr());

    lx0::save_png(image, boost::str( boost::format("dds_texture_%04d.png") % (count++) ).c_str());
#endif
}

GLuint _loadDDS(std::istream& stream)
{
    GLuint id = 0;

    char buffer[4];
    stream.read(&buffer[0], 4);

    if (strncmp(buffer, "DDS ", 4) == 0)
    {
        DDSURFACEDESC2 surface;
        stream.read((char*)&surface, sizeof(surface));

        int format = 0;
        int blockSize = 0;
        int channels = 0;
 
        if (surface.ddpfPixelFormat.dwFourCC == FOURCC_DXT1)
        {
            channels = 3;
            format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            blockSize = 8;
        }
        else if (surface.ddpfPixelFormat.dwFourCC == FOURCC_DXT3)
        {
            channels = 4;
            format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            blockSize = 16;
        }
        else if (surface.ddpfPixelFormat.dwFourCC == FOURCC_DXT5)
        {
            channels = 4;
            format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            blockSize = 16;
        }
        else
            throw lx_error_exception("Unrecognized pixel format");

        int width = surface.dwWidth;
        int height = surface.dwHeight;
        int mipmaps = surface.dwMipMapCount;
        size_t dataSize = surface.dwLinearSize * ((mipmaps > 1) ? 2 : 1);

        gl->genTextures(1, &id);
        gl->bindTexture(GL_TEXTURE_2D, id);
        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmaps - 1);

        int mipmapWidth = width;
        int mipmapHeight = height;
        for (int level = 0; level < mipmaps; ++level)
        {
            // The maps composed of square compressed blocks, so ensure the right sized data is read
            size_t blockWidth = (mipmapWidth + 3) / 4;
            size_t blockHeight = (mipmapHeight + 3) / 4;
            size_t size =  blockWidth * blockHeight * blockSize;
            std::vector<char> buffer;
            buffer.resize(size);
            stream.read(&buffer[0], size);

            // Some source indicate the DDS textures need to be flipped?

            gl->compressedTexImage2D(GL_TEXTURE_2D, level, format, mipmapWidth, mipmapHeight, 0, size, &buffer[0]);

            mipmapWidth  = std::max(mipmapWidth / 2, 1);
            mipmapHeight = std::max(mipmapHeight / 2, 1);
        }

        if (false)
            _dumpTextureToDisk(id);

    }
    else
        throw lx_error_exception("Does not appear to be a valid DDS stream!");

    return id;
}
