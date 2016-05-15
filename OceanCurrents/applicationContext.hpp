/**
 * application context for OpenGL program based on glew&glfw.
 *
 * this is designed to reduce overwhelming boilerplate code in 'normal' OpenGL program, and avoid
 * annoying global variable and global function. this class is a singlton obviously.
 * 
 * author: alei  mailto:rayingecho@hotmail.com
 */
#ifndef APPLICATION_CONTEXT_HPP
#define APPLICATION_CONTEXT_HPP

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utils/shaderProgram.hpp>

// color structure, default to black
struct MyColor {
    GLclampf red = 0.0;
    GLclampf green = 0.0;
    GLclampf blue = 0.0;
    GLclampf alpha = 0.0;
};

struct ApplicationConfig {
    // ------- GLFW related ---------
    unsigned int windowWidth = 1024;
    unsigned int windowHeight = 768;
    unsigned int windowSample = 4;
    unsigned int glfwMajor = 3;
    unsigned int glfwMinor = 3;
    std::string windowTitle = "default";

    // --------- GL related ----------
    MyColor* color = new MyColor();
    std::string fragmentShader = "default.fragment";
    std::string vertexShader = "default.vertex";
};

/*ApplicationConfig Builder to provide unified interface to config*/
class ConfigBuilder {
public:
    ConfigBuilder() {
        _config = new ApplicationConfig();
    }
    ~ConfigBuilder() {}
    ApplicationConfig* build() const {
        return this->_config;
    }
    ConfigBuilder& windowWidth(unsigned int width) {
        _config->windowWidth = width;
        return *this;
    }
    ConfigBuilder& windowHeight(unsigned int height) {
        _config->windowHeight = height;
        return *this;
    }
    ConfigBuilder& windowSample(unsigned int sample) {
        _config->windowSample = sample;
        return *this;
    }
    ConfigBuilder& glfwMajor(unsigned int major) {
        _config->glfwMajor = major;
        return *this;
    }
    ConfigBuilder& glfwMinor(unsigned int major) {
        _config->glfwMinor = major;
        return *this;
    }
    ConfigBuilder& windowTitle(std::string title) {
        _config->windowTitle = title;
        return *this;
    }
    ConfigBuilder& color(MyColor* color) {
        // fuck memory management
        MyColor* toDelete = _config->color;
        _config->color = color;
        delete toDelete;
        return *this;
    }
    ConfigBuilder& color(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
        _config->color->red = red;
        _config->color->green = green;
        _config->color->blue = blue;
        _config->color->alpha = alpha;
        return *this;
    }
    ConfigBuilder& fragmentShader(std::string shaderSource) {
        _config->fragmentShader = shaderSource;
        return *this;
    }
    ConfigBuilder& vertexShader(std::string shaderSource) {
        _config->vertexShader = shaderSource;
        return *this;
    }
private:
    ApplicationConfig* _config;
};

class ApplicationContext {
public:
    // factory methods for singleton.
    static ApplicationContext& init(ApplicationConfig* config = nullptr);

    template <class T>
    GLuint populateBuffer(std::vector<T> &bufferData) {
        GLuint buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(T), &bufferData[0], GL_STATIC_DRAW);
        _buffers.push_back(buffer);
        return buffer;
    }

    template <class T>
    GLuint populateElementBuffer(std::vector<T> &bufferData) {
        GLuint buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferData.size() * sizeof(T), &bufferData[0], GL_STATIC_DRAW);
        _buffers.push_back(buffer);
        return buffer;
    }

    GLFWwindow* getWindow() {return _window;}

    ShaderProgram* getShaderProgram() const { return _program; }

    GLuint getVertexArrayId() const { return _vertexArrayId; }

    void finalize();

private:
    static ApplicationContext* _instance;

    ApplicationConfig* _appConfig;

    GLFWwindow* _window;

    // shaders program instance
    ShaderProgram* _program;

    GLuint _vertexArrayId;

    std::vector<GLuint> _buffers;

    // singleton, forbid instantiating from client.
    ApplicationContext() {}
};

#endif




