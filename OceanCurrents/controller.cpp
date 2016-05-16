/* controller that handle mouse event.
 *
 * uthor: alei  mailto:rayingecho@hotmail.com
 */
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controller.hpp"

Controller* Controller::_instance = nullptr;

Controller::Controller() {
    // initial camera postion
    _position = glm::vec3(0.0f, 0.0f, 3.0f);
    // inital look direction: toward -Z
    _verticalAngle = 0.0f;
    _horizontalAngle = 3.14f;
    //_projectionMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.01f, 100.0f);
    _projectionMatrix = glm::perspective(glm::radians(30.0f), 3.0f / 3.0f, 0.1f, 100.0f);
    _direction = glm::vec3(
        cos(_verticalAngle) * sin(_horizontalAngle),
        sin(_verticalAngle),
        cos(_verticalAngle) * cos(_horizontalAngle)
        );
    _right = glm::vec3(
        sin(_horizontalAngle - 3.14f / 2.0f),
        0,
        cos(_horizontalAngle - 3.14f / 2.0f)
        );
    _up = glm::cross(_right, _direction);
    _viewMatrix = glm::lookAt(_position, _position + _direction, _up);
    _modelMatrix = glm::mat4(1.0);
    _pressFlag = false;
    _lastCusorPos = glm::vec2(1200.0f / 2.0f, 1200.0f / 2.0f);
}

Controller * Controller::init() {
    if (_instance != nullptr) {
        return _instance;
    }
    _instance = new Controller();
    return _instance;
}

/**
 * handle mouse scorll event: modify the distance between camera and model
 */
void Controller::OnScroll(GLFWwindow *window, double xoffset, double yoffset) {
    double mDistance = _instance->_position.z - yoffset / 10;
    _instance->_position = glm::vec3(_instance->_position.x, _instance->_position.y, mDistance);
    _instance->_viewMatrix = glm::lookAt(_instance->_position, _instance->_position + _instance->_direction, _instance->_up);
}

/**
 * record if mouse left button if pressed, for Controller::RefreshMatrices to use
 */
void Controller::OnMouseButtonEvent(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            _instance->_pressFlag = true;
        } else if (action == GLFW_RELEASE) {
            _instance->_pressFlag = false;
        }
    }
}

void Controller::refreshMatrices(GLFWwindow *window) {
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    // only apply change when press down mouse left button
    if(_pressFlag) {
        float xDelta = xPos - _lastCusorPos.x;
        glm::vec3 axis(0.0f, 1.0f, 0.0f);
        if (abs(xDelta) > 1.0) {
            _modelMatrix = glm::rotate(_modelMatrix, xDelta / 100, axis);
        }
    }
    _lastCusorPos = glm::vec2(xPos, yPos);
}

glm::mat4 Controller::getViewMatrix() const {
    return _viewMatrix;
}

glm::mat4 Controller::getModelMatrix() const {
    return _modelMatrix;
}

glm::mat4 Controller::getProjectionMatrix() const {
    return _projectionMatrix;
}

