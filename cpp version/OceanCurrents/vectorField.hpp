/* vector field interface
 *
 * uthor: alei  mailto:rayingecho@hotmail.com
 */

#ifndef VECTOR_FIELD_HPP
#define VECTOR_FIELD_HPP

#include <stdlib.h>
#include <utility>
#include <glm/detail/type_vec2.hpp>
#include "GeoArray.h"

class VectorField {
public:
    /**
     * @brief using RK intergral method to calculate next point upon the vector field
     * 
     * @param originPoint the point that preceed the point to calculate
     * @param step the integral step
     * @return the next point for given point and step in this vector field
     */
    glm::vec2 RKIntergral(glm::vec2 originPoint, float step);
    
    glm::vec2 getVector(std::pair<int, int> point);

    glm::vec2 getVector(glm::vec2 point);

    explicit VectorField(GeoArray<float>& field);
private:
    GeoArray<float> _field;
};

#endif


