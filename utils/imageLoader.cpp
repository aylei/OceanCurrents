/*
 * @brief util that load BMP or DDS as an OpenGL texture.
 *
 * @author alei  mailto:rayingecho@hotmail.com
 */
#include "imageLoader.hpp"

GLuint ImageLoader::loadBmpAsTexture(std::string imgPath) {
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    // Actual RGB data
    unsigned char * data;

    // Open the file
    FILE * file = fopen(imgPath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("loadBMP - no texture file found for" + imgPath);
    }
    // Read the header, i.e. the 54 first bytes
    if (fread(header, 1, 54, file) != 54) {
        throw std::runtime_error("loadBMP - illegal BMP file!");
    }
    // A BMP files always begins with "BM"
    if (header[0] != 'B' || header[1] != 'M') {
        throw std::runtime_error("loadBMP - illeagl BMP file!");
    }
    // Make sure this is a 24bpp file
    if (*reinterpret_cast<int*>(&(header[0x1E])) != 0 ||
        *reinterpret_cast<int*>(&(header[0x1C])) != 24) {
        throw std::runtime_error("loadBMP - illeagl BMP file!");
    }

    // Read the information about the image
    dataPos = *reinterpret_cast<int*>(&(header[0x0A]));
    imageSize = *reinterpret_cast<int*>(&(header[0x22]));
    width = *reinterpret_cast<int*>(&(header[0x12]));
    height = *reinterpret_cast<int*>(&(header[0x16]));

    // Some BMP files are misformatted, guess missing information
    if (imageSize == 0) {
        imageSize = width*height * 3; // 3 : one byte for each Red, Green and Blue component
    }
    if (dataPos == 0) {
        dataPos = 54; // The BMP header is done that way
    }

    data = new unsigned char[imageSize];

    // Read the actual data from the file into the buffer
    fread(data, 1, imageSize, file);

    // Everything is in memory now, the file wan be closed
    fclose(file);

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    // OpenGL has now copied the data. Free our own version
    delete[] data;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    return textureID;
}

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

GLuint ImageLoader::loadDdsAsTexture(std::string imgPath) {
    unsigned char header[124];

    FILE *fp;

    /* try to open the file */
    fp = fopen(imgPath.c_str(), "rb");
    if (fp == nullptr) {
        throw std::runtime_error("loadDDS - no such file" + imgPath);
    }

    /* verify the type of file */
    char filecode[4];
    fread(filecode, 1, 4, fp);
    if (strncmp(filecode, "DDS ", 4) != 0) {
        fclose(fp);
        return 0;
    }

    /* get the surface desc */
    fread(&header, 124, 1, fp);

    unsigned int height = *(unsigned int*)&(header[8]);
    unsigned int width = *(unsigned int*)&(header[12]);
    unsigned int linearSize = *(unsigned int*)&(header[16]);
    unsigned int mipMapCount = *(unsigned int*)&(header[24]);
    unsigned int fourCC = *(unsigned int*)&(header[80]);


    unsigned char * buffer;
    unsigned int bufsize;
    /* how big is it going to be including all mipmaps? */
    bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
    buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
    fread(buffer, 1, bufsize, fp);
    /* close the file pointer */
    fclose(fp);

    unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
    unsigned int format;
    switch (fourCC) {
    case FOURCC_DXT1:
        format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case FOURCC_DXT3:
        format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case FOURCC_DXT5:
        format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    default:
        free(buffer);
        return 0;
    }

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
    unsigned int offset = 0;

    /* load the mipmaps */
    for (unsigned int level = 0; level < mipMapCount && (width || height); ++level) {
        unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
            0, size, buffer + offset);

        offset += size;
        width /= 2;
        height /= 2;

        if (width < 1) width = 1;
        if (height < 1) height = 1;
    }

    free(buffer);

    return textureID;
}
