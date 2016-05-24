/* vector field interface
 *
 * uthor: alei  mailto:rayingecho@hotmail.com
 */

#ifndef VECTOR_FIELD_HPP
#define VECTOR_FIELD_HPP

#include <stdlib.h>
#include <utility>
#include <glm/detail/type_vec2.hpp>

class VectorField {
public:
    /**
     * @brief using RK intergral method to calculate next point upon the vector field
     * 
     * @param originPoint the point that preceed the point to calculate
     * @param step the integral step
     * @return the next point for given point and step in this vector field
     */
    std::pair<int, int> RKIntergral(std::pair<int, int> originPoint, float step);
    
    glm::vec2 getVector(std::pair<int, int> point);
};

inline std::pair<int, int> VectorField::RKIntergral(std::pair<int, int> originPoint, float step) {
    return std::pair<int, int>(1, 1);
}

inline glm::vec2 VectorField::getVector(std::pair<int, int> point) {
    return glm::vec2(1.0f, 1.0f);
}
#endif


