/* controller that handle mouse event.
 *
 * uthor: alei  mailto:rayingecho@hotmail.com
 */
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controller.hpp"

Controller::Controller() {
    // initial camera postion
    _position = glm::vec3(0, 0, 3);
    // inital look direction: toward -Z
    _verticalAngle = 0.0f;
    _horizontalAngle = 3.14f;
    _projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    glm::vec3 direction(
        cos(_verticalAngle) * sin(_horizontalAngle),
        sin(_verticalAngle),
        cos(_verticalAngle) * cos(_horizontalAngle)
        );
    glm::vec3 right = glm::vec3(
        sin(_horizontalAngle - 3.14f / 2.0f),
        0,
        cos(_horizontalAngle - 3.14f / 2.0f)
        );
    glm::vec3 up = glm::cross(right, direction);
    _viewMatrix = glm::lookAt(_position, _position + direction, up);
    _modelMatrix = glm::mat4(1.0);
}

void Controller::OnScroll(GLFWwindow *window, double xoffset, double yoffset) {
}

void Controller::OnMouseButtonEvent(GLFWwindow *window, int button, int action, int mods) {
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

