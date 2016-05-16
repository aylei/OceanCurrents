/* controller interface that handle mouse and keyboard event
 *
 * author: alei  mailto:rayingecho@hotmail.com
 */
#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

class Controller {
public:
    static Controller* init();

    /**
     * mouse scroll callback
     */
    static void OnScroll(GLFWwindow* window, double xoffset, double yoffset);

    /**
     * mouse button callback
     */
    static void OnMouseButtonEvent(GLFWwindow* window, int button, int action, int mods);

    /**
     * called every frame to refresh matrix according to inputs
     *
     * I use this method instead of another callback method because GLFW do not have 'mouse press down and drag'
     * such events, so its more convenient to handle this condition every frame.
     */
    void refreshMatrices(GLFWwindow* window);

    glm::mat4 getViewMatrix() const;

    glm::mat4 getModelMatrix() const;

    glm::mat4 getProjectionMatrix() const;

private:
    /**
     * this constructor init all contorll related matrix and variables, include:
     * camera position, M.V.P matrices, watch direction (stored as horizontalAngle & verticeAngle)
     */
    Controller();


    /**
     * we have to store an instance here for our static callback (GLFW's C-style callback interface sucks)
     */
    static Controller* _instance;

    // rousources related to input
    glm::mat4 _modelMatrix;

    glm::mat4 _viewMatrix;

    glm::mat4 _projectionMatrix;

    glm::vec3 _position;

    glm::vec3 _direction;

    glm::vec3 _right;
    
    glm::vec3 _up;

    glm::vec2 _lastCusorPos;

    bool _pressFlag;

    float _horizontalAngle;

    float _verticalAngle;
};

#endif