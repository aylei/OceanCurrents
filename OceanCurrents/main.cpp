#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/shaderProgram.hpp"
#include "utils/imageLoader.hpp"
#include "utils/objectLoader.hpp"
#include "controller.hpp"
#include "applicationContext.hpp"


using namespace glm;

int main(void) {
    auto glContext = ApplicationContext::init(ConfigBuilder().windowTitle("OceanCurrents")
                                                             .fragmentShader("OceanCurrents.frag")
                                                             .vertexShader("OceanCurrents.vert")
                                                             .windowHeight(1024)
                                                             .windowWidth(1024)
                                                             .color(0.0, 0.0, 0.0, 0.0)
                                                             .build());

    // get uniforms
    auto matrixId = glContext.getShaderProgram()->getUniform("MVP");
    auto viewMatrixId = glContext.getShaderProgram()->getUniform("V");
    auto modelMatrixId = glContext.getShaderProgram()->getUniform("M");
    auto texture = ImageLoader::loadBmpAsTexture("color.bmp");
    auto textureId = glContext.getShaderProgram()->getUniform("myTextureSampler");
    auto lightId = glContext.getShaderProgram()->getUniform("LightPosition_worldspace");

    // load obj
    //std::vector<glm::vec3> vertices;
    //std::vector<glm::vec2> uvs;
    //std::vector<glm::vec3> normals;
    //loadOBJ("ball_small.obj", vertices, uvs, normals);
    //std::vector<unsigned short> indices;
    //std::vector<vec3> indexedVertices;
    //std::vector<vec2> indexedUvs;
    //std::vector<vec3> indexedNormals;
    //loadAssImp("suzanne.obj", indices, indexedVertices, indexedUvs, indexedNormals);
    //indexVBO(vertices, uvs, normals, indices, indexedVertices, indexedUvs, indexedNormals);

    ObjectLoader loader = ObjectLoader::loadObj("sphere.obj");

    // populate buffers
    auto vertexBuffer = glContext.populateBuffer(loader.getVertices());
    auto uvBuffer = glContext.populateBuffer(loader.getUvs());
    auto normalBuffer = glContext.populateBuffer(loader.getNormals());
    auto elementBuffer = glContext.populateElementBuffer(loader.getIndices());

    auto lastTime = glfwGetTime();
    auto nbFrames = 0;

    Controller* controller = Controller::init();

    // set scroll callback
    glfwSetScrollCallback(glContext.getWindow(), Controller::OnScroll);
    glfwSetMouseButtonCallback(glContext.getWindow(), Controller::OnMouseButtonEvent);

    do {
        auto currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime > 1.0) {
            printf("%f ms/frame\n", 1000.0 / double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        controller->refreshMatrices(glContext.getWindow());

        glUseProgram(glContext.getShaderProgram()->getProgramId());

        auto ProjectionMatrix = controller->getProjectionMatrix();
        auto ViewMatrix = controller->getViewMatrix();
        auto ModelMatrix = controller->getModelMatrix();
        auto MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(matrixId, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &ModelMatrix[0][0]);
        glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &ViewMatrix[0][0]);
        glm::vec3 lightPos = glm::vec3(4, 4, 4);
        glUniform3f(lightId, lightPos.x, lightPos.y, lightPos.z);
        
        // 每帧动画都要更新 色表
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(textureId, 0);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(
            0,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glVertexAttribPointer(
            1,                                // attribute
            2,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
            );

        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glVertexAttribPointer(
            2,                                // attribute
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
            );

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

        // Draw the triangles !
        glDrawElements(
            GL_TRIANGLES,      // mode
            loader.getIndices().size(),    // count
            GL_UNSIGNED_SHORT,   // type
            (void*)0           // element array buffer offset
            );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        // Swap buffers
        glfwSwapBuffers(glContext.getWindow());
        glfwPollEvents();
    } while (glfwGetKey(glContext.getWindow(), GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(glContext.getWindow()) == 0);

    glContext.finalize();
    return 0;
}



