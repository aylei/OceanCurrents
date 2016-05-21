/* olic implementation.
 *
 * uthor: alei  mailto:rayingecho@hotmail.com
 */

#include "olic.hpp"
#include <algorithm>

static const glm::vec4 BLACK = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

static const glm::vec4 WHITE = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

OlicContext* OlicContext::_instance = nullptr;

OlicContext & OlicContext::init(OlicParam &olicParam, VectorField &field) {
    if (OlicContext::_instance != nullptr) {
        return *_instance;
    }
    OlicContext* context = new OlicContext(olicParam, field);
    _instance = context;
    return *_instance;
}

OlicContext::OlicContext(OlicParam &olicParam, VectorField &field) {
    _param = &olicParam;
    auto size = olicParam.width * olicParam.height;
    // initial: black
    _sourceTex = std::vector<glm::vec4>(size, BLACK);
    _resultTex = std::vector<glm::vec4>(size, BLACK);
    _isColored = std::vector<bool>(size, false);
    _offset = std::vector<float>(size, 0.0f);
    _field = &field;

    // init the source texture
    for (auto i = 0; i < size; i++) {
        if (rand() > 1.0 - olicParam.dropletRate) {
            int xCoords = i % olicParam.width;
            int yCoords = int(i / olicParam.width);

            // make sure the droplet is within the canvas
            if (xCoords < olicParam.width - olicParam.dimPixel &&
                yCoords < olicParam.height - olicParam.dimPixel) {
                // set this pixel and the around dim pixels to WHITE  
                for (auto k = 0; k < olicParam.dimPixel; ++k) {
                    for (auto j = 0; j < olicParam.dimPixel; ++j) {
                        _sourceTex[(xCoords + k) + (yCoords + j) * olicParam.width] = WHITE;
                    }
                }
            }
        }
    }
}

void OlicContext::calculateOLIC() {
    int halfWidth = _param->width / 2;
    int halfHeight = _param->width / 2;
    /* OLIC only allow one pixel be colored once, so if we scan points from upper to bottom, the streamline will be 
     * will be clusterd in the upper left of the canvas, which is inhomogeneous.
     * Pick random pixel is expensive, so the trade-off method is every time we select 4 points located in different
     * part of the canvas.
     */
    for (auto i = 0; i < halfHeight * halfWidth; i++) {
        std::vector<std::pair<int, int>> points(0);
        points.push_back(std::pair<int, int>(i % halfWidth, i / halfWidth));
        points.push_back(std::pair<int, int>(i % halfWidth + halfWidth, i / halfWidth));
        points.push_back(std::pair<int, int>(i % halfWidth, i / halfWidth + halfHeight));
        points.push_back(std::pair<int, int>(i % halfWidth + halfWidth, i / halfWidth + halfHeight));

        for (std::pair<int, int> point : points) {
            if (!getIsColored(point)) {
                StreamLine streamLine = this->calculateStreamLine(point);
                convolution(streamLine);
            }
        }
    }

}

StreamLine & OlicContext::calculateStreamLine(std::pair<int, int> point) {
    std::vector<std::pair<int, int>> fowardPoints(_param->sideLength);
    std::vector<std::pair<int, int>> backwardPoints(_param->sideLength);
    std::pair<int, int> currentFoward(point.first, point.second);
    std::pair<int, int> currentBackward(point.first, point.second);

    // calculate forward integral and backward integral
    for (auto i = 0; i < _param->sideLength; i++) {
        std::pair<int, int> nextFoward = _field->RKIntergral(currentFoward, _param->integralStep);
        fowardPoints[i] = nextFoward;
        currentFoward = nextFoward;

        std::pair<int, int> nextBackward = _field->RKIntergral(currentBackward, -_param->integralStep);
        backwardPoints[i] = nextBackward;
        currentFoward = nextBackward;
    }
    
    // reverse the backward points to get the correct order
    std::reverse(backwardPoints.begin(), backwardPoints.end());
    backwardPoints.resize(2 * _param->sideLength);
    backwardPoints.insert(backwardPoints.end(), fowardPoints.begin(), fowardPoints.end());
    StreamLine* streamLine = new StreamLine();
    streamLine->points = backwardPoints;
    return *streamLine;
}

void OlicContext::convolution(StreamLine &streamLine) {
    
}
