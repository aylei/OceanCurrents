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
    _droplets = std::vector<Droplet>();
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
                // store this droplet and give it a random local offset
                _droplets.push_back(Droplet(xCoords, yCoords, int(rand() * 2 * olicParam.sideLength)));
                int dropletIndex = _droplets.size() - 1;
                // set this pixel and the around dim pixels to max intensity
                for (auto k = 0; k < olicParam.dimPixel; ++k) {
                    for (auto j = 0; j < olicParam.dimPixel; ++j) {
                        _sourceTex[(xCoords + k) + (yCoords + j) * olicParam.width] = 1.0f;
                        // store the pixel's related droplet
                        _relateDroplets[xCoords + yCoords * olicParam.width] = dropletIndex;
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
                StreamLine* streamLine = this->calculateStreamLine(point);
                if (streamLine != nullptr) {
                    convolve(streamLine);
                }
                _hitCounts[point.first + point.second * _param->width]++;
            }
        }
    }
}

StreamLine* OlicContext::calculateStreamLine(std::pair<int, int> point) {
    std::vector<glm::vec2> fowardPoints(_param->sideLength);
    std::vector<glm::vec2> backwardPoints(_param->sideLength);
    glm::vec2 currentFoward(point.first, point.second);
    glm::vec2 currentBackward(point.first, point.second);
    int hittedDropletIndex = -1;

    // calculate forward integral and backward integral
    for (auto i = 0; i < _param->sideLength; i++) {
        glm::vec2 nextFoward = _field->RKIntergral(currentFoward, _param->integralStep);
        fowardPoints[i] = nextFoward;
        currentFoward = nextFoward;

        glm::vec2 nextBackward = _field->RKIntergral(currentBackward, -_param->integralStep);
        backwardPoints[i] = nextBackward;
        currentBackward = nextBackward;

        if (hittedDropletIndex < 0) {
            int m = getRelateDropletIndex(currentFoward);
            int n = getRelateDropletIndex(currentBackward);
            if (n > 0) {
                hittedDropletIndex = n;
            }
            if (m > 0) {
                hittedDropletIndex = m;
            }
        }
    }
    
    // streamline do not hit any droplet, discard it
    if (hittedDropletIndex < 0 && getRelateDropletIndex(point) < 0) {
        return nullptr;
    }
    // reverse the backward points to get the correct order
    std::reverse(backwardPoints.begin(), backwardPoints.end());
    backwardPoints.resize(2 * _param->sideLength);
    backwardPoints.insert(backwardPoints.end(), fowardPoints.begin(), fowardPoints.end());

    // record the droplet info for this pixel
    if (getRelateDropletIndex(point) < 0) {
        _relateDroplets[point.first + point.second * 2] = hittedDropletIndex;
    }
    StreamLine* streamLine = new StreamLine();
    streamLine->points = backwardPoints;
    streamLine->length = backwardPoints.size();
    return streamLine;
}

/**
 * @brief convolve the streamline to get the final intensity of those points.
 */
void OlicContext::convolve(StreamLine *streamLine) {
    int mid = streamLine->length / 2;
    auto midPoint = streamLine->points[mid];
    float intensity = 0.0f;
    float acum = 0.0f;
    for (auto i = -_param->sideLength; i <= _param->sideLength; i++) {
        auto currentPoint = streamLine->points[mid + i];
        if (isInclude(currentPoint)) {
            float filterWeight = RampFilter(mid + i, getRelateDroplet(midPoint).offset, streamLine->length);
            intensity += getSourceTexel(currentPoint) * filterWeight;
            acum += filterWeight;
        }
    }
    _resultTex[round(midPoint).x + round(midPoint).y * _param->width] = intensity / acum;
}
