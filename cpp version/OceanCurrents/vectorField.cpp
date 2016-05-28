/* vector field implementation, based on netcdf dataset
 *
 * uthor: alei  mailto:rayingecho@hotmail.com
 */

#include "vectorField.hpp"

glm::vec2 VectorField::RKIntergral(glm::vec2 originPoint, float step) {
    glm::vec2 vector = getVector(originPoint);
    glm::vec2 k1 = (vector *= step);
    vector = getVector(originPoint + glm::vec2(k1.x * 0.5, k1.y * 0.5));
    glm::vec2 k2 = (vector *= step);
    vector = getVector(originPoint + glm::vec2(k2.x * 0.5, k2.y * 0.5));
    glm::vec2 k3 = (vector *= step);
    vector = getVector(originPoint + glm::vec2(k3.x, k3.y));
    glm::vec2 k4 = (vector *= step);

    glm::vec2 finalPoint = originPoint + (k1 *= 1 / 6) + (k2 *= 1 / 3) + (k3 *= 1 / 3) + (k4 *= 1 / 6);
    
    return finalPoint;
}

glm::vec2 VectorField::getVector(std::pair<int, int> point) {

}

glm::vec2 VectorField::getVector(glm::vec2 point) {
    return getVector(std::pair<int, int>(round(point.x), round(point.y)));
}

VectorField::VectorField(GeoArray<float> &field) {

}
