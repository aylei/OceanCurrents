/**
 * application context for OpenGL program based on glew&glfw.
 *
 * this is designed to reduce overwhelming boilerplate code in 'normal' OpenGL program, and avoid
 * annoying global variable and global function. this class is a singlton obviously.
 *
 * author: alei  mailto:rayingecho@hotmail.com
 */

#include "applicationContext.hpp"
#include <utils/shaderProgram.hpp>

ApplicationContext* ApplicationContext::_instance = nullptr;

ApplicationContext& ApplicationContext::init(ApplicationConfig* config) {
    // exists instance, return it
    if (ApplicationContext::_instance != nullptr) {
        return *ApplicationContext::_instance;
    }

    // ELSE: application context has not been initialized, initialize it
    auto context = new ApplicationContext();
    if (config == nullptr) {
        // no configuration provided, use default config.
        config = new ApplicationConfig();
    }
    // init GLFW window
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, config->windowSample);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config->glfwMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config->glfwMinor);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    context->_window = glfwCreateWindow(config->windowWidth, config->windowHeight, config->windowTitle.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(context->_window);

    // init GLEW
    glewExperimental = true;
    glewInit();

    // init GLFW input mode
    glfwSetInputMode(context->_window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(context->_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwPollEvents();
    glfwSetCursorPos(context->_window, config->windowWidth / 2, config->windowHeight / 2);

    // clear color and enable z-buffer
    glClearColor(config->color->red, config->color->green, config->color->blue, config->color->alpha);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    // generate vertex array
    GLuint vertexArrayId;
    glGenVertexArrays(1, &vertexArrayId);
    glBindVertexArray(vertexArrayId);

    context->_buffers = std::vector<GLuint>();
    context->_buffers.clear();

    context->_vertexArrayId = vertexArrayId;

    // init shader program
    context->_program = &ShaderProgram::init(config->vertexShader, config->fragmentShader);

    // hold config and context
    context->_appConfig = config;
    ApplicationContext::_instance = context;
    
    return *context;
}

// finalize method to release resources
void ApplicationContext::finalize() {
    // delete all the buffers
    for (GLuint buffer : this->_buffers) {
        glDeleteBuffers(1, &buffer);
    }
    // release program
    this->_program->finalize();
    // delete vertexArray
    glDeleteVertexArrays(1, &this->_vertexArrayId);

    // close GLFW window
    glfwTerminate();

    delete this;
}
