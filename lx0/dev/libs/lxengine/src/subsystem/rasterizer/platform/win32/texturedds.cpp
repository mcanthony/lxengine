
#include <iostream>

#include <lx0/lxengine.hpp>

#include <ddraw.h>
#include <GL3/gl3w_modified.hpp>

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3

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
            lx_error("Unrecognized pixel format");

        int width = surface.dwWidth;
        int height = surface.dwHeight;
        int mipmaps = surface.dwMipMapCount;
        size_t dataSize = surface.dwLinearSize * ((mipmaps > 1) ? 2 : 1);


        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmaps - 1);

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

            glCompressedTexImage2D(GL_TEXTURE_2D, level, format, mipmapWidth, mipmapHeight, 0, size, &buffer[0]);

            mipmapWidth  = std::max(mipmapWidth / 2, 1);
            mipmapHeight = std::max(mipmapHeight / 2, 1);
        }
    }
    else
        lx_error("Does not appear to be a valid DDS stream!");

    return id;
}
