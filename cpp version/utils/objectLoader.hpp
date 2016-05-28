/*
 * @brief object load interface.
 *
 * @author alei  mailto:rayingecho@hotmail.com
 */
#ifndef OBJECT_LOADER_HPP
#define OBJECT_LOADER_HPP
#include <GL/glew.h>
#include <string>
#include <vector>
#include <glm/detail/type_vec3.hpp>

class ObjectLoader {
public:
    // factory method
    static ObjectLoader& loadObj(std::string objPath);
    std::vector<unsigned short>& getIndices();
    std::vector<glm::vec3>& getVertices();
    std::vector<glm::vec2>& getUvs();
    std::vector<glm::vec3>& getNormals();
    std::vector<glm::vec2>& getReversedUvs();
private:
    std::vector<unsigned short> _indices;
    std::vector<glm::vec3> _vertices;
    std::vector<glm::vec2> _uvs;
    std::vector<glm::vec3> _normals;
    ObjectLoader() {}
};

#endif
