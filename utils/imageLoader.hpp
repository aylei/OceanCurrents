/*
 * @brief util that load BMP or DDS as an OpenGL texture.
 *
 * @author alei  mailto:rayingecho@hotmail.com
 */
#ifndef IMAGE_LOADER_HPP
#define IMAGE_LOADER_HPP

#include <GL/glew.h>
#include <string>

class ImageLoader {
public:
    static GLuint loadBmpAsTexture(std::string imgPath);
    static GLuint loadDdsAsTexture(std::string imgPath);
private:
    // util class, forbid instantiating.
    ImageLoader() {}
};

#endif