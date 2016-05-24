/* olic implementation.
 *
 * uthor: alei  mailto:rayingecho@hotmail.com
 */

#include "olic.hpp"
#include <algorithm>

OlicContext* OlicContext::_instance = nullptr;

OlicContext & OlicContext::init(OlicParam &olicParam, VectorField &field) {
    if (OlicContext::_instance != nullptr) {
        return *_instance;
    }
    OlicContext* context = new OlicContext(olicParam, field);
    _instance = context;
    return *_instance;
}

/**
 * @brief constructor for factory method to preduce the singlton. 
 * @param olicParam {@link OlicParam} instance holding vita algorithm parameters
 * @param field {@link VectorField} instance providing friendly interface to interact with underlying vector field 
 * 
 * this constructor will init the context and its containers, additionally, will generate the source texutre
 */
OlicContext::OlicContext(OlicParam &olicParam, VectorField &field) {
    _param = &olicParam;
    auto size = olicParam.width * olicParam.height;
    // initial: black
    _sourceTex = std::vector<float>(size, 0.0f);
    _resultTex = std::vector<float>(size, 0.0f);
    _hitCounts = std::vector<int>(size, 0);
    _relateDroplets = std::vector<int>(size, -1);
    _field = &field;
    buildSourceTexture(olicParam);
}

/**
 * @brief build the source sparse droplets texture.
 * 
 * according to anothre paper (Fast Oriented Line Integral Convolution for Vector Field Visualization via
 * the Internet), the droplets placement is very important: the purpose is cover as much area of the final
 * texture as possible while decreasing the overlapping of streamlines. In the other hand, the algorithm
 * have to be efficient enough. So its a challenge to dig out a perfect algorithm to generate the source
 * texture.
 *
 * but util now, a persudo-random generator method is still adopted. 
 * TODO: improve the source texture generating strategy.
 */
void OlicContext::buildSourceTexture(OlicParam& olicParam) {
    auto size = olicParam.width * olicParam.height;
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
                        _sourceTex[(xCoords + k) + (yCoords + j) * olicParam.width] = 1.0f;
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

        // for the point that has not hitted yet, calculate steamline and convolve to get final result
        for (std::pair<int, int> point : points) {
            if (getHitCount(point) < _param->maxHitNum) {
                StreamLine streamLine = this->calculateStreamLine(point);
                convolve(streamLine);
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
        currentBackward = nextBackward;
    }
    
    // reverse the backward points to get the correct order
    std::reverse(backwardPoints.begin(), backwardPoints.end());
    backwardPoints.resize(2 * _param->sideLength);
    backwardPoints.insert(backwardPoints.end(), fowardPoints.begin(), fowardPoints.end());
    StreamLine* streamLine = new StreamLine();
    streamLine->points = backwardPoints;
    return *streamLine;
}

void OlicContext::convolve(StreamLine &streamLine) {
    

    
}
