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
#include "NetCDFArray.h"


using namespace glm;

void testNetCDF() {
    NetCDFArray nca("2015031500_ocean.nc");

    /*get attributes*/
    std::vector<std::string> var_lists = nca.getVariableList();
    std::cout << "the attributes are:" << std::endl;
    for (std::vector<std::string>::iterator it = var_lists.begin();
    it != var_lists.end(); ++it) {
        std::cout << *it << "   ";
        if (it + 1 == var_lists.end())
            std::cout << std::endl;
    }

    /*get altitudes*/
    std::vector<levels_t> level_lists = nca.getLevelsList();
    std::cout << "the altitudes are:" << std::endl;
    for (std::vector<levels_t>::iterator it = level_lists.begin();
    it != level_lists.end(); ++it) {
        std::cout << *it << "   ";
        if (it + 1 == level_lists.end())
            std::cout << std::endl;
    }

    {
        /*select a time frame tick. 0 - 23*/
        size_t tick = 1;

        /*select an attribute.*/
        std::string attribute_str = var_lists[6];
        std::cout << "the attribute " << attribute_str << " has been selected" << std::endl;

        /*select an alititude index*/
        size_t level_index = 4;

        /*give a GeoArray instance to save the result*/
        GeoArray<float> U;

        /*read geo grid data from the netcdf file*/
        nca.getGeoArrayData(U, attribute_str, tick, level_index);

        std::cout << "max value = " << U.maxVal_ << std::endl << "min value = " << U.minVal_ << std::endl;
        std::cout << "latitude count = " << U.latitude_num_ << "; from " << U.latitude_start_ << " to " << U.latitude_end_ << std::endl;
        std::cout << "longitude count = " << U.longitude_num_ << "; from " << U.longitude_start_ << " to " << U.longitude_end_ << std::endl;
        std::cout << "data[1][300] = " << U(1, 300) << "; namely, the value of attribute U at (latitude[1] = " << U.latitude_start_ + 1 * U.latitude_interval_
            << ", longitude[300] = " << U.longitude_start_ + 300 * U.longitude_interval_ << ")" << std::endl;
    }
}
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

    testNetCDF();
    

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





