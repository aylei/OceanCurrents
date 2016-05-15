/* controller interface that handle mouse and keyboard event
 *
 * author: alei  mailto:rayingecho@hotmail.com
 */
#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

class Controller {
public:
    Controller();
    // scroll callback
    void OnScroll(GLFWwindow* window, double xoffset, double yoffset);
    // mouse button callback
    void OnMouseButtonEvent(GLFWwindow* window, int button, int action, int mods);

    glm::mat4 getViewMatrix() const;

    glm::mat4 getModelMatrix() const;

    glm::mat4 getProjectionMatrix() const;

private:
    // rousources related to input
    glm::mat4 _modelMatrix;

    glm::mat4 _viewMatrix;

    glm::mat4 _projectionMatrix;

    glm::vec3 _position;

    float _horizontalAngle;

    float _verticalAngle;
};

#endif